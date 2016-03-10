#include "../include/Networking.h"

Networking::Networking(SocketType type, SocketFamily family, SocketUser user, int port = 0) : serverFileDescriptor(0), clientFileDescriptor(0){
	if(!createSocket(type, family, user)){
			// throw: what(): Socket creation error
			;
	}
	struct sockaddr_in* address = &(user == SocketUser::CLIENT ? clientAddress : serverAddress);

	memset((char*)address, 0, sizeof(*address));
	address->sin_family = (family == SocketFamily::IPV4 ? AF_INET : AF_INET6);
    address->sin_port   = htons(port);
    address->sin_addr.s_addr = INADDR_ANY;

    slen = sizeof(*address);

	if(user == SocketUser::SERVER){
		if(!bindSocket()){
			// throw: what(): Bind socket error
			;
		}
	}
}

int Networking::createSocket(SocketType type, SocketFamily family, SocketUser user){
	if(((user == SocketUser::CLIENT ? clientFileDescriptor : serverFileDescriptor) = socket((family == SocketFamily::IPV4 ? AF_INET : AF_INET6), (type == SocketType::UDP ? SOCK_DGRAM : SOCK_STREAM), 0)) < 0){
		return 0;
	}
	return 1;
}

int Networking::bindSocket(){
	if(bind(serverFileDescriptor, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0){
		return 0;
	}
	return 1;
}

int Networking::receiveDataCallback(callbackFuction callback, int bufferLength){
	int receiveLen = 0;
	char buffer[bufferLength];

	while(true){
		if((receiveLen = recvfrom(serverFileDescriptor, buffer, bufferLength, 0, (struct sockaddr*)& clientAddress, &slen)) == -1){
			if(callback(buffer, receiveLen, 1)){
				continue;
			}else{
				break;
			}
		}
		if(!callback(buffer, receiveLen, 0)){
			break;
		}
	}
}

int Networking::receiveData(char* buffer, int bufferLength){
	if((receiveLen = recvfrom(serverFileDescriptor, buffer, bufferLength, 0, (struct sockaddr*)& clientAddress, &slen)) == -1){
		return NULL;
	}
}

int Networking::sendData(char* data, struct sockadd_in location){

}