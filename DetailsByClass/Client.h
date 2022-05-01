#ifndef CLIENT_H
#define CLIENT_H
using namespace std;

#include <string>
#include "Common.h"

class Client {
public:
	//无参构造函数
	Client();
	//连接服务器
	void Connect();
	//断开连接
	void Close();
	//启动服务器
	void Start();
private:
	//当前服务器创建的socket
	int sock;
	//当前进程的ID
	int pid;
	//epoll_create创建后的返回值
	int epfd;
	//创建管道，其中 fd[0] 用于父进程读，fd[1] 用于子进程写
	int pipe_fd[2];
	//表示客户端是否正常工作
	bool isClientwork;
	//聊天区的缓冲
	char message[BUF_SIZE];
	//用户所连接的服务器 IP + port
	struct sockaddr_in serverAddr;
};

#endif // !CLIENT_H
