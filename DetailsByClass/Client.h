#ifndef CLIENT_H
#define CLIENT_H
using namespace std;

#include <string>
#include "Common.h"

class Client {
public:
	//�޲ι��캯��
	Client();
	//���ӷ�����
	void Connect();
	//�Ͽ�����
	void Close();
	//����������
	void Start();
private:
	//��ǰ������������socket
	int sock;
	//��ǰ���̵�ID
	int pid;
	//epoll_create������ķ���ֵ
	int epfd;
	//�����ܵ������� fd[0] ���ڸ����̶���fd[1] �����ӽ���д
	int pipe_fd[2];
	//��ʾ�ͻ����Ƿ���������
	bool isClientwork;
	//�������Ļ���
	char message[BUF_SIZE];
	//�û������ӵķ����� IP + port
	struct sockaddr_in serverAddr;
};

#endif // !CLIENT_H
