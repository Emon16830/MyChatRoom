#include "Server.h"

//服务器端主函数
//实例化服务器类之后开启连接
int main(int argc, char* argv[]) {
	Server server;
	server.Start();
	return 0;
}