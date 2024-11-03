#include "../inc/Client.hpp"

Client::Client()
{
	this->fd = 0;
	this->username = "";
	this->nickname = "";
	this->ip_add = "";
	this->logged_in = false;
	this->registered = false;
	this->buff = "";
	this->isOperator = false;
}

// Client::Client(std::string username, std::string nickname, int fd) : username(username), nickname(nickname), fd(fd) {}

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
		this->logged_in = src.logged_in;
		this->ip_add = src.ip_add;
	}
	return *this;
}

std::string Client::getUsername(){
	return this->username;
}

std::string Client::getNickname() const{
	return this->nickname;	
}

int Client::getFd() const {
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

std::string Client::getTmpNick() {
	return this->tmp_nick;
}

std::string Client::getTmpUser() {
	return this->tmp_user;
}

void Client::setUsername(const std::string& username){
	this->username = username;
}

void Client::setBuff(std::string buff){
	(void) buff;
}

void Client::setFd(int fd){
	this->fd = fd;
}

void Client::setIp_add(std::string ip_add){
	(void) ip_add;
}

void Client::setNickname(const std::string& nickname){
	this->nickname = nickname;
}

void Client::setHostname(const std::string& hostname){
	this->hostname = hostname;
}

void Client::setTmpNick(const std::string& tmp_nick) {
	this->tmp_nick = tmp_nick;
}

void Client::setTmpUser(const std::string& tmp_user) {
	this->tmp_user = tmp_user;
}

void Client::setIsOperator(bool isOperator){
	this->isOperator = isOperator;
}

void Client::setLogged(bool status) {
	this->logged_in = status;
}

void Client::setPswdEnterd(bool status) {
	this->pswd_entered = status;
}


bool Client::isLogged() const {
	return this->logged_in;
}

bool Client::isPswdEnterd() const {
	return this->pswd_entered;
}
