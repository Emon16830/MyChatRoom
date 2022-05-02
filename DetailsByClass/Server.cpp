#include "Common.h"
#include "Server.h"
using namespace std;

//��������ʵ��

//�������๹���޲���
Server::Server() {
	//������������˿ڼ���ַ
		//��ַ��IP����Э������  PF_INET = 2
	serverAddr.sin_family = PF_INET;
		//�� common.h ��Ĭ�ϵķ������˿ں�ת��Ϊ�����ֽ�˳�򣬼�ͳһ����֮��ı�׼
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//��ʼ��socket 
	listener = 0;

	//epoll id
	epfd = 0;
}

//��������ʼ�������������¼�
void Server::Init() {
	cout << "Init Server..." << endl;
	
	//����������socket
	listener = socket(PF_INET, SOCK_STREAM, 0);
	if(listener < 0) {
		perror("Listener error");
		exit(-1);
	}

	//�Դ��ڼ���״̬�� socket �󶨷���˵�ַ
		//bind(int sockfd,const struct sockaddr* addr,socklen_t addrlen)
		/*
		* sockfd ��Ϊ socket() �����������׽��֣���Ҫ�󶨵�socket
		* addr һ��const struct sockaddr* ��ָ�룬���������ͨ�ŵĵ�ַ�Ͷ˿�
		* addrlen ��ʶaddr�ṹ��Ĵ�С
		* return 0(true) / -1(false)
		*/
	if (bind(listener, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind error");
		exit(-1);
	}

	//���� 
		//listen() ������TCP����������
		//����socket() �������׽���Ĭ���������׽ӿڣ���׼������connect�������ӵĿͻ����׽ӿڣ�
		//ʹ��listen() ��ʹ���׽ӿڱ�Ϊ�����׽ӿڣ�����CLSED״̬��LISTEN״̬
		//�ڶ��������涨���ں�Ϊ��Ӧ��socket�����Ŷӵ�������Ӹ��� �����ȴ����ӵĶ��д�С��
	int ret = listen(listener, 5);
	if (ret < 0){
		perror("listen error");
		exit(-1);
	}

	cout << "Start to listen: " << SERVER_IP << endl;

	//���ں��д����¼���
		//epoll ��Linux �ں�������һ�����׵��ļ�ϵͳ����ԭ�ȵ�select �� poll ���÷ֳ���������
		//1. ���� epoll_create ����һ�� epoll ���󣨼��� epoll �ļ��и�������������Դ��
		//2. ���� epoll_ctl �� epoll ���������100������ӵ��׽���
		//3. ���� epoll_wait �ռ������¼�������
		/*
		* ��ĳһ���̵���epoll_create ʱ��Linux �ᴴ��һ�� eventpoll �ṹ��
		* struct eventpoll{
			//������ĸ��ڵ㣬������洢��������ӵ� epoll ���¼���Ҳ����� epoll ����ص��¼�
			struct rb_root rbr;
			//˫������ rdllist �����Ž�Ҫͨ�� epoll_wait ���ظ��û��ģ������������¼�
			struct list_head rdllist;
		 }
		* �ں�cache�н���������洢�Ժ� epoll_ctl ���ص� socket
		* epoll_wait ����ʱ�������۲���� rdllist ˫���������Ƿ�������ݼ��ɣ�
		������sleep����timeoutʱ���ʹû������Ҳ����
		* ������ӵ� epoll ���¼��������豸�������������������ص���ϵ
		������Ӧ�¼��ķ������������Ļص����������˷������ں��н� ep_poll_callback,
		�����¼��Ż� rdllist ˫��������
		* �� epoll_wait ����Ƿ����¼�����ʱ��ֻ���� eventpoll ������ rdllist ˫������
		�Ƿ����epitem����
		*/
	//epoll_create(size) 
		//����һ��epoll�ľ����size�����ں������������Ŀһ���ж��
	epfd = epoll_create(EPOLL_SIZE);

	if (epfd < 0) {
		perror("Epfd error");
		exit(-1);
	}
	
	//���¼�������Ӽ����¼�
	addfd(epfd, listener, true);
}

void Server::Close() {
	//�ر�socket
	close(listener);
	//�ر�epoll����
	close(epfd);
}

int Server::SendBroadcastMessage(int clientfd) {
	//buf[BUF_SIZE]			��������Ϣ
	//message[BUF_SIZE]		�����ʽ������Ϣ
	char buf[BUF_SIZE];
	char message[BUF_SIZE];
	//��ջ���������buf/message��ǰBUG_SIZE���ֽ�����
	bzero(buf, BUF_SIZE);
	bzero(message, BUF_SIZE);

	//��������Ϣ
	cout << "read from Client(ClientID = " << clientfd << ")" << endl;
	int len = recv(clientfd, buf, BUF_SIZE, 0);

	//����ͻ��˹ر�������
	if (len == 0) {
		close(clientfd);

		//�ڿͻ����б���ɾ���ÿͻ���
		clients_list.remove(clientfd);
		cout << "ClientID " << clientfd << " closed" << endl
			<< "Now there are " << clients_list.size()
			<< " client in the chatroom" << endl;
	}
	//���͹㲥��Ϣ�����еĿͻ���
	else {
		//�ж��Ƿ񻹴��������ͻ�������
		if (clients_list.size() == 1) {
			//������Ϣ
			send(clientfd, CAUTION, strlen(CAUTION), 0);
			return len;
		}
		//��ʽ�����͵���Ϣ����
		sprintf(message, SERVER_MESSAGE, clientfd, buf);

		//�����ͻ����б����η�����Ϣ����Ҫ�ж��Ƿ����Դ�ͻ��˷���
		list<int>::iterator it;
		for (it = clients_list.begin(); it != clients_list.end(); ++it) {
			if (*it != clientfd) {
				if (send(*it, message, BUF_SIZE, 0) < 0) {
					return -1;
				}
			}
		}
	}
	return len;
}

//����������
void Server::Start() {

	//epoll �¼�����
	static struct epoll_event events[EPOLL_SIZE];

	//��ʼ��������
	Init();

	//��ѭ��
	while (true) {
		//epoll_events_count ��ʾ�����¼�����Ŀ
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		
		if (epoll_events_count < 0) {
			perror("Epoll Failure");
			break;
		}

		cout << "epoll_events_count =" << endl << epoll_events_count << endl;

		//������epoll_events_count ���������¼�
		for (int i = 0; i < epoll_events_count; ++i) {
			int sockfd = events[i].data.fd;
			//���û�������
			if (sockfd == listener) {
				struct sockaddr_in client_address;
				socklen_t client_addrLength = sizeof(struct sockaddr_in);
				int clientfd = accept(listener, (struct sockaddr*)&client_address, &client_addrLength);

				cout << "client connection from:"
					<< inet_ntoa(client_address.sin_addr) << ":"
					<< ntohs(client_address.sin_port) << ", clientfd = "
					<< clientfd << endl;

				addfd(epfd, clientfd, true);

				//��������list �����û�����
				clients_list.push_back(clientfd);
				cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
				cout << "Now there are " << clients_list.size() << " clients in the chatroom" << endl;

				//���������ͻ�ӭ��Ϣ
				cout << "Welcome message" << endl;
				char message[BUF_SIZE];
				bzero(message, BUF_SIZE);
				sprintf(message, SERVER_WELCOME, clientfd);
				int ret = send(clientfd, message, BUF_SIZE, 0);
				if (ret < 0) {
					perror("Send error");
					Close();
					exit(-1);
				}
			}
			//�����û����͵���Ϣ�����ҹ㲥��ʹ�����û��յ���Ϣ
			else {
				int ret = SendBroadcastMessage(sockfd);
				if (ret < 0) {
					perror("error");
					Close();
					exit(-1);
				}
			}
		}
	}
	//�رշ���
	Close();
}
