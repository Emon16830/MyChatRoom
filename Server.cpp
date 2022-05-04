#include "server.h"

using namespace std;

//初始化sock_arr数组，记录套接字是否打开
vector<bool> Server::sock_arr(10000, false);

//构造
Server::Server(int port, string ip) : server_port(port), server_ip(ip) {}

//析构
Server::~Server() {
	for (int i = 0; i < sock_arr.size(); ++i) {
		if (sock_arr[i]) {
			close(i);
		}
	}
	close(server_sockfd);
}

void Server::run() {
	//定义sockfd
		
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//定义socketaddr_in
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family = AF_INET;				// TCP/IP协议族，AF_INET 本机通信
	server_sockaddr.sin_port = htons(server_port);		// 将端口的主机字节序转换为网络字节序
	server_sockaddr.sin_addr.s_addr = inet_addr(server_ip.c_str());		//ip地址，127.0.0.1

	//给创建好的socket 绑定地址
	if (bind(server_sockfd, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
		perror("Bind error");
		exit(-1);
	}
	
	//将创建好的socket放入listen监听队列，等待客户端的请求
	if (listen(server_sockfd, 5) == -1) {
		perror("Listen error");
		exit(-1);
	}

	//为客户端创建套接字
	struct sockaddr_in client_addr;
	socklen_t client_addr_length = sizeof(client_addr);

	//不断取出新连接并且创建子线程为其服务
	while (true) {
		//accept 返回一个新的连接 socket，服务器读写conn进行通信
		//accept 并不关心网络状态，只是从监听队列中取出连接
		int conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_addr_length);
		if (conn < 0) {
			perror("Connect error");
			exit(-1);
		}
		cout << "The Client ID : " << conn << " connected successful" << endl;
		sock_arr.push_back(conn);
		//创建线程，让子线程进行连接好的socket进行读写
		thread t(Server::RecvMsg, conn);
		//与主线程分离出来，join() 会堵塞主线程
		t.detach();
	}
}

//子线程工作函数
void Server::RecvMsg(int conn) {
	//设立接受缓存区
	char buffer[1000];
	//循环检测接受数据
	while (true) {
		//初始化内存块，作用同 bzero()
		memset(buffer, 0, sizeof(buffer));
		//从conn 标识的socket 的TCP缓存区中读取，并赋到buffer中，0 为是否清除TCP缓存区
		int len = recv(conn, buffer, sizeof(buffer), 0);
		//接收到客户端发送的 exit 或者 异常时退出
		if (strcmp(buffer, "exit") == 0 || len <= 0) {
			//关闭当前套接字
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