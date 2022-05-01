#include <iostream>
#include "Client.h"
using namespace std;

//客户端类的实现

//客户端类的构造
Client::Client() {
	//初始化要连接的服务器地址和端口
		//serverAddr 是结构体socketaddr_in的一个实例
		/*
			struct socketaddr_in{
				sa_family_t		sin_family;		//地址族
				unit16_t		sin_port;		//16位的TCP/UDP端口号
				struct in_addr	sin_addr;		//32位的IP地址
				char			sin_zero[8];	//不使用
			};
			struct in_addr{
				In_addr_it		s_addr;			//32位的IPV4地址
			};
		*/
	serverAddr.sin_family = PF_INET;
		//htons 将主机字节顺序转换为（统一）的网络字节顺序
	serverAddr.sin_port = htons(SERVER_PORT);
		//inet_addr 将ip地址转化为一个整数值
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//初始化socket
	sock = 0;
	//初始化进程号
	pid = 0;
	//客户端状态是否启用
	isClientwork = true;
	// epoll fd
	epfd = 0;
}

//连接服务器
void Client::Connect() {
	//显示连接服务器端口的IP和端口号
	cout << "Connect Server: " << SERVER_IP << " : " << SERVER_PORT << endl;

	//创建socket
	//socket(int domain ,int type, int protocol)
		//domain协议域，决定了socket的地址类型
		//type 指定socket的类型，1 SOCK_STREAM 使用TCP，可靠，双向  2 SOCK_SGRAM  无连接，使用UDP
		//protocol 协议，对应IPPROTO_TCP、IPPTOTO_UDP、IPPROTO_SCTP、IPPROTO_TIPC ，为 0 自动选择
		//返回一个标识这个套接字的文件描述符
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Sock error");
		exit(-1);
	}
	//连接服务器
		//connect(int sockfd,const struct sockaddr *addr,socklen_t,addlen)
		//客户端 socket 的标识，套接字要连接的主机地址和端口号，name的缓冲区
	if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("Connect error");
		exit(-1);
	}
	//创建管道，其中 fd[0] 用于父进程读，fd[1] 用于子进程写
		//UNIX系统中IPC（进程之间通信）的最古老方式
		//半双工，数据只能从一段到另一端 ，先入先出 ，写入和读出都必须约定好数据格式
		//从 fd[1] 写入只能从 fd[0] 读出
		//从管路中读取为一次性操作，即读完就丢
	if (pipe(pipe_fd) < 0) {
		perror("Pipe error");
		exit(-1);
	}
	//创建epoll
		//EPOLL_SIZE为创建的 epoll 实例的最大监听数量，epfd为epoll句柄
	epfd = epoll_create(EPOLL_SIZE);
	if (epfd < 0) {
		perror("epfd error");
		exit(-1);
	}

	//将sock和管道读写端的描述符都添加到内核的事件表中
	addfd(epfd, sock, true);
	addfd(epfd, pipe_fd[0], true);
}
//断开连接，清理并且关闭文件描述符
void Client::Close() {
	if (pid) {
		//关闭父进程的管道和sock
		close(pipe_fd[0]);
		close(sock);
	}
	else {
		//关闭子进程的管道
		close(pipe_fd[1]);
	}
}
//启动客户端
void Client::Start() {
	//epoll 事件队列
	static struct epoll_event events[2];
	//连接服务器
	Connect();
	//创建子进程
		//通过系统调用创建一个与原来完全相同的进程，即两个进程可以做完全相同的事情
		//调用fork之后，先给新进程分配资源，然后把原来的进程所有值复制进新进程中
	pid = fork();
	//子进程创建失败，退出
	if (pid < 0) {
		perror("fork error");
		close(sock);
		exit(-1);
	}
	else if (pid == 0) {
		//进入子进程执行流程
		//子进程负责写入管道，因此先关闭读端
		close(pipe_fd[0]);

		//输出"exit"退出
		cout << "Please input “exit” to exit the ChatRoom " << endl;

		//若客户端运行正常则不断读取输入发送给服务端
		while (isClientwork) {
			//将字符串message中前BUF_SIZE个字节置零
			bzero(&message, BUF_SIZE);
			//message 字符型指针，指向存储读入数据的缓冲区地址	（容器地址）
			//从流中读入n-1个字符								（容器的大小）	
			//stdin 指向读取的流								（从哪里读取）
			fgets(message, BUF_SIZE, stdin);

			//客户端输出exit，退出聊天室
				//strncasecmp 比较字符串str1 和 str2 字符串的前 strlen（EXIT）个字符
			if (strncasecmp(message, EXIT, strlen(EXIT)) == 0) {
				isClientwork = 0;
			}
			else {
				//write（int fd,const void* buf,size_t count)
				//将buff缓冲区的数据 count 个字节写入到指定的文件 fd 中
				if (write(pipe_fd[1], message, strlen(message) - 1) < 0) {
					perror("Fork Error");
					exit(-1);
				}
			}
		}
	}
	else {
		//pid > 0 父进程，父进程中的pid等于fork返回的子进程ID
		//父进程负责读管道数据，因此先关闭写端
		close(pipe_fd[1]);

		//主循环（epoll_wait)
		while (isClientwork) {
			int epoll_events_count = epoll_wait(epfd, events, 2, -1);

			//处理就绪事件
			for (int i = 0; i < epoll_events_count; ++i) {
				//清空缓存区
				bzero(&message, BUF_SIZE);
				//服务端发来消息
				if (events[i].data.fd == sock) {
					//接受服务端信息
					// int recv(SOCKET s,char FAR* buf,int len,int flags)
					// s的接受缓存区接受完毕，将 s 中的数据 copy 到buf中，如果大于buf的长度，需要多次调用recv
					// s指定接收段套接字描述符，指明缓冲区，用于存放recv函数接收到的数据，
					// len指明缓冲区的长度，0 则是把TCP缓冲区的数据读取之后清除，MSG_PEEK则不清除
					int ret = recv(sock, message, BUF_SIZE, 0);
					//ret = 0 服务端关闭
					if (ret == 0) {
						cout << "Server closed connection: " << sock << endl;
						close(sock);
						isClientwork = 0;
					}
					else {
						cout << message << endl;
					}
				}
				//子进程写入事件的发生，父进程处理并且发送给服务端
				else {
					//父进程从管道中读取数据
					//read(int fd,void* buf,size_t count)
					//将 fd 所指向的文件 count 个字节写入到 buf 所指向的内存中
					int ret = read(events[i].data.fd, message, BUF_SIZE);

					//ret = 0
					if (ret == 0) {
						isClientwork = 0;
					}
					else {
						//发送消息给服务器
						// send(int sockfd,const void* msg,size_t len,int flags)
						// 指定发送套接字描述符，msg存放发送数据的缓存区
						// len 指定要发送的数据字符大小，flags 标志
						// 只有在套接字处于连接时才能使用
						send(sock, message, BUF_SIZE, 0);
					}
				}
			}//end for
		}//end while
	}
	//退出进程
	Close();
}
