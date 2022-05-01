#include <iostream>
#include "Client.h"
using namespace std;

//�ͻ������ʵ��

//�ͻ�����Ĺ���
Client::Client() {
	//��ʼ��Ҫ���ӵķ�������ַ�Ͷ˿�
		//serverAddr �ǽṹ��socketaddr_in��һ��ʵ��
		/*
			struct socketaddr_in{
				sa_family_t		sin_family;		//��ַ��
				unit16_t		sin_port;		//16λ��TCP/UDP�˿ں�
				struct in_addr	sin_addr;		//32λ��IP��ַ
				char			sin_zero[8];	//��ʹ��
			};
			struct in_addr{
				In_addr_it		s_addr;			//32λ��IPV4��ַ
			};
		*/
	serverAddr.sin_family = PF_INET;
		//htons �������ֽ�˳��ת��Ϊ��ͳһ���������ֽ�˳��
	serverAddr.sin_port = htons(SERVER_PORT);
		//inet_addr ��ip��ַת��Ϊһ������ֵ
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//��ʼ��socket
	sock = 0;
	//��ʼ�����̺�
	pid = 0;
	//�ͻ���״̬�Ƿ�����
	isClientwork = true;
	// epoll fd
	epfd = 0;
}

//���ӷ�����
void Client::Connect() {
	//��ʾ���ӷ������˿ڵ�IP�Ͷ˿ں�
	cout << "Connect Server: " << SERVER_IP << " : " << SERVER_PORT << endl;

	//����socket
	//socket(int domain ,int type, int protocol)
		//domainЭ���򣬾�����socket�ĵ�ַ����
		//type ָ��socket�����ͣ�1 SOCK_STREAM ʹ��TCP���ɿ���˫��  2 SOCK_SGRAM  �����ӣ�ʹ��UDP
		//protocol Э�飬��ӦIPPROTO_TCP��IPPTOTO_UDP��IPPROTO_SCTP��IPPROTO_TIPC ��Ϊ 0 �Զ�ѡ��
		//����һ����ʶ����׽��ֵ��ļ�������
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Sock error");
		exit(-1);
	}
	//���ӷ�����
		//connect(int sockfd,const struct sockaddr *addr,socklen_t,addlen)
		//�ͻ��� socket �ı�ʶ���׽���Ҫ���ӵ�������ַ�Ͷ˿ںţ�name�Ļ�����
	if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Connect error");
		exit(-1);
	}
	//�����ܵ������� fd[0] ���ڸ����̶���fd[1] �����ӽ���д
		//UNIXϵͳ��IPC������֮��ͨ�ţ�������Ϸ�ʽ
		//��˫��������ֻ�ܴ�һ�ε���һ�� �������ȳ� ��д��Ͷ���������Լ�������ݸ�ʽ
		//�� fd[1] д��ֻ�ܴ� fd[0] ����
		//�ӹ�·�ж�ȡΪһ���Բ�����������Ͷ�
	if (pipe(pipe_fd) < 0) {
		perror("Pipe error");
		exit(-1);
	}
	//����epoll
		//EPOLL_SIZEΪ������ epoll ʵ����������������epfdΪepoll���
	epfd = epoll_create(EPOLL_SIZE);
	if (epfd < 0) {
		perror("epfd error");
		exit(-1);
	}

	//��sock�͹ܵ���д�˵�����������ӵ��ں˵��¼�����
	addfd(epfd, sock, true);
	addfd(epfd, pipe_fd[0], true);
}
//�Ͽ����ӣ������ҹر��ļ�������
void Client::Close() {
	if (pid) {
		//�رո����̵Ĺܵ���sock
		close(pipe_fd[0]);
		close(sock);
	}
	else {
		//�ر��ӽ��̵Ĺܵ�
		close(pipe_fd[1]);
	}
}
//�����ͻ���
void Client::Start() {
	//epoll �¼�����
	static struct epoll_event events[2];
	//���ӷ�����
	Connect();
	//�����ӽ���
		//ͨ��ϵͳ���ô���һ����ԭ����ȫ��ͬ�Ľ��̣����������̿�������ȫ��ͬ������
		//����fork֮���ȸ��½��̷�����Դ��Ȼ���ԭ���Ľ�������ֵ���ƽ��½�����
	pid = fork();
	//�ӽ��̴���ʧ�ܣ��˳�
	if (pid < 0) {
		perror("fork error");
		close(sock);
		exit(-1);
	}
	else if (pid == 0) {
		//�����ӽ���ִ������
		//�ӽ��̸���д��ܵ�������ȹرն���
		close(pipe_fd[0]);

		//���"exit"�˳�
		cout << "Please input ��exit�� to exit the ChatRoom " << endl;

		//���ͻ������������򲻶϶�ȡ���뷢�͸������
		while (isClientwork) {
			//���ַ���message��ǰBUF_SIZE���ֽ�����
			bzero(&message, BUF_SIZE);
			//message �ַ���ָ�룬ָ��洢�������ݵĻ�������ַ	��������ַ��
			//�����ж���n-1���ַ�								�������Ĵ�С��	
			//stdin ָ���ȡ����								���������ȡ��
			fgets(message, BUF_SIZE, stdin);

			//�ͻ������exit���˳�������
				//strncasecmp �Ƚ��ַ���str1 �� str2 �ַ�����ǰ strlen��EXIT�����ַ�
			if (strncasecmp(message, EXIT, strlen(EXIT)) == 0) {
				isClientwork = 0;
			}
			else {
				//write��int fd,const void* buf,size_t count)
				//��buff������������ count ���ֽ�д�뵽ָ�����ļ� fd ��
				if (write(pipe_fd[1], message, strlen(message) - 1) < 0) {
					perror("Fork Error");
					exit(-1);
				}
			}
		}
	}
	else {
		//pid > 0 �����̣��������е�pid����fork���ص��ӽ���ID
		//�����̸�����ܵ����ݣ�����ȹر�д��
		close(pipe_fd[1]);

		//��ѭ����epoll_wait)
		while (isClientwork) {
			int epoll_events_count = epoll_wait(epfd, events, 2, -1);

			//��������¼�
			for (int i = 0; i < epoll_events_count; ++i) {
				//��ջ�����
				bzero(&message, BUF_SIZE);
				//����˷�����Ϣ
				if (events[i].data.fd == sock) {
					//���ܷ������Ϣ
					// int recv(SOCKET s,char FAR* buf,int len,int flags)
					// s�Ľ��ܻ�����������ϣ��� s �е����� copy ��buf�У��������buf�ĳ��ȣ���Ҫ��ε���recv
					// sָ�����ն��׽�����������ָ�������������ڴ��recv�������յ������ݣ�
					// lenָ���������ĳ��ȣ�0 ���ǰ�TCP�����������ݶ�ȡ֮�������MSG_PEEK�����
					int ret = recv(sock, message, BUF_SIZE, 0);
					//ret = 0 ����˹ر�
					if (ret == 0) {
						cout << "Server closed connection: " << sock << endl;
						close(sock);
						isClientwork = 0;
					}
					else {
						cout << message << endl;
					}
				}
				//�ӽ���д���¼��ķ����������̴����ҷ��͸������
				else {
					//�����̴ӹܵ��ж�ȡ����
					//read(int fd,void* buf,size_t count)
					//�� fd ��ָ����ļ� count ���ֽ�д�뵽 buf ��ָ����ڴ���
					int ret = read(events[i].data.fd, message, BUF_SIZE);

					//ret = 0
					if (ret == 0) {
						isClientwork = 0;
					}
					else {
						//������Ϣ��������
						// send(int sockfd,const void* msg,size_t len,int flags)
						// ָ�������׽�����������msg��ŷ������ݵĻ�����
						// len ָ��Ҫ���͵������ַ���С��flags ��־
						// ֻ�����׽��ִ�������ʱ����ʹ��
						send(sock, message, BUF_SIZE, 0);
					}
				}
			}//end for
		}//end while
	}
	//�˳�����
	Close();
}
