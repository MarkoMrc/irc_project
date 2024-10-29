#include <typeinfo>
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

void Server::handlePass(int socket, const std::string& params, bool firstConnexion, std::string nick, std::string user) {
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
			if (firstConnexion)
			{
				handleNick(socket, nick);
				handleUser(socket, user);
			}
			else {
				(void)nick;
				(void)user;
			}
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
	if (!client){
		std::cout << "erreur !client : "<< getClient(socket) << std::endl;
	}
	client->setNickname(params);  // Met a jour le surnom du client
	std::cout << "Client socket " << socket << " set nickname to: " << client->getNickname() << std::endl;

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
		return;
	}
	// else
		// std::cout << "tout good" << std::endl;

	// remove le client de tous les channels auxquels il est connecte
	for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it){
		(*it)->removeClient(client->getFd());
	}

	std::cout << "Client " << client->getNickname() << " has quit." << std::endl;
	const char *msg = "You quit.\r\n";
	send(socket, msg, strlen(msg), 0);
	close(socket);
}


std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> Server::parsJoin(const std::string& params)
{
	std::vector<std::string> param = split(params, ' ');
	// std::string arg[2];
	if (param.size() > 2)
		return std::vector<std::string>();
	else
		return param;
	// for (int i = 0; i < 2; i++)
		// arg[i] = split(param[i], ',');
}

void Server::handleJoin(int socket, const std::string& params) {
	std::cout << "Commande JOIN reçue avec params: " << params << std::endl;

	std::vector<std::string> arg;
	// parsing de join
	arg = parsJoin(params);
	if (arg.empty())
	{
		std::cerr << "parametre vide " << socket << std::endl;
		return;
	}
	Client *client = getClient(socket);
	if (!client)
	{
		std::cerr << "Client not found for socket: " << socket << std::endl;
		return;
	} 
	std::istringstream iss(arg[0]);
	std::string channel_name;

	iss >> channel_name;
	std::cout << "Nom du channel" << channel_name << std::endl;
	if (channel_name.empty() || channel_name[0] != '#')
	{
		std::cerr << "Invalid channel name: " << channel_name << std::endl;
		return;
	}

	// Verifier si le channel exsite deja, sinon creer nouveau channel
	Channel *channel = getChannel(channel_name);
	if (!channel) {
	// il faut le creer
	Channel* new_channel = new Channel();
	new_channel->setName(channel_name);
	new_channel->addAdmin(*client);
	addChannel(new_channel);
	std::cout << "arg size == " << arg.size() << std::endl;
	std::cout << "arg 0 == " << arg[0] << std::endl;
	// std::cout << "arg 1 == " << arg[1] << std::endl;
	if (arg.size() == 2)
	{
		new_channel->setModePasswordProtected(true);
		new_channel->setPassword(arg[1]);
		std::cout << "Mot de passe du channel : " << new_channel->getPassword() << std::endl;
		std::cout << "Type du mot de passe : " << typeid(new_channel->getPassword()).name() << std::endl;
	}
	channel = getChannel(channel_name);
	std::cout << "Created new channel: " << channel_name << std::endl;
	}

	const std::vector<Client>& clients = channel->getClients();
    for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->getFd() == client->getFd()) {
            std::cerr << client->getNickname() << " is already in channel " << channel_name << std::endl;
            return;
        }
    }
	// std::cout << "channel : " << channel->isModePasswordProtected() << " size : " << arg.size() << " password : " << channel->getPassword() << " arg : " << arg[1] << std::endl;
	// verification du mot de pass
	if (channel->isModePasswordProtected() && (arg.size() < 2 || arg[1] != channel->getPassword()))
	{
		std::cout << "error password" << std::endl;
		return;
	}
	if (channel->isModePasswordProtected()) {
    std::cout << "Mode protégé activé pour le canal" << std::endl;
    // if (arg.size() < 2) {
    //     std::cout << "Mot de passe manquant pour rejoindre le canal." << std::endl;
    // } else {
    //     std::cout << "Mot de passe fourni : " << arg[1] << ", mot de passe attendu : " << channel->getPassword() << std::endl;
    // }
}
	channel->addClient(*client);
	std::cout << client->getNickname() << " joined channel " << channel_name << std::endl;

	// message de confirmation
	std::string join_message = ":" + client->getNickname() + " JOIN " + channel_name + "\r\n";
	send(socket, join_message.c_str(), join_message.length(), 0);

	// // Envoyer la liste des utilisateurs actuels du channel au client qui vient de rejoindre
	std::string names_list = "= " + channel_name + " :";
	for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		names_list += it->getNickname() + " ";
	}

	names_list += "\r\n";
	send(socket, names_list.c_str(), names_list.length(), 0);

	// Envoyer le sujet du channel
	std::string topic_message = ":" + channel_name + " TOPIC :" + channel->getTopic_N() + "\r\n";
	send(socket, topic_message.c_str(), topic_message.length(), 0);
}

