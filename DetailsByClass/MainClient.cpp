#include "Client.h"

//客户端主函数
//创建客户端对象后启动客户端
int main(int argc, char* argv[]) {
	Client client;
	client.Start();
	return 0;
}
//argc	整型，统计程序运行时发送给main函数的命令行参数的个数
//argv	为 argc 个参数，存放指向字符串参数的指针数组，
//		其中第一个为文件名字，后续记录 DOS 命令行中执行程序名后的第一个参数