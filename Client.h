#ifndef CLIENT_H
#define CLIENT_H

#include "Common.h"

class Client {
private:
	int server_port;				//服务器端口
	string server_ip;				//服务器的IP地址
	int sock;						//与服务器建立连接的套接字描述符
public:
	Client(int port, string ip);		//构造
	~Client();						//析构
	void run();						//运行
	static void SendMsg(int conn);	//子线程，专用于发送信息
	static void RecvMsg(int conn);	//子线程，专用于接受信息
};

#endif // !CLIENT_H
