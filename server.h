#ifndef SERVER_H
#define SERVER_H

#include "Common.h"

using namespace std;
class Server {
private:
    int server_port;                //服务器端口号
    int server_sockfd;              //设为listen状态的套接字描述符
    string server_ip;               //服务器ip
    static vector<bool> sock_arr;   //保存所有套接字描述符当前的状态，打开/关闭
public:
    Server(int port, string ip);    //构造函数
    ~Server();                      //析构函数
    void run();                     //服务器开始工作
    static void RecvMsg(int conn);  //子线程工作的静态函数
};

#endif // !SERVER_H
