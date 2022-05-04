#include "server.h"

using namespace std;

//��ʼ��sock_arr���飬��¼�׽����Ƿ��
vector<bool> Server::sock_arr(10000, false);

//����
Server::Server(int port, string ip) : server_port(port), server_ip(ip) {}

//����
Server::~Server() {
	for (int i = 0; i < sock_arr.size(); ++i) {
		if (sock_arr[i]) {
			close(i);
		}
	}
	close(server_sockfd);
}

void Server::run() {
	//����sockfd
		
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//����socketaddr_in
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;				// TCP/IPЭ���壬AF_INET ����ͨ��
	server_sockaddr.sin_port = htons(server_port);		// ���˿ڵ������ֽ���ת��Ϊ�����ֽ���
	server_sockaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());		//ip��ַ��127.0.0.1

	//�������õ�socket �󶨵�ַ
	if (bind(server_sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
		perror("Bind error");
		exit(-1);
	}
	
	//�������õ�socket����listen�������У��ȴ��ͻ��˵�����
	if (listen(server_sockfd, 5) == -1) {
		perror("Listen error");
		exit(-1);
	}

	//Ϊ�ͻ��˴����׽���
	struct sockaddr_in client_addr;
	socklen_t client_addr_length = sizeof(client_addr);

	//����ȡ�������Ӳ��Ҵ������߳�Ϊ�����
	while (true) {
		//accept ����һ���µ����� socket����������дconn����ͨ��
		//accept ������������״̬��ֻ�ǴӼ���������ȡ������
		int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_addr_length);
		if (conn < 0) {
			perror("Connect error");
			exit(-1);
		}
		cout << "The Client ID : " << conn << " connected successful" << endl;
		sock_arr.push_back(conn);
		//�����̣߳������߳̽������Ӻõ�socket���ж�д
		thread t(Server::RecvMsg, conn);
		//�����̷߳��������join() ��������߳�
		t.detach();
	}
}

//���̹߳�������
void Server::RecvMsg(int conn) {
	//�������ܻ�����
	char buffer[1000];
	//ѭ������������
	while (true) {
		//��ʼ���ڴ�飬����ͬ bzero()
		memset(buffer, 0, sizeof(buffer));
		//��conn ��ʶ��socket ��TCP�������ж�ȡ��������buffer�У�0 Ϊ�Ƿ����TCP������
		int len = recv(conn, buffer, sizeof(buffer), 0);
		//���յ��ͻ��˷��͵� exit ���� �쳣ʱ�˳�
		if (strcmp(buffer, "exit") == 0 || len <= 0) {
			//�رյ�ǰ�׽���
			close(conn);
			sock_arr[conn] = false;
			break;
		}
		cout << "Recv from ID: " << conn << endl << " message:" << buffer << endl;
		string ans = "Recved";
		int ret = send(conn, ans.c_str(), ans.length(), 0);
		if (ret <= 0) {
			close(conn);
			sock_arr[conn] = false;
			break;
		}
	}
}