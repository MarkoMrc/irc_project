#include "../inc/Channel.hpp"

Channel::Channel()
{
	this->key = 0;
	this->topic = 0;
	this->limit = 0;
	this->name = "";
	this->topic_n = "";
	this->modePasswordProtected = false;
}

Channel::~Channel(){}

Channel::Channel(Channel const &src){
	*this = src;
}

Channel &Channel::operator=(Channel const &src){
	if(this != &src){
		this->key = src.key;
		this->topic = src.topic;
		this->limit = src.limit;
		this->name = src.name;
		this->topic_n = src.topic_n;
	}
	return *this;
}

void Channel::setTopic(int topic){
	this->topic = topic;
}

void Channel::setTopic_n(std::string topic_n){
	this->topic_n = topic_n;
}

void Channel::setName(std::string name){
	this->name = name;
}

std::string Channel::getTopic_N(){
	return this->topic_n;
}

std::string Channel::getPassword() const {
	return this->password;
}

std::string Channel::getName(){
	return this->name;
}

int Channel::getTopic(){
	return this->topic;
}

void Channel::setModeInviteOnly(bool value){ 
	modeInviteOnly = value;
}

void Channel::setModeTopicOp(bool value){
	modeTopicOp = value;
}

void Channel::setModePasswordProtected(bool value){
	modePasswordProtected = value;
}

void Channel::setModeLimit(bool value){
	modeLimit = value;
}

void Channel::setPassword(const std::string& pass){
	password = pass;
}

void Channel::setLimit(int limit){
	limit = limit;
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

bool Channel::isAdmin(const Client& client) const {
    // Parcourir la liste des opérateurs et vérifier si le client est présent
    for (std::vector<Client>::const_iterator it = admins.begin(); it != admins.end(); ++it) {
        if (it->getFd() == client.getFd()) {
            return true; // Le client est un opérateur
        }
    }
    return false; // Le client n'est pas un opérateur
}

bool Channel::isClient(const Client& client) const {
	for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->getFd() == client.getFd()) {
			return true;
		}
	}
	return false;
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


std::vector<Client>& Channel::getClients() {
	return clients;
}