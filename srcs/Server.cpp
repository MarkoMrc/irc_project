#include "../inc/Server.hpp"

Server::Server(/* args */){
	this->server_socket_fd = -1;
}

Server::~Server(){}

Server::Server(Server const &src){
	*this = src;
}
Server &Server::operator=(Server const &src){
	if (this != &src)
	{
		this->channels = src.channels;
		this->clients = src.clients;
		this->password = src.password;
		this->port = src.port;
	}
	return *this;
}

int Server::getFd(){
	return this->server_socket_fd;
}

int Server::getPort(){
	return this->port;
}

std::string Server::getPassword(){
	return this->password;
}

Client *Server::getClient(int fd){
	for (size_t i = 0; i < this->clients.size(); i++)
	{
		if (this->clients[i].getFd() == fd)
			return &this->clients[i];
	}
	return NULL;
}
Channel *Server::getChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i].getName() == name)
			return &channels[i];
	}
	return NULL;
}

void Server::setFd(int fd_socket){
	this->server_socket_fd = fd_socket;
}

void Server::setPort(int port){
	this->port = port;
}

void Server::setPassword(std::string password){
	this->password = password;
}


void Server::addClient(Client new_client){
	this->clients.push_back(new_client);
}

void Server::addChannel(Channel new_channel){
	this->channels.push_back(new_channel);
}

void Server::serv_init(int port, std::string password){
	this->password = password;
	this->port = port;
}


bool Server::askPassword(int socket)
{
	const char *ask_password = "Veuillez entrer le mot de passe:\n";
    send(socket, ask_password, strlen(ask_password), 0);

    // Lire la réponse du client
    char buffer[1024] = {0};
    int valread = recv(socket, buffer, 1024, 0);

    // Gérer le cas où la réception échoue ou la connexion est fermée
    if (valread <= 0)
	{
		std::cerr << "Erreur lors de la réception ou connexion fermée." << std::endl;
		return false;
	}

    // Supprimer le saut de ligne s'il y en a
	if (buffer[valread - 1] == '\n')
    	buffer[valread - 1] = '\0';  // Pour enlever le '\n' ajouté par le client

    // Comparer le mot de passe fourni avec celui du serveur
    if (strcmp(password.c_str(), buffer) == 0)
	{
        std::cout << "Mot de passe correct. Connexion autorisée." << std::endl;
        const char *welcome_message = "Bienvenue sur ce serveur IRC!\n";
        send(socket, welcome_message, strlen(welcome_message), 0);
		return true;
    }
	else
	{
        std::cout << "Mot de passe incorrect. Connexion refusée." << std::endl;
		return false;
	}
}

void Server::setSocketBlockingMode(int socket, bool blocking) {
    int flags = fcntl(socket, F_GETFL, 0);  // Obtenir les flags actuels
    if (blocking) {
        // Si tu veux un mode bloquant, supprime le flag O_NONBLOCK
        flags &= ~O_NONBLOCK;
    } else {
        // Si tu veux un mode non-bloquant, ajoute le flag O_NONBLOCK
        flags |= O_NONBLOCK;
    }
    fcntl(socket, F_SETFL, flags);  // Appliquer les nouveaux flags
}