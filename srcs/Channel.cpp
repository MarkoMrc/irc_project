#include "../inc/Channel.hpp"

Channel::Channel()
{
	this->key = 0;
	this->topic = 0;
	this->limit = 10;
	this->name = "";
	this->topic_n = "";
	this->modeLimit = false;
	this->modePasswordProtected = false;
	this->modeInviteOnly = false;
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

void Channel::setLimit(size_t limit){
	limit = limit;
}

int Channel::getClient_N(){
	return this->clients.size() + this->admins.size();
}

Client *Channel::getClient(int fd){
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ((*it)->getFd() == fd)
			return (*it);
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

void Channel::addClient(Client* new_client){
	clients.push_back(new_client);
	std::cout << "Client FD: " << new_client->getFd() << " ajouté au canal." << std::endl;
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
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << "Testing socket: " << (*it)->getFd() << " against client socket: " << client.getFd() << std::endl;
		if ((*it)->getFd() == client.getFd()) {
			return true;
		}
	}
	return false;
}

void Channel::removeClient(int fd) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ) {
		if ((*it)->getFd() == fd) {
			std::cout << "element supprimer" << std::endl;
			// delete *it; // Si vous devez libérer la mémoire (si les pointeurs sont alloués dynamiquement)
			it = clients.erase(it); // Supprime le client et obtient l'itérateur vers le nouvel élément
		} else {
			++it; // On avance l'itérateur seulement si aucun élément n'a été supprimé
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


std::vector<Client*>& Channel::getClients(){
	return this->clients;
}

void Channel::broadcastMessage(const std::string& message, Client* excludeClient) {
	// Parcours du vecteur de clients en utilisant un itérateur
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		Client* client = *it;
		
		// Si excludeClient est défini, on exclut ce client de la diffusion
		if (excludeClient != NULL && client == excludeClient) {
			continue;
		}
		
		// Envoi du message au client
		send(client->getFd(), message.c_str(), message.length(), 0);
	}
}


bool Channel::isFull() const {
		return clients.size() >= limit; // Vérifie si le canal est plein
}

void Channel::inviteClient(Client* client) {
		// Ajoute un client à la liste des invités
		invitedClients.push_back(client);
}

bool Channel::isInvited(Client* client) const {
        // Vérifie si le client est dans la liste des invités
        return std::find(invitedClients.begin(), invitedClients.end(), client) != invitedClients.end();
    }