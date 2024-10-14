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

Client* Server::getClient(int fd) {
    for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->getFd() == fd) {
            return &(*it);  //pointeur vers le client trouvé
        }
    }
    return nullptr;
}

Client* Server::getClient(const std::string& nickname) {
    for (auto& client : clients) {
        if (client.getNickname() == nickname) {
            return &client;  //pointeur vers le client trouvé
        }
    }
    return nullptr;
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
	std::cout << "INIT" << std::endl;
	this->password = password;
	this->port = port;
	int opt = 1;

	// Création du socket
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        std::cerr << "Erreur lors de la création du socket" << std::endl;
        exit(EXIT_FAILURE);
    }

	// Configurer le socket
    if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
        perror("Échec de setsockopt");
        exit(EXIT_FAILURE);
    }
    // Configuration de l'adresse du serveur
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // Initialiser la structure à zéro
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accepter les connexions sur toutes les interfaces
    server_addr.sin_port = htons(port); // Convertir le port en format réseau
    // Lier le socket à l'adresse
    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erreur lors de la liaison du socket" << std::endl;
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }
    // Écoute des connexions entrantes
    if (listen(server_socket_fd, 10) < 0) {
        std::cerr << "Erreur lors de l'écoute des connexions" << std::endl;
        close(server_socket_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Serveur IRC démarré. En attente de connexions..." << std::endl;
}

void Server::run(){
	while (true){
		acceptClient();
	}
}

void Server::acceptClient() {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	std::cout << "client_socket : "<< client_socket << std::endl;
    if (client_socket < 0) {
        std::cerr << "Error accepting client connection" << std::endl;
        return;
    }

    // Crée un nouvel objet Client avec le socket
    Client new_client;
    new_client.setFd(client_socket);  // Setter pour définir le socket du client
	std::cout << "new_client : "<< new_client.getFd() << std::endl;

    // Ajoute le client à la liste des clients du serveur
    addClient(new_client);

    // Gère la connexion
    handleConnection(client_socket);
}

bool Server::askPassword(int socket)
{
	const char *ask_password = "Veuillez entrer le mot de passe:\n";
    send(socket, ask_password, strlen(ask_password), 0);

    // Lire la réponse du client
    char buffer[1024] = {0};
    int valread = recv(socket, buffer, 1024, 0);

	std::cout << "test :" << buffer << std::endl;

    // Gérer le cas où la réception échoue ou la connexion est fermée
    if (valread <= 0)
	{
		std::cerr << "Erreur lors de la réception ou connexion fermée." << std::endl;
		return false;
	}
	if (valread > 0)
	{
		std::cout << "buffer :" << buffer << std::endl;
	}

    // Supprimer le saut de ligne s'il y en a
	if (buffer[valread - 1] == '\n' || buffer[valread - 1] == '\r' )
    	buffer[valread - 1] = '\0';  // Pour enlever le '\n' ajouté par le client

    // Comparer le mot de passe fourni avec celui du serveur
    if (strcmp(password.c_str(), buffer) == 0)
	{
		std::cout << "test :" << buffer << std::endl;
        std::cout << "Mot de passe correct. Connexion autorisée." << std::endl;
        const char *welcome_message = "Bienvenue sur ce serveur IRC!\n";
        send(socket, welcome_message, strlen(welcome_message), 0);
		return true;
    }
	else
	{
		const char *ask_password = "Veuillez entrer le mot de passe:\n";
    	send(socket, ask_password, strlen(ask_password), 0);
		int valread = recv(socket, buffer, 1024, 0);
		std::cout << "test :" << buffer << std::endl;
        std::cout << "Mot de passe incorrect. Connexion refusée." << std::endl;
		return false;
	}
}

#include <fcntl.h>

void Server::setSocketBlockingMode(int socket, int blocking)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1) return;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    fcntl(socket, F_SETFL, flags);
}

void Server::handleConnection(int socket) {
    char buffer[1024] = {0};
    int valread;

    while ((valread = recv(socket, buffer, 1024, 0)) > 0) {
        std::string received_data(buffer, valread);

		// j'enleve le CRLF
		// size_t pos;
        // while ((pos = received_data.find('\r')) != std::string::npos) {
        //     received_data.erase(pos, 1);
        // }
        // while ((pos = received_data.find('\n')) != std::string::npos) {
        //     received_data.erase(pos, 1);
        // }

        // je recup les cmd
        // std::string command = received_data.substr(0, received_data.find(' '));
        // std::string params = received_data.substr(received_data.find(' ') + 1);
		std::istringstream iss(received_data);
		std::cout << "received data" << received_data << std::endl;
        std::string command;
        std::getline(iss, command, ' '); // Premier token : la commande

        // Récupérer les paramètres après la commande
        std::string params;
        std::getline(iss, params); // Le reste après le premier espace

        if (command == "CAP") {
            if (params.substr(0, 2) == "LS") {
                handleCapLs(socket);
            }
        } else if (command == "PASS") {
            handlePass(socket, params);
        } else if (command == "NICK") {
            handleNick(socket, params);
        } else if (command == "USER") {
            handleUser(socket, params);
        } else if (command == "OPER") {
            handleOper(socket, params);
        } else if (command == "MODE") {
            handleMode(socket, params);
        } else if (command == "QUIT") {
            handleQuit(socket);
        } else if (command == "JOIN") {
            handleJoin(socket, params);
        } else if (command == "PART") {
            handlePart(socket, params);
        } else if (command == "TOPIC") {
            handleTopic(socket, params);
        } else if (command == "KICK") {
            handleKick(socket, params);
        } else if (command == "PRIVMSG") {
            handlePrivmsg(socket, params);
        } else {
            std::cerr << "Commande inconnue: " << command << std::endl;
        }

        memset(buffer, 0, sizeof(buffer)); // buffer set à 0 pour la prochaine boucle
    }
    if (valread == 0) {
        std::cout << "disconnected" << std::endl;
        close(socket);
    } else if (valread < 0) {
        std::cerr << "Error" << std::endl;
    }
}