#include "server.h"

int main(int argc,char* argv[]) {
	Server serv(8023, "127.0.0.1");
	serv.run();
	return 0;
}