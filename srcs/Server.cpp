#include "../inc/Server.hpp"

Server *Server::server = NULL;

Server::Server(/* args */){
	this->server_socket_fd = -1;
	setFirstConnexion(true);
	
}

Server::~Server(){
	//ajouter methode nettoyage Channel
	// if (this->epoll_fd >= 0) {
    // close(this->epoll_fd);
	// }
	// if (this->server_socket_fd >= 0) {
    // close(this->server_socket_fd);
	// }
}


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

bool Server::getFirstConnexion() {
	return this->firstConnexion;
}

std::string Server::getPassword(){
	return this->password;
}

Server *Server::getServer() {
	if (Server::server == NULL) {
		Server::server = new Server();
	}
	return Server::server;
}

Client* Server::getClient(int fd) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getFd() == fd) {
			return (*it);
		}
	}
	return NULL;
}

Client* Server::getClient(const std::string& nickname) {
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getNickname() == nickname) {
			return (*it);
		}
	}
	return NULL;
}

Channel *Server::getChannel(std::string name){
	for (size_t i = 0; i < this->channels.size(); i++)
	{
		if (this->channels[i]->getName() == name)
			return this->channels[i];
	}
	return NULL;
}

void Server::setFd(int fd_socket){
	this->server_socket_fd = fd_socket;
}

void Server::setPort(int port){
	this->port = port;
}

void Server::setFirstConnexion(bool fc) {
	this->firstConnexion = fc;
}

void Server::setPassword(std::string password){
	this->password = password;
}


void Server::addClient(Client* new_client){
	this->clients.push_back(new_client);
}

void Server::addChannel(Channel* new_channel){
	this->channels.push_back(new_channel);
}


bool Server::isRegistered(int socket) {
	Client* client = getClient(socket);
	if (client == NULL) {
		std::cerr << "Client not found for socket: " << socket << std::endl;
		return false;
	}

	if (!client->getNickname().empty() && !client->getUsername().empty() && client->isLogged()) {
		return true;
	}
	return false;
}

void Server::authenticateClient(int socket, const std::string& password, const std::string& nickname, const std::string& username) {
	Client* client = getClient(socket); // Recupere le client via son socket
	if (!client){
		std::cout << "erreur !client : "<< getClient(socket) << std::endl;
		return ;
	}
	(void) password;
	if (client == NULL) {
		std::cerr << "Client not found for socket: " << socket << std::endl;
		return;
	}

	if (!nickname.empty() && !username.empty() && client->isPswdEnterd()){
		client->setUsername(username);
		client->setNickname(nickname);
		client->setLogged(true); // Indique que le client est authentifie
		const char *authentication_message = "Vous êtes bien authentifie!\n";
		send(socket, authentication_message, strlen(authentication_message), 0);
		std::cout << "Client authentifie avec le nickname: " << nickname << " et username: " << username << std::endl;
	}
}

bool Server::isNewClient(int client_socket) const {
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getFd() == client_socket) {
			return false; // Le client existe déjà
		}
	}
	return true; // Le client est nouveau
}

void Server::acceptClient() {
	int client_socket = createClientSocket();
	if (client_socket < 0) return;

	if (!handleNewClient(client_socket)) return;

	setSocketBlockingMode(client_socket);
	addSocketToEpoll(client_socket);
	createAndAddClient(client_socket);
}

int Server::createClientSocket() {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_socket = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	if (client_socket < 0) {
		std::cerr << "Erreur lors de accept()" << std::endl;
		return -1;
	}
	std::cout << "Nouvelle connexion acceptée, socket client : " << client_socket << std::endl;
	return client_socket;
}

bool Server::handleNewClient(int client_socket) {
	// if (!isNewClient(client_socket)) {
	// 	std::cout << "Client déjà existant avec socket : " << client_socket << std::endl;
	// 	setFirstConnexion(false);
	// 	return false;  // Client déjà existant
	// }
	(void) client_socket;
	setFirstConnexion(true);
	return true;  // Nouveau client
}

