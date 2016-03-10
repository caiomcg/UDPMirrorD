#ifndef NETWORKIN_ERRORS_H
#define NETWORKIN_ERRORS_H

#include <stdexcept>
#include <exception>
#include <string>

class SocketFailedException: public std::runtime_error{
public:
	SocketFailedException():runtime_error("Failed to create socket"){}
	SocketFailedException(std::string msg):runtime_error(msg.c_str()){}
};

class BindFailedException: public std::runtime_error{
public:
	BindFailedException():runtime_error("Failed to bind socket"){}
	BindFailedException(std::string msg):runtime_error(msg.c_str()){}
};
#endif // NETWORKIN_ERRORS_H
