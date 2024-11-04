#include "../inc/Server.hpp"

void Server::handleConnection(int socket) {
	Client *client = getClient(socket);
	if (!client) {
		std::cerr << "Erreur: client non trouvé pour le socket " << socket << std::endl;
		return; // On ne continue pas si le client n'existe pas
	}
	char buffer[1024] = {0};
	int valread = recv(socket, buffer, sizeof(buffer), 0);

	if (valread > 0) {
		processReceivedData(socket, client, buffer, valread);
	} else if (valread == 0) {
		handleClientDisconnection(socket, client);
	} else if (valread < 0) {
		handleReceiveError(socket);
	}
}

void Server::processReceivedData(int socket, Client* client, char* buffer, int valread) {
	std::string pass;
	std::vector<std::string> commands;
	static std::map<int, std::string> partial_data;

	partial_data[socket] += std::string(buffer, valread);
	std::string& received_data = partial_data[socket];
	// std::cout << "===received data===" << std::endl << received_data << "===" << std::endl;

	extractCommands(received_data, commands);

	handleCommands(socket, client, commands, pass);
}

void Server::extractCommands(std::string& received_data, std::vector<std::string>& commands) {
	size_t crlf_pos, lf_pos;
	while (true) {
		crlf_pos = received_data.find("\r\n");
		lf_pos = received_data.find("\n");

		if (crlf_pos == std::string::npos && lf_pos == std::string::npos) {
			break; // Aucune commande complète, rester dans le buffer
		}

		size_t end_pos = (crlf_pos != std::string::npos && (lf_pos == std::string::npos || crlf_pos < lf_pos)) ? crlf_pos : lf_pos;

		commands.push_back(received_data.substr(0, end_pos));
		received_data.erase(0, end_pos + (received_data[end_pos] == '\r' ? 2 : 1));
	}
}

void Server::handleCommands(int socket, Client* client, const std::vector<std::string>& commands, std::string& pass) {
	(void) client;
	for (std::vector<std::string>::const_iterator it = commands.begin(); it != commands.end(); ++it) {
		processCommand(socket, *it, pass);
	}
}

void Server::processCommand(int socket, const std::string& command_line, std::string& pass) {
	Client *client = getClient(socket);
	if (!client) {
		std::cerr << "Erreur: client non trouvé pour le socket " << socket << std::endl;
		return; // On ne continue pas si le client n'existe pas
	}
	std::istringstream iss(command_line);
	std::string command;
	std::getline(iss, command, ' '); // Premier token : la commande

	std::string params;
	std::getline(iss, params); // Récupérer les paramètres

	std::vector<std::string> paramList;
	std::istringstream paramStream(params);
	std::string param;
	while (paramStream >> param) {
		paramList.push_back(param);
	}

	if (command == "CAP" && !paramList.empty() && (paramList[0] == "LS" || paramList[0] == "END")) {
		handleCapLs(socket, paramList[0]);
	} else if (command == "PASS") {
		handlePassCommand(socket, paramList, client, pass);
	} else if (command == "NICK") {
		handleNickCommand(socket, paramList, client);
	} else if (command == "USER") {
		handleUserCommand(socket, params, client);
	} else if (command == "MODE" && client->isLogged()) {
		handleModeCommand(socket, paramList);
	} else if (command == "QUIT") {
		handleQuit(socket, params);
	} else if (command == "JOIN" && client->isLogged()) {
		handleJoin(socket, params);
	} else if (command == "TOPIC" && client->isLogged()) {
		handleTopic(socket, params);
	} else if (command == "KICK" && client->isLogged()) {
		handleKick(socket, params);
	} else if (command == "PRIVMSG" && client->isLogged()) {
		handlePrivmsg(socket, params);
	} else if (command == "INVITE" && client->isLogged()) {
		handleInvite(socket, params);
	} else {
		handleUnknownCommand(socket, client, command);
	}
	client = getClient(socket);
	if (client && command != "QUIT")
	{
		if (!getClient(socket)->isLogged() && !getClient(socket)->getNickname().empty() && !getClient(socket)->getUsername().empty()) {
			std::string nick = getClient(socket)->getNickname();
			std::string user = getClient(socket)->getUsername();
			authenticateClient(socket, pass, nick, user);
		}
	}
}

void Server::handlePassCommand(int socket, const std::vector<std::string>& paramList, Client* client, std::string& pass) {
	if (paramList.size() == 1) {
		handlePass(socket, paramList[0], getFirstConnexion(), client->getTmpNick(), client->getTmpUser());
		pass = paramList[0];
		setFirstConnexion(false);
	} else {
		std::cerr << "Erreur: PASS necessite 1 param" << std::endl;
	}
}

void Server::handleNickCommand(int socket, const std::vector<std::string>& paramList, Client* client) {
	if (paramList.size() == 1) {
		if (getFirstConnexion()) {
			client->setTmpNick(paramList[0]);
			const char *msg = "Veuillez entrer le mot de passe (PASS mdp) \r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			std::cout << "===Commande NICK===" << std::endl;
			handleNick(socket, paramList[0]);
		}
	} else {
		std::cerr << "Erreur: NICK necessite 1 param" << std::endl;
	}
}

void Server::handleUserCommand(int socket, const std::string& params, Client* client) {
	if (getFirstConnexion()) {
		client->setTmpUser(params);
	} else {
		handleUser(socket, params);
	}
}

void Server::handleModeCommand(int socket, const std::vector<std::string>& paramList) {
	if (paramList.size() < 2) {
		std::cerr << "Erreur: MODE nécessite au moins 2 paramètres" << std::endl;
	} else {
		std::string params;
		for (size_t i = 0; i < paramList.size(); ++i) {
			params += paramList[i] + (i < paramList.size() - 1 ? " " : "");
		}
		handleMode(socket, params);
	}
}

void Server::handleUnknownCommand(int socket, Client* client, const std::string& command) {
	(void) socket;
	if (!client->isLogged())
		std::cerr << "Veuillez vous authentifier" << std::endl;
	else
		std::cerr << "Commande inconnue: " << command << std::endl;
}

void Server::handleClientDisconnection(int socket, Client* client) {
	for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		Channel* channel = *it;
		if (channel->isClient(*client)) {
			notifyClientsOfDisconnection(channel, client);
			(*it)->removeClient(client->getFd());
		}
	}
	removeClient(client);
	std::cout << "Client deconnecte, socket: " << socket << std::endl;
	close(socket);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, NULL);  // delete epoll socket
}

void Server::notifyClientsOfDisconnection(Channel* channel, Client* client) {
	std::string quit_message = "a client has quit.\r\n";
	channel->broadcastMessage(quit_message, client);

	std::string names_list = "= " + channel->getName() + " :";
	const std::vector<Client*>& clients = channel->getClients();
	for (std::vector<Client*>::const_iterator remainingIt = clients.begin(); remainingIt != clients.end(); ++remainingIt) {
		names_list += (*remainingIt)->getNickname() + " ";
	}
	names_list += "\r\n";
	// Vous pouvez envoyer la names_list ici si nécessaire
}

void Server::handleReceiveError(int socket) {
	if (errno == EAGAIN || errno == EWOULDBLOCK)
		return;
	else {
		perror("Erreur de reception sur le socket");
		close(socket);
		return;
	}
}
