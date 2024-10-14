#include "../inc/Server.hpp"

// CAP LS
void Server::handleCapLs(int socket) {
    std::cout << "Commande CAP LS reçue" << std::endl;
	Client new_client;
	new_client.setFd(socket);  // Assigner le socket au client
	clients.push_back(new_client);  // Ajouter le client à la liste des clients
	Client *clients = getClient(socket);
    if (clients){
		std::cout << "erreur !client" << std::endl;
	};

    const char *cap_response = "CAP * LS :\r\n";
    send(socket, cap_response, strlen(cap_response), 0);
	std::cout << "CAP LS" << std::endl;
}

void Server::handlePass(int socket, const std::string& params) {
	std::string client_password = params.substr(0, params.find(' '));
    std::cout << "Commande PASS reçue avec params: " << params << std::endl;
	std::string mdp = getPassword();
	// std::cout << "client password " << client_password << std::endl;
	// std::cout << "mdp " << mdp << std::endl;
    if (client_password == mdp) {
        std::cout << "Mot de passe correct." << std::endl;
        const char *msg = "Mot de passe correct. Connexion acceptée.\r\n";
        send(socket, msg, strlen(msg), 0);
    } else {
        std::cout << "Mot de passe incorrect." << std::endl;
        const char *msg = "Mot de passe incorrect. Connexion refusée.\r\n";
        send(socket, msg, strlen(msg), 0);
        close(socket);  // Fermer la connexion en cas de mot de passe incorrect
    }
}

void Server::handleNick(int socket, const std::string& params) {
    std::cout << "Commande NICK reçue avec params: " << params << std::endl;
	Client *clients = getClient(socket);
    if (clients){
		std::cout << "erreur !client" << std::endl;
	};

    // clients->setNickname(params);  // Met à jour le surnom du client
    // std::cout << "Client socket " << socket << " set nickname to: " << params << std::endl;
}


void Server::handleUser(int socket, const std::string& params) {
    std::cout << "Commande USER reçue avec params: " << params << std::endl;
}


void Server::handleOper(int socket, const std::string& params) {
    std::cout << "Commande OPER reçue avec params: " << params << std::endl;
}

void Server::handleMode(int socket, const std::string& params) {
    std::cout << "Commande MODE reçue avec params: " << params << std::endl;
}

void Server::handleQuit(int socket) {
    std::cout << "Commande QUIT reçue" << std::endl;
	Client *clients = getClient(socket);
	std::cout << "socket " << getClient(socket) << std::endl;
    if (!clients) return;
	else
		std::cout << "tout good" << std::endl;

    // Retirer le client de tous les channels auxquels il est connecté
    for (Channel &channel : channels) {
        channel.removeClient(clients->getFd());
    }

    std::cout << "Client " << clients->getNickname() << " has quit." << std::endl;
	const char *msg = "You quit.\r\n";
    send(socket, msg, strlen(msg), 0);
    close(socket);  // Fermer le socket du client
	std::cout << "socket after close" << getClient(socket) << std::endl;
}

void Server::handleJoin(int socket, const std::string& params) {
    std::cout << "Commande JOIN reçue avec params: " << params << std::endl;
}

void Server::handlePart(int socket, const std::string& params) {
    std::cout << "Commande PART reçue avec params: " << params << std::endl;
}

void Server::handleTopic(int socket, const std::string& params) {
    std::cout << "Commande TOPIC reçue avec params: " << params << std::endl;
}

void Server::handleKick(int socket, const std::string& params) {
    std::cout << "Commande KICK reçue avec params: " << params << std::endl;
}

void Server::handlePrivmsg(int socket, const std::string& params) {
    std::cout << "Commande PRIVMSG reçue avec params: " << params << std::endl;
}