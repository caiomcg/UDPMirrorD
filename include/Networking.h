#ifndef NETWORKING_H
#define NETWORKING_H

#include "NetworkingErrors.h"

//Networking Headers
//---------------------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
//---------------------------------------------------------------------------------------------
#include <errno.h>

typedef int(*callbackFuction)(char* data, int size, int error);

enum SocketType{
	UDP,
	TCP
};

enum SocketFamily{
	IPV4,
	IPV6
};

enum SocketUser{
	CLIENT,
	SERVER
};

class Networking{
private:
	int serverFileDescriptor;
	int clientFileDescriptor;

	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;
	socklen_t slen;

	int createSocket(SocketType type, SocketFamily family, SocketUser user);
	int bindSocket();
public:
	Networking(SocketType type, SocketFamily family, SocketUser user, int port = 0);
	~Networking();

	int receiveDataCallback(callbackFuction callback, int bufferLength);
	int receiveData(char* buffer, int bufferLength);

	int sendData(char* data, struct sockaddr_in location);
};


#endif // NETWORKING_H
