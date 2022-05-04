#ifndef SERVER_H
#define SERVER_H

#include "Common.h"

using namespace std;
class Server {
private:
    int server_port;                //�������˿ں�
    int server_sockfd;              //��Ϊlisten״̬���׽���������
    string server_ip;               //������ip
    static vector<bool> sock_arr;   //���������׽�����������ǰ��״̬����/�ر�
public:
    Server(int port, string ip);    //���캯��
    ~Server();                      //��������
    void run();                     //��������ʼ����
    static void RecvMsg(int conn);  //���̹߳����ľ�̬����
};

#endif // !SERVER_H
