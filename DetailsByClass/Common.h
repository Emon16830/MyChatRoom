#ifndef COMMON_H
#define COMMON_H
using namespace std;

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Ĭ�Ϸ������˵�IP��ַ
#define SERVER_IP "127.0.0.1"

//�������˿ں�
#define SERVER_PORT 8888

//int epoll_create(int size)�е�size
//Ϊepoll֧�ֵ��������
#define EPOLL_SIZE 5000

//�������Ĵ�С65535
#define BUF_SIZE 0xFFFF

//���û���½��Ļ�ӭ��Ϣ
#define SERVER_WELCOME "Welcome to join the chatroom,Your ID is: Client #%d"

//�����û��յ���ǰ׺
#define SERVER_MESSAGE "ClientID %d say >> %s"

//�˳�ϵͳ
#define EXIT "EXIT"

#define CAUTION "There is only one int the chatroom!"

//ע���µ� fd �� epollfd ��
//����enable_et��ʾ�Ƿ�����ETģʽ�����Ϊ True �����ã�����ʹ��LTģʽ
static void addfd(int epollfd, int fd, bool enable_et) {
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN;
	if (enable_et) {
		ev.events = EPOLLIN | EPOLLET;
	}
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	//���� socket Ϊ nonblocking ģʽ
	// ִ�����ת����һ��ָ����ܺ�����û�з���
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
	cout << "fd added to epoll" << endl;
}
#endif // !COMMON_H
