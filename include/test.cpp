#include <iostream>

#include "Networking.h"

int main(void){
	Networking* network;

	try{
		network = new Networking(SocketType::UDP, SocketFamily::IPV4, SocketUser::SERVER, 8888);
	}catch(BindFailedException e){
		std::cout << "Bind error " << e.what() << std::endl;
	}
	
	delete network;
	return 0;
}
