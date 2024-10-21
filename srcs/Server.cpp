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
			return &(*it);
		}
	}
	return NULL;
}

Client* Server::getClient(const std::string& nickname) {
	for (std::vector<Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->getNickname() == nickname) {
			return &(*it);
		}
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

bool Server::askPassword(int socket)
{
	const char *ask_password = "Veuillez entrer le mot de passe:\n";
	send(socket, ask_password, strlen(ask_password), 0);

	// Lire la reponse du client
	char buffer[1024] = {0};
	int valread = recv(socket, buffer, 1024, 0);

	std::cout << "test :" << buffer << std::endl;

	// Gerer le cas ou la reception echoue ou la connexion est fermee
	if (valread <= 0)
	{
		std::cerr << "Erreur lors de la reception ou connexion fermee." << std::endl;
		return false;
	}
	if (valread > 0)
	{
		std::cout << "buffer :" << buffer << std::endl;
	}

	// Supprimer le saut de ligne s'il y en a
	if (buffer[valread - 1] == '\n' || buffer[valread - 1] == '\r' )
		buffer[valread - 1] = '\0';  // Pour enlever le '\n' ajoute par le client

	// Comparer le mot de passe fourni avec celui du serveur
	if (strcmp(password.c_str(), buffer) == 0)
	{
		std::cout << "test :" << buffer << std::endl;
		std::cout << "Mot de passe correct. Connexion autorisee." << std::endl;
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
		std::cout << "Mot de passe incorrect. Connexion refusee." << std::endl;
		(void) valread;
		return false;
	}
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
	if (client == NULL) {
		std::cerr << "Client not found for socket: " << socket << std::endl;
		return;
	}

    // if (!password.empty() && password != this->password) {
    //     std::cerr << "Mot de passe incorrect pour le client sur le socket: " << socket << std::endl;
    //     // close(socket);
    //     // return;
    // }

    // Definit le pseudonyme et le nom d'utilisateur pour le client
	if (!nickname.empty() && !username.empty() && password == this->password){
    	client->setUsername(username);
    	client->setNickname(nickname);
    	client->setLogged(true); // Indique que le client est authentifie
		const char *authentication_message = "Vous êtes bien authentifie!\n";
		send(socket, authentication_message, strlen(authentication_message), 0);
		std::cout << "Client authentifie avec le nickname: " << nickname << " et username: " << username << std::endl;
	}
}

void Server::acceptClient() {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);
	int client_socket = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	if (client_socket < 0) {
		std::cerr << "Erreur lors de accept()" << std::endl;
		return;
	}
	std::cout << "Nouvelle connexion acceptee, socket client : " << client_socket << std::endl;

	// Rendre le socket client non bloquant
	setSocketBlockingMode(client_socket);

	// rajout du fd socket client a epoll pour surveiller les evenements in
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = client_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &ev) == -1) {
		perror("Erreur lors de epoll_ctl: client_socket");
		exit(EXIT_FAILURE);
	}

	// Creer un nouvel objet Client avec le socket
	Client new_client;
	new_client.setFd(client_socket);  // Setter pour definir le socket du client
	new_client.setPswdEnterd(false);
	// Ajouter le client a la liste des clients du serveur
	addClient(new_client);
}

// Definir une methode pour rendre un socket non bloquant
void Server::setSocketBlockingMode(int socket_fd) {
	int flags = fcntl(socket_fd, F_GETFL, 0);
	fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::serv_init(int port, std::string password) {
	std::cout << "INIT" << std::endl;
	this->password = password;
	this->port = port;
	int opt = 1;

	// Creation du socket
	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket_fd == -1) {
		std::cerr << "Erreur lors de la creation du socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	// Configurer le socket
	if (setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		perror("echec de setsockopt");
		exit(EXIT_FAILURE);
	}

	// Configuration de l'adresse du serveur
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr)); // Initialiser la structure a zero
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY; // Accepter les connexions sur toutes les interfaces
	server_addr.sin_port = htons(port); // Convertir le port en format reseau

	// Lier le socket a l'adresse
	if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		std::cerr << "Erreur lors de la liaison du socket" << std::endl;
		close(server_socket_fd);
		exit(EXIT_FAILURE);
	}

	// ecoute des connexions entrantes
	if (listen(server_socket_fd, 10) < 0) {
		std::cerr << "Erreur lors de l'ecoute des connexions" << std::endl;
		close(server_socket_fd);
		exit(EXIT_FAILURE);
	}

	// Rendre le socket serveur non bloquant
	setSocketBlockingMode(server_socket_fd);

	// Initialisation de epoll
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1) {
		perror("Erreur de epoll_create");
		exit(EXIT_FAILURE);
	}

	// Ajouter le socket serveur a epoll pour surveiller les evenements d'entree
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = server_socket_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket_fd, &ev) == -1) {
		perror("Erreur lors de epoll_ctl: server_socket_fd");
		exit(EXIT_FAILURE);
	}

	std::cout << "Serveur IRC demarre. En attente de connexions..." << std::endl;
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

