#ifndef SERVER_H
#define SERVER_H
using namespace std;

#include <string>
#include "Common.h"


class Server {
public:
	//无参的构造
	Server();
	
	//初始化服务器端的设置
	void Init();
	
	//关闭服务
	void Close();

	//启动服务端
	void Start();
	
private:
	//给所有客户端广播
	int SendBroadcastMessage(int clientfd);

	//服务器端serverAddr信息
	struct sockaddr_in serverAddr;

	//创建监听的socket
	int listener;

	// epoll_create 创建后的返回值
	int epfd;

	//客户端列表
	list<int> clients_list;

};
#endif // !SERVER_H
