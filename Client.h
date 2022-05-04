#ifndef CLIENT_H
#define CLIENT_H

#include "Common.h"

class Client {
private:
	int server_port;				//�������˿�
	string server_ip;				//��������IP��ַ
	int sock;						//��������������ӵ��׽���������
public:
	Client(int port, string ip);		//����
	~Client();						//����
	void run();						//����
	static void SendMsg(int conn);	//���̣߳�ר���ڷ�����Ϣ
	static void RecvMsg(int conn);	//���̣߳�ר���ڽ�����Ϣ
};

#endif // !CLIENT_H
