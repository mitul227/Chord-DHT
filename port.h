#ifndef port_h
#define port_h

#include <iostream>
#include <netinet/in.h>
using namespace std;

class SocketAndPort{
	private:
		int portNoServer;
		int sock;
		struct sockaddr_in current;

	public:
		void specifyPortServer();
		void changePortNumber(int portNo);
		void closeSocket();
		bool portInUse(int portNo);
		string getIpAddress();
		int getPortNumber();
		int getSocketFd();
};

#endif