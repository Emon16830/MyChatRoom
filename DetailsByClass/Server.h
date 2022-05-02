#ifndef SERVER_H
#define SERVER_H
using namespace std;

#include <string>
#include "Common.h"


class Server {
public:
	//�޲εĹ���
	Server();
	
	//��ʼ���������˵�����
	void Init();
	
	//�رշ���
	void Close();

	//���������
	void Start();
	
private:
	//�����пͻ��˹㲥
	int SendBroadcastMessage(int clientfd);

	//��������serverAddr��Ϣ
	struct sockaddr_in serverAddr;

	//����������socket
	int listener;

	// epoll_create ������ķ���ֵ
	int epfd;

	//�ͻ����б�
	list<int> clients_list;

};
#endif // !SERVER_H
