#include "Common.h"
#include "Server.h"
using namespace std;

//服务器类实现

//服务器类构造无参数
Server::Server() {
	//输出话服务器端口及地址
		//地址族IP，即协议类型  PF_INET = 2
	serverAddr.sin_family = PF_INET;
		//将 common.h 中默认的服务器端口号转换为网络字节顺序，即统一网络之间的标准
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

	//初始化socket 
	listener = 0;

	//epoll id
	epfd = 0;
}

//服务器初始化并启动监听事件
void Server::Init() {
	cout << "Init Server..." << endl;
	
	//创建监听的socket
	listener = socket(PF_INET, SOCK_STREAM, 0);
	if(listener < 0) {
		perror("Listener error");
		exit(-1);
	}

	//对处于监听状态的 socket 绑定服务端地址
		//bind(int sockfd,const struct sockaddr* addr,socklen_t addrlen)
		/*
		* sockfd 即为 socket() 函数创建的套接字，需要绑定的socket
		* addr 一个const struct sockaddr* 的指针，服务端用于通信的地址和端口
		* addrlen 标识addr结构体的大小
		* return 0(true) / -1(false)
		*/
	if (bind(listener, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		perror("bind error");
		exit(-1);
	}

	//监听 
		//listen() 仅可由TCP服务器调用
		//调用socket() 创建的套接字默认是主动套接口（即准备调用connect发起连接的客户端套接口）
		//使用listen() 将使该套接口变为被动套接口，即从CLSED状态到LISTEN状态
		//第二个参数规定了内核为相应的socket可以排队的最大连接个数 （即等待连接的队列大小）
	int ret = listen(listener, 5);
	if (ret < 0){
		perror("listen error");
		exit(-1);
	}

	cout << "Start to listen: " << SERVER_IP << endl;

	//在内核中创建事件表
		//epoll 在Linux 内核中申请一个简易的文件系统，将原先的select 或 poll 调用分成三个部分
		//1. 调用 epoll_create 建立一个 epoll 对象（即在 epoll 文件中给这个句柄分配资源）
		//2. 调用 epoll_ctl 向 epoll 对象添加这100万个连接的套接字
		//3. 调用 epoll_wait 收集发生事件的连接
		/*
		* 当某一进程调用epoll_create 时，Linux 会创建一个 eventpoll 结构体
		* struct eventpoll{
			//红黑树的根节点，这棵树存储着所有添加到 epoll 的事件，也是这个 epoll 所监控的事件
			struct rb_root rbr;
			//双向链表 rdllist 保存着将要通过 epoll_wait 返回给用户的，满足条件的事件
			struct list_head rdllist;
		 }
		* 内核cache中建立红黑树存储以后 epoll_ctl 返回的 socket
		* epoll_wait 调用时，仅仅观察这个 rdllist 双向链表中是否存在数据即可，
		无数据sleep，或timeout时间后即使没有数据也返回
		* 所有添加到 epoll 的事件都会与设备驱动（如网卡）建立回调关系
		（即相应事件的发生会调用这里的回调方法），此方法在内核中叫 ep_poll_callback,
		它将事件放回 rdllist 双向链表中
		* 当 epoll_wait 检查是否有事件连接时，只需检查 eventpoll 对象中 rdllist 双向链表
		是否存在epitem即可
		*/
	//epoll_create(size) 
		//创建一个epoll的句柄，size告诉内核这个监听的数目一共有多大
	epfd = epoll_create(EPOLL_SIZE);

	if (epfd < 0) {
		perror("Epfd error");
		exit(-1);
	}
	
	//往事件表中添加监听事件
	addfd(epfd, listener, true);
}

void Server::Close() {
	//关闭socket
	close(listener);
	//关闭epoll监听
	close(epfd);
}

int Server::SendBroadcastMessage(int clientfd) {
	//buf[BUF_SIZE]			接受新消息
	//message[BUF_SIZE]		保存格式化的消息
	char buf[BUF_SIZE];
	char message[BUF_SIZE];
	//清空缓存区，将buf/message中前BUG_SIZE个字节置零
	bzero(buf, BUF_SIZE);
	bzero(message, BUF_SIZE);

	//接受新消息
	cout << "read from Client(ClientID = " << clientfd << ")" << endl;
	int len = recv(clientfd, buf, BUF_SIZE, 0);

	//如果客户端关闭了连接
	if (len == 0) {
		close(clientfd);

		//在客户端列表中删除该客户端
		clients_list.remove(clientfd);
		cout << "ClientID " << clientfd << " closed" << endl
			<< "Now there are " << clients_list.size()
			<< " client in the chatroom" << endl;
	}
	//发送广播消息给所有的客户端
	else {
		//判断是否还存在其他客户端连接
		if (clients_list.size() == 1) {
			//发送消息
			send(clientfd, CAUTION, strlen(CAUTION), 0);
			return len;
		}
		//格式化发送的消息内容
		sprintf(message, SERVER_MESSAGE, clientfd, buf);

		//遍历客户端列表依次发送消息，需要判断是否给来源客户端发送
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

//启动服务器
void Server::Start() {

	//epoll 事件队列
	static struct epoll_event events[EPOLL_SIZE];

	//初始化服务器
	Init();

	//主循环
	while (true) {
		//epoll_events_count 表示就绪事件的数目
		int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
		
		if (epoll_events_count < 0) {
			perror("Epoll Failure");
			break;
		}

		cout << "epoll_events_count =" << endl << epoll_events_count << endl;

		//处理这epoll_events_count 个就绪的事件
		for (int i = 0; i < epoll_events_count; ++i) {
			int sockfd = events[i].data.fd;
			//新用户的连接
			if (sockfd == listener) {
				struct sockaddr_in client_address;
				socklen_t client_addrLength = sizeof(struct sockaddr_in);
				int clientfd = accept(listener, (struct sockaddr*)&client_address, &client_addrLength);

				cout << "client connection from:"
					<< inet_ntoa(client_address.sin_addr) << ":"
					<< ntohs(client_address.sin_port) << ", clientfd = "
					<< clientfd << endl;

				addfd(epfd, clientfd, true);

				//服务器用list 保存用户连接
				clients_list.push_back(clientfd);
				cout << "Add new clientfd = " << clientfd << " to epoll" << endl;
				cout << "Now there are " << clients_list.size() << " clients in the chatroom" << endl;

				//服务器发送欢迎信息
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
			//处理用户发送的消息，并且广播，使其他用户收到消息
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
	//关闭服务
	Close();
}
