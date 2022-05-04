# MyChatRoom
 采用C/S架构，基于TCP的网络聊天室


Update on May 2th :
    由于之前是跟着别人的写的，用的都是库函数，不是很了解底层的实现原理和知识，所以找了好一点的项目重新写一遍，加深理解。
    亮点：
	使用pipe（pipe_fd[2])（可改进为双工的sockpair）和fork() 函数，实现父进程读，子进程写。 
 	采用epoll 建立事件表，epoll_wait 调用得到启用的socket
    
    
Update on May 4th:    
  	将初始版本的 Server 类和 Client 类使用多线程实现同时收发
	采用vector容器记录每一个socket连接    