void Server::handleTopic(int socket, const std::string& params) {
    std::cout << "Commande TOPIC reçue avec params: " << params << std::endl;

    Client *client = getClient(socket);
    if (!client) return;

    std::istringstream iss(params);
    std::string channel_name, topic;
    iss >> channel_name;
    getline(iss, topic);  // Le reste de la ligne après le nom du canal est le sujet

    Channel *channel = getChannel(channel_name);
    if (channel) {
        if (!topic.empty()) { // Vérifie si un nouveau sujet a été fourni
            channel->setTopic_n(topic);
            std::cout << "Topic for channel " << channel_name << " set to: " << topic << std::endl;

            std::string topic_message = ":" + client->getNickname() + " TOPIC " + channel_name + " :" + topic + "\r\n";

            // Envoyer le message à tous les clients dans le canal
            const std::vector<Client>& clients = channel->getClients();
            for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
                send(it->getFd(), topic_message.c_str(), topic_message.length(), 0);
            }
        } else { // Si aucun nouveau sujet n'est fourni, envoyer le sujet actuel
            std::string topic_name = channel->getTopic_N();
            std::string topic_message = ":" + client->getNickname() + " TOPIC " + channel_name + " :" + topic_name + "\r\n";

            // Envoyer le message à tous les clients dans le canal
            const std::vector<Client>& clients = channel->getClients();
            for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
                send(it->getFd(), topic_message.c_str(), topic_message.length(), 0);
            }
        }
    } else {
        std::cerr << "Channel " << channel_name << " not found." << std::endl;
    }
}


void Server::handleKick(int socket, const std::string& params) {
	std::cout << "Commande KICK reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handlePrivmsg(int socket, const std::string& params) {
	std::cout << "Commande PRIVMSG reçue avec params: " << params << std::endl;

	Client *client = getClient(socket);
	if (!client) {
		std::cerr << "Client not found for socket: " << socket << std::endl;
		return;
	}

	std::istringstream iss(params);
	std::string target;
	iss >> target;

	std::string message = params.substr(target.length() + 1);  // Extraire le message apres le destinataire

	if (target[0] == '#') {  // Si le message est destine a un channel
		Channel *channel = getChannel(target);
		if (!channel) {
			std::cerr << "Channel " << target << " not found." << std::endl;
			return;
		}

		// Envoyer le message a tous les clients dans le channel, sauf l'expediteur
		std::string formatted_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		std::vector<Client>::iterator it;
		for (it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
			if (it->getFd() != client->getFd()) {
				send(it->getFd(), formatted_message.c_str(), formatted_message.length(), 0);
			}
		}

		std::cout << "Message from " << client->getNickname() << " to channel " << target << ": " << message << std::endl;
	} else {  // Si le message est destine a un utilisateur
		Client *target_client = getClient(target);
		if (!target_client) {
			std::cerr << "Client " << target << " not found." << std::endl;
			return;
		}
		std::cout << "Client " << target << std::endl;

		// Envoyer le message a l'utilisateur cible
		std::string formatted_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
		send(target_client->getFd(), formatted_message.c_str(), formatted_message.length(), 0);
		std::cout << "Message from " << client->getNickname() << " to " << target << ": " << message << std::endl;
	}
}

void Server::handleOper(int socket, const std::string& params) {
    std::cout << "Commande TOPIC reçue avec params: " << params << std::endl;
	(void) socket;
}