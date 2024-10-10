#include "../inc/Channel.hpp"

Channel::Channel()
{
	this->key = 0;
	this->topic = 0;
	this->max = 0;
	this->name = "";
	this->topic_n = "";
}

Channel::~Channel(){}

Channel::Channel(Channel const &src){
	*this = src;
}

Channel &Channel::operator=(Channel const &src){
	if(this != &src){
		this->key = src.key;
		this->topic = src.topic;
		this->max = src.max;
		this->name = src.name;
		this->topic_n = src.topic_n;
	}
	return *this;
}

void Channel::setTopic(int topic){
	this->topic = topic;
}

void Channel::setKey(int key){
	this->key = key;
}

void Channel::setMax(int max){
	this->max = max;
}

void Channel::setTopic_n(std::string topic_n){
	this->topic_n = topic_n;
}

void Channel::setName(std::string name){
	this->name = name;
}

void Channel::setPassword(std::string password){
	this->password = password;
}


std::string Channel::getTopic_N(){
	return this->topic_n;
}

std::string Channel::getPassword(){
	return this->password;
}

std::string Channel::getName(){
	return this->name;
}

int Channel::getTopic(){
	return this->topic;
}

int Channel::getMax(){
	return this->max;
}
int Channel::getClient_N(){
	return this->clients.size() + this->admins.size();
}
Client *Channel::getClient(int fd){
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getFd() == fd)
			return &(*it);
	}
	return NULL;
}

Client *Channel::getAdmin(int fd){
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it)
	{
		if (it->getFd() == fd)
			return &(*it);
	}
	return NULL;
}

void Channel::addClient(Client new_client){
	clients.push_back(new_client);
}

void Channel::addAdmin(Client new_client){
	admins.push_back(new_client);
}

void Channel::removeClient(int fd){
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->getFd() == fd)
		{
			clients.erase(it);
			break;
		}
	}
}


void Channel::removeAdmin(int fd){
	for (std::vector<Client>::iterator it = admins.begin(); it != admins.end(); ++it)
	{
		if (it->getFd() == fd)
		{
			admins.erase(it);
			break;
		}
	}
}