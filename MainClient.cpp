#include "Client.h"

int main(int argc, char* argv[]) {
	Client clnt(8023, "127.0.0.1");
	clnt.run();
}