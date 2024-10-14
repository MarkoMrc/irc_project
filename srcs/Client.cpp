#include "../inc/Client.hpp"

Client::Client()
{
	this->fd = -1;
	this->username = "";
	this->nickname = "";
	this->ip_add = "";
	this->logged = false;
	this->registered = false;
	this->buff = "";
	this->isOperator = false;
}

Client::Client(std::string username, std::string nickname, int fd) : username(username), nickname(nickname), fd(fd) {}

Client::~Client(){}

Client::Client(Client const &src){
	*this = src;
}

Client &Client::operator=(Client const &src){
	if (this != &src){
		this->fd = src.fd;
		this->username = src.username;
		this->nickname = src.nickname;
		this->buff = src.buff;
		this->registered = src.registered;
		this->logged = src.logged;
		this->ip_add = src.ip_add;
	}
	return *this;
}

std::string Client::getUsername(){
	return this->username;
}

std::string Client::getNickname(){
	return this->nickname;	
}

int Client::getFd(){
	return this->fd;
}

std::string Client::getIp_add(){
	return this->ip_add;
}

std::string Client::getBuff(){
	return this->buff;
}

std::string Client::getHostname(){
	return this->hostname;
}

void Client::setUsername(const std::string& username){
	this->username = username;
}

void Client::setBuff(std::string buff){}

void Client::setFd(int fd){}

void Client::setIp_add(std::string ip_add){}

void Client::setNickname(const std::string& nickname){
	this->nickname = nickname;
}

void Client::setHostname(const std::string& hostname){
	this->hostname = hostname;
}

void Client::setIsOperator(bool isOperator){
	this->isOperator = isOperator;
}