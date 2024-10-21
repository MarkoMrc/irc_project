#include "../inc/Server.hpp"

// CAP LS
void Server::handleCapLs(int socket) {
	std::cout << "Commande CAP LS reçue" << std::endl;
	Client *client = getClient(socket);
	if (!client){
		std::cout << "erreur !client : "<< getClient(socket) << std::endl;
	};

	// const char *cap_response = "CAP * LS :\r\n";
	// send(socket, cap_response, strlen(cap_response), 0);
	// std::cout << "CAP LS" << std::endl;
}

void Server::handlePass(int socket, const std::string& params) {
	if (params[0] != ' ') {
		std::string client_password = params.substr(0, params.find(' '));
		std::cout << "Commande PASS reçue avec params: " << params << std::endl;
		std::string mdp = getPassword();
		// std::cout << "client password " << client_password << std::endl;
		// std::cout << "mdp " << mdp << std::endl;
		if (client_password == mdp) {
			std::cout << "Mot de passe correct." << std::endl;
			Client *client = getClient(socket);
			client->setPswdEnterd(true);
			const char *msg = "Mot de passe correct. Connexion acceptee.\r\n";
			send(socket, msg, strlen(msg), 0);
		}
		else {
			std::cout << "Mot de passe incorrect." << std::endl;
			const char *msg = "Mot de passe incorrect. Connexion refusee. Veuillez reessayer.\r\n";
			send(socket, msg, strlen(msg), 0);
			// close(socket);  // Fermer la connexion en cas de mot de passe incorrect
		}
	}
}

void Server::handleNick(int socket, const std::string& params) {
	std::cout << "Commande NICK reçue avec params: " << params << std::endl;

	Client *client = getClient(socket);
	if (client->isPswdEnterd()) {
		if (!client){
			std::cout << "erreur !client : "<< getClient(socket) << std::endl;
		};

		client->setNickname(params);  // Met a jour le surnom du client
		std::cout << "Client socket " << socket << " set nickname to: " << client->getNickname() << std::endl;
	}
	else {
		const char * msg = "Veuillez d'abord entrer le mot de passe (PASS mdp) \n";
		send(socket, msg, strlen(msg), 0);
	}

}


void Server::handleUser(int socket, const std::string& params) {
	std::cout << "Commande USER reçue avec params: " << params << std::endl;
	std::istringstream iss(params);
	std::string nickname, hostname, servername, username;

	std::getline(iss, nickname, ' ');
	std::getline(iss, hostname, ' ');
	std::getline(iss, servername, ':');
	std::getline(iss, username, ' ');  // username est precede de ':'
	std::cout << "Username : " << username << std::endl;
	Client *client = getClient(socket);
	if (client) {
		std::cout << "client existant" << std::endl;
		client->setNickname(nickname);
		client->setHostname(hostname);
		client->setUsername(username);
		std::cout << "Utilisateur defini nickname : " << client->getNickname() << std::endl;
		std::cout << "Utilisateur defini hostname : " << client->getHostname() << std::endl;
		std::cout << "Utilisateur defini username : " << client->getUsername() << std::endl;
	} else {
		std::cerr << "Client introuvable pour le socket : " << socket << std::endl;
	}
}


void Server::handleOper(int socket, const std::string& params) {
	std::cout << "Commande OPER reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handleMode(int socket, const std::string& params) {
	std::cout << "Commande MODE reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handleQuit(int socket) {
	std::cout << "Commande QUIT reçue" << std::endl;
	Client *client = getClient(socket);
	// std::cout << "socket " << getClient(socket) << std::endl;
	if (!client){
		std::cout << "erreur !client" << std::endl;
	}
	// else
		// std::cout << "tout good" << std::endl;

	// remove le client de tous les channels auxquels il est connecte
	for (std::vector<Channel>::iterator it = channels.begin(); it != channels.end(); ++it){
		it->removeClient(client->getFd());
	}

	std::cout << "Client " << client->getNickname() << " has quit." << std::endl;
	const char *msg = "You quit.\r\n";
	send(socket, msg, strlen(msg), 0);
	close(socket);  // Fermer le socket du client
}

void Server::handleJoin(int socket, const std::string& params) {
	if (params == "")
		std::cerr << "Pas assez de paramètres fournis pour la commande JOIN." << std::endl;
	std::cout << "Commande JOIN reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handlePart(int socket, const std::string& params) {
	std::cout << "Commande PART reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handleTopic(int socket, const std::string& params) {
	std::cout << "Commande TOPIC reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handleKick(int socket, const std::string& params) {
	std::cout << "Commande KICK reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handlePrivmsg(int socket, const std::string& params) {
	std::cout << "Commande PRIVMSG reçue avec params: " << params << std::endl;
	(void) socket;
}
