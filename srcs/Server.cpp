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

bool Server::isRegistered(int socket) {
    Client* client = getClient(socket);
    if (client == nullptr) {
        std::cerr << "Client not found for socket: " << socket << std::endl;
        return false;
    }

    if (!client->getNickname().empty() && !client->getUsername().empty() && client->isLogged()) {
        return true;
    }
    return false;
}

void Server::authenticateClient(int socket, const std::string& password, const std::string& nickname, const std::string& username) {
    Client* client = getClient(socket); // Récupère le client via son socket
    if (client == nullptr) {
        std::cerr << "Client not found for socket: " << socket << std::endl;
        return;
    }

    // if (!password.empty() && password != this->password) {
    //     std::cerr << "Mot de passe incorrect pour le client sur le socket: " << socket << std::endl;
    //     // close(socket);
    //     // return;
    // }

    // Définit le pseudonyme et le nom d'utilisateur pour le client
	if (!nickname.empty() && !username.empty() && password == this->password){
    	client->setUsername(username);
    	client->setNickname(nickname);
    	client->setLogged(true); // Indique que le client est authentifié
		const char *authentication_message = "Vous êtes bien authentifié!\n";
        send(socket, authentication_message, strlen(authentication_message), 0);
    	std::cout << "Client authentifié avec le nickname: " << nickname << " et username: " << username << std::endl;
	}

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
		std::string pass, nick, user;
        std::string received_data(buffer, valread);
        std::cout << "===received data===" << std::endl << received_data << "===" << std::endl;

        // Séparer les commandes basées sur '\r\n' ou '\n' (IRC spécifie \r\n, mais certains clients peuvent utiliser \n seul)
        std::vector<std::string> commands;
        size_t pos = 0;
        while ((pos = received_data.find("\r\n")) != std::string::npos) {
            commands.push_back(received_data.substr(0, pos));
            received_data.erase(0, pos + 2); // Effacer la commande traitée
        }
        
        // Gérer chaque commande séparément
        for (size_t i = 0; i < commands.size(); i++) {
            std::string command_line = commands[i];
            std::istringstream iss(command_line);
            std::string command;
            std::getline(iss, command, ' '); // Premier token : la commande

            // Récupérer les paramètres après la commande
            std::string params;
            std::getline(iss, params); // Le reste après le premier espace

            // Diviser les paramètres en fonction des espaces
            std::istringstream paramStream(params);
            std::vector<std::string> paramList;
            std::string param;

        while (paramStream >> param) {
            paramList.push_back(param); // Ajoute chaque paramètre dans un vecteur
        }

        if (command == "CAP") {
            if (!paramList.empty() && paramList[0] == "LS") {
                handleCapLs(socket);
            }
        } else if (command == "PASS" || command == "PASS\r\n") {
            if (paramList.size() == 1 && paramList[0] != "\0") {
                handlePass(socket, paramList[0]);
				pass = paramList[0];
            } else {
                std::cerr << "Erreur: PASS nécessite 1 param" << std::endl;
            }
        } else if (command == "NICK") {
            // Vérifier qu'il n'y a qu'un seul paramètre pour NICK
            if (paramList.size() == 1) {
                handleNick(socket, paramList[0]);
            } else {
                std::cerr << "Erreur: NICK nécessite 1 param" << std::endl;
            }
        } else if (command == "USER") {
			// user = param;
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
		if (!getClient(socket)->isLogged() && !getClient(socket)->getNickname().empty() && !getClient(socket)->getUsername().empty()) {
			// std::cout << "condition isLogged: " << (getClient(socket)->isLogged() ? "true" : "false") << std::endl;
			nick = getClient(socket)->getNickname();
			// std::cout << "nickname handleconnection " + nick << std::endl;
			// std::cout << "password handleconnection " + pass << std::endl;
            user = getClient(socket)->getUsername();
			// std::cout << "nickname handleconnection " + nick << std::endl;
			authenticateClient(socket, pass, nick, user);
			// std::cout << "after authentication condition isLogged: " << (getClient(socket)->isLogged() ? "true" : "false") << std::endl;
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
}