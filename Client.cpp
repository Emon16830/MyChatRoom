#include "Client.h"

//����
Client::Client(int port, string ip) : server_port(port), server_ip(ip) {}

//����
Client::~Client() {
	close(sock);
}

//����������������Ӳ������������̺߳ͽ����߳�
void Client::run() {
	//����sockfd
	sock = socket(AF_INET, SOCK_STREAM, 0);

	//����sockaddr_in
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;							//��ַ�� AF_INETΪ����ͨ��
	servaddr.sin_port = htons(server_port);					//�˿ںţ������ֽ���
	servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());//�������� IPv4 ��ַ

	//���ӷ�����
	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("connect");
		exit(-1);
	}
	cout << "Connect successful" << endl;

	//�����������̺߳ͷ������߳�
	thread send_t(SendMsg, sock);
	thread recv_t(RecvMsg, sock);
	send_t.join();
	cout << "Send thread has been destroy" << endl;
	recv_t.join();
	cout << "Recv thread has been destroy" << endl;
	return;
}

void Client::SendMsg(int conn) {
	//����������
	char sendbuf[1000];
	//ѭ��������
	while (true) {
		//��ʼ��������
		memset(sendbuf, 0, sizeof(sendbuf));
		cin >> sendbuf;
		int ret = send(conn, sendbuf, strlen(sendbuf), 0);
		//�쳣ʱ����� ������ֻ�� exit ʱ�˳�
		if (strcmp(sendbuf, "exit") == 0 || ret <= 0) {
			break;
		}
	}
}

void Client::RecvMsg(int conn) {
	//����������
	char recvbuf[1000];
	//ѭ��������
	while (true) {
		//��ʼ��������
		memset(recvbuf, 0, sizeof(recvbuf));
		int len = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (len <= 0) {
			break;
		}
		cout << "Recv message from Server��" << recvbuf << endl;
	}
}