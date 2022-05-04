#include "Client.h"

//构造
Client::Client(int port, string ip) : server_port(port), server_ip(ip) {}

//析构
Client::~Client() {
	close(sock);
}

//建立与服务器的连接并且启动发送线程和接受线程
void Client::run() {
	//定义sockfd
	sock = socket(AF_INET, SOCK_STREAM, 0);

	//定义sockaddr_in
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;							//地址族 AF_INET为本机通信
	servaddr.sin_port = htons(server_port);					//端口号，网络字节序
	servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());//服务器的 IPv4 地址

	//连接服务器
	if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("connect");
		exit(-1);
	}
	cout << "Connect successful" << endl;

	//创建接收子线程和发送子线程
	thread send_t(SendMsg, sock);
	thread recv_t(RecvMsg, sock);
	send_t.join();
	cout << "Send thread has been destroy" << endl;
	recv_t.join();
	cout << "Recv thread has been destroy" << endl;
	return;
}

void Client::SendMsg(int conn) {
	//设立缓冲区
	char sendbuf[1000];
	//循环检测接收
	while (true) {
		//初始化缓冲区
		memset(sendbuf, 0, sizeof(sendbuf));
		cin >> sendbuf;
		int ret = send(conn, sendbuf, strlen(sendbuf), 0);
		//异常时候或者 缓存区只有 exit 时退出
		if (strcmp(sendbuf, "exit") == 0 || ret <= 0) {
			break;
		}
	}
}

void Client::RecvMsg(int conn) {
	//设立缓冲区
	char recvbuf[1000];
	//循环检测接收
	while (true) {
		//初始化缓冲区
		memset(recvbuf, 0, sizeof(recvbuf));
		int len = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (len <= 0) {
			break;
		}
		cout << "Recv message from Server：" << recvbuf << endl;
	}
}