void Server::addSocketToEpoll(int client_socket) {
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = client_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev) == -1) {
		perror("Erreur lors de epoll_ctl: client_socket");
		exit(EXIT_FAILURE);
	}
}

void Server::createAndAddClient(int client_socket) {
	Client* new_client = new Client();
	new_client->setFd(client_socket);  // Setter pour définir le socket du client
	new_client->setPswdEnterd(false);
	// Ajouter le client à la liste des clients du serveur
	addClient(new_client);
	clients.push_back(new_client);
}

// Definir une methode pour rendre un socket non bloquant
void Server::setSocketBlockingMode(int socket_fd) {
	if (fcntl(socket_fd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl(F_SETFL a echoué)");
		exit(EXIT_FAILURE);
	}
}

void Server::serv_init(int port, std::string password) {
	std::cout << "INIT" << std::endl;
	this->password = password;
	this->port = port;

	createSocket();
	configureSocket();
	bindSocket();
	startListening();
	setSocketBlockingMode(server_socket_fd);
	initializeEpoll();
	std::cout << "Serveur IRC démarré. En attente de connexions..." << std::endl;
}

void Server::createSocket() {
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket_fd == -1) {
		std::cerr << "Erreur lors de la création du socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	this->server_socket_fd = server_socket_fd;
}

void Server::configureSocket() {
	int opt = 1;
	if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("échec de setsockopt");
		exit(EXIT_FAILURE);
	}
}

void Server::bindSocket() {
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr)); // Initialiser la structure à zéro
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; // Accepter les connexions sur toutes les interfaces
	server_addr.sin_port = htons(port); // Convertir le port en format réseau

	if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Erreur lors de la liaison du socket" << std::endl;
		close(server_socket_fd);
		exit(EXIT_FAILURE);
	}
}

void Server::startListening() {
	if (listen(server_socket_fd, 10) < 0) {
		std::cerr << "Erreur lors de l'écoute des connexions" << std::endl;
		close(server_socket_fd);
		exit(EXIT_FAILURE);
	}
}

void Server::initializeEpoll() {
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("Erreur de epoll_create");
		exit(EXIT_FAILURE);
	}
	this->epoll_fd = epoll_fd;
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = server_socket_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &ev) == -1) {
		perror("Erreur lors de epoll_ctl: server_socket_fd");
		exit(EXIT_FAILURE);
	}
}

void Server::run() {
	struct epoll_event events[MAX_EVENTS];

	while (true) {
		// Attendre les evenements sur epoll
		int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (event_count == -1) {
			perror("Erreur lors de epoll_wait");
			exit(EXIT_FAILURE);
		}

		// Parmi nouveaux events
		for (int i = 0; i < event_count; ++i) {
			// Si nouvelle connexion
			if (events[i].data.fd == server_socket_fd) {
				acceptClient();
			}
			// Si new data
			else {
				int client_socket = events[i].data.fd;
				handleConnection(client_socket);
			}
		}
	}
}

void Server::closing_sockets()
{
	std::cout << "closing sockets" << std::endl;
	close(server_socket_fd);
	close(epoll_fd);
	std::vector<Client*>::iterator it;
	size_t i = 0;
	for (it = clients.begin(); it != clients.end(); ++it) {
		if (i % 2 == 0) {
			close((*it)->getFd());
			delete (*it);
		}
		i++;
	}

}

std::vector<Client*>& Server::getClients(){
	return this->clients;
}

void Server::removeClient(Client* client) {
	if (!client) {
		std::cerr << "Erreur: tentative de suppression d'un client nul." << std::endl;
		return;
	}

	size_t i = 0;
	// Cherchez le client dans la liste
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (i % 2 + 1 == 0) {
			if (*it == client) {
			// Fermer le socket du client
				close(client->getFd());

				// Supprimer le client de la liste
				clients.erase(it);
				delete client; // Libération de la mémoire
				std::cout << "Client supprimé de la liste." << std::endl;
				return;
			}

			std::cerr << "Erreur: client non trouvé dans la liste." << std::endl;
		} 
		i++;
	}
}