void Server::handleConnection(int socket) {
	bool fc = true;
	bool fcn = true;
	bool fcu = true;
	Client *client = getClient(socket);
	char buffer[1024] = {0};
	int valread = recv(socket, buffer, sizeof(buffer), 0);

	if (valread > 0) {
		std::string received_data(buffer, valread);
		std::string pass;
		std::cout << "===received data===" << std::endl << received_data << "===" << std::endl;

		// Separer les commandes basees sur '\r\n'
		std::vector<std::string> commands;
		// while ((pos = received_data.find("\r\n")) != std::string::npos) {
		//     commands.push_back(received_data.substr(0, pos));
		//     received_data.erase(0, pos + 2); // Effacer la commande traitee
		// }
		while (true) {
		// trouver la position de la premiere terminaison de ligne
			size_t crlf_pos = received_data.find("\r\n");
			size_t lf_pos = received_data.find("\n");

			if (crlf_pos == std::string::npos && lf_pos == std::string::npos) {
				break;
			}

			// connaitre la position de la premiere terminaison de ligne
			size_t end_pos;
			if (crlf_pos != std::string::npos && (lf_pos == std::string::npos || crlf_pos < lf_pos)) {
				end_pos = crlf_pos; // Utiliser '\r\n'
			} else {
				end_pos = lf_pos; // Utiliser '\n'
			}

			// Ajouter la commande a la liste
			commands.push_back(received_data.substr(0, end_pos));

			// Effacer la commande traitee
			received_data.erase(0, end_pos + (received_data[end_pos] == '\r' ? 2 : 1));
		}

		// Gerer chaque commande separement
		for (std::vector<std::string>::iterator it = commands.begin(); it != commands.end(); ++it) {
			std::istringstream iss(*it);
			std::string command;
			std::getline(iss, command, ' '); // Premier token : la commande

			// Recup les param apres la commande
			std::string params;
			std::getline(iss, params); //

			// Diviser les parametres
			std::istringstream paramStream(params);
			std::vector<std::string> paramList;
			std::string param;

			while (paramStream >> param) {
				paramList.push_back(param);
			}

			if (command == "CAP") {
				if (!paramList.empty() && paramList[0] == "LS") {
					handleCapLs(socket);
				}
			} else if (command == "PASS") {
				if (paramList.size() == 1) {
					fc = fcn && fcu;
					std::cout << "nick = " << client->getTmpNick() << std::endl;
					handlePass(socket, paramList[0], fc, client->getTmpNick(), client->getTmpUser());
					pass = paramList[0];
					fc = false;
				} else {
					std::cerr << "Erreur: PASS necessite 1 param" << std::endl;
				}
			} else if (command == "NICK") {
				std::cout << "commande NICK" << std::endl;
				if (paramList.size() == 1) {
					if (fcn) {

						client->setTmpNick(paramList[0]);
						fcn = false;
						const char *msg = "Veuillez entrer le mot de passe (PASS mdp) \r\n";
						send(socket, msg, strlen(msg), 0);
					}
					else
						handleNick(socket, paramList[0]);
				} else {
					std::cerr << "Erreur: NICK necessite 1 param" << std::endl;
				}
			} else if (command == "USER") {
				if (fcu) {
					client->setTmpUser(params);
					fcu = false;
				}
				else
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

			// std::cout << "isLogged: " << (getClient(socket)->isLogged() ? "true" : "false") << std::endl;
			// std::cout << "condition isLogged: " << (getClient(socket)->isLogged() ? "true" : "false") << std::endl;
			if (!getClient(socket)->isLogged() && !getClient(socket)->getNickname().empty() && !getClient(socket)->getUsername().empty()) {
				std::string nick = getClient(socket)->getNickname();
			// std::cout << "password handleconnection " + pass << std::endl;
				std::string user = getClient(socket)->getUsername();
			// std::cout << "nickname handleconnection " + nick << std::endl;
				authenticateClient(socket, pass, nick, user);
			// std::cout << "after authentication condition isLogged: " << (getClient(socket)->isLogged() ? "true" : "false") << std::endl;
			}
		}
	} else if (valread == 0) {
		std::cout << "Client deconnecte, socket: " << socket << std::endl;
		close(socket);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, NULL);  // delete epoll socket
	} else if (valread < 0) {
		std::cerr << "Erreur de reception sur le socket" << std::endl;
	}
}

void Server::closing_sockets()
{
	close(server_socket_fd);
	std::vector<Client>::iterator it;
	for (it = clients.begin(); it != clients.end(); it++) {
		close((*it).getFd());
	}
}
