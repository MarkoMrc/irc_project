#include <typeinfo>
#include "../inc/Server.hpp"

// CAP LS
void Server::handleCapLs(int socket, std::string command) {
	Client *client = getClient(socket);
	if (!client){
		std::cout << "erreur !client : "<< getClient(socket) << std::endl;
	};

	if (command == "LS") {
        // Logique pour gérer CAP LS
		std::cout << "===Commande CAP LS reçue===" << std::endl;
        std::string response = ":server CAP * LS :\r\n";
        send(socket, response.c_str(), response.size(), 0); // Envoyer la liste des capacités
    } 
    // Vérifiez si la commande est CAP END
    else if (command == "END") {
        // Logique pour gérer CAP END
        // std::string response = ":server CAP * END :End of CAP list\r\n";
		std::cout << "===Command CAP END reçue===" << std::endl;
        // send(socket, response.c_str(), response.size(), 0); // Envoyer la confirmation de la fin des capacités
    } 
    else {
        // Gérer les commandes non reconnues si nécessaire
        std::string errorResponse = ":server ERR_UNKNOWNCAP :Unknown CAP command\r\n";
        send(socket, errorResponse.c_str(), errorResponse.size(), 0);
    }
}

void Server::handlePass(int socket, const std::string& params, bool firstConnexion, std::string nick, std::string user) {
	if (params[0] != ' ') {
		std::string client_password = params.substr(0, params.find(' '));
		std::cout << "===Commande PASS reçue avec params: " << params << "===" << std::endl;
		std::string mdp = getPassword();
		if (client_password == mdp) {
			std::cout << "Mot de passe correct." << std::endl;
			Client *client = getClient(socket);
			if (!client){
				std::cout << "erreur !client : "<< getClient(socket) << std::endl;
				return ;
			}
			client->setPswdEnterd(true);
			const char *msg = "Mot de passe correct. Connexion acceptee.\r\n";
			send(socket, msg, strlen(msg), 0);
			if (firstConnexion)
			{
				if (!nick.empty())
					handleNick(socket, nick);
				if (!user.empty())
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
		}
	}
}

bool Server::checkNick(const std::string& params)
{
	for (long unsigned int i = 0; i < params.size(); i++)
		if (!std::isalnum(params[i]) && params[i] != '_' && params[i] != '-')
			return false;
	return true;
}


void Server::handleNick(int socket, const std::string& params) {
	std::cout << "===Commande NICK reçue avec params: " << params << "===" << std::endl;
	Client *client = getClient(socket);
	if (!client){
		std::cout << "erreur !client : "<< getClient(socket) << std::endl;
	}
	if (params.empty()) {
		std::cout << "message d'erreur" << std::endl;
		return ;
	}
	if (!checkNick(params))
	{
		std::cout << "le nickname doit etre compose uniquement de lettre chiffre ou tiret" << std::endl; // mettre les bon messages d'erreur
		return;
	}
	std::vector<Client*> clients = getClients();

	// Vérifier si le nouveau nickname existe déjà
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getNickname() == params) {
			std::cout << "Erreur : le nickname '" << params << "' existe déjà." << std::endl;
			return;
		}
	}
	client->setNickname(params);  // Met a jour le surnom du client
	std::cout << "Client socket " << socket << " set nickname to: " << client->getNickname() << std::endl;

}


void Server::handleUser(int socket, const std::string& params) {
	std::cout << "===Commande USER reçue avec params: " << params << "===" << std::endl;

	std::istringstream iss(params);
	std::string realname, hostname, servername, username;

	std::getline(iss, username, ' ');
	std::getline(iss, hostname, ' ');
	std::getline(iss, servername, ':');
	std::getline(iss, realname, ' ');  // username est precede de ':'
	Client *client = getClient(socket);
	if (client) {
		if (!username.empty()) {
			client->setUsername(username);
			std::cout << "Utilisateur defini username : " << client->getUsername() << std::endl;
		}
		if (!hostname.empty()) {
			client->setHostname(hostname);
			std::cout << "Utilisateur defini hostname : " << client->getHostname() << std::endl;
		}
		if (!servername.empty()) {
			client->setServername(servername);
			std::cout << "Utilisateur defini servername : " << client->getServername() << std::endl;
		}
		if (!realname.empty()) {
			client->setRealname(realname);
			std::cout << "Utilisateur defini realname : " << client->getRealname() << std::endl;
		}
	} else {
		std::cerr << "Client introuvable pour le socket : " << socket << std::endl;
		}
}

void Server::handleQuit(int socket, const std::string& params) {
	std::cout << "===Commande QUIT reçue===" << std::endl;
	Client *client = getClient(socket);
	if (!client) {
		std::cout << "Erreur : client introuvable." << std::endl;
		return;
	}

	// Supprimer le client de tous les canaux auxquels il est connecté
	for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		Channel* channel = *it;
		if (channel->isClient(*client)) {
			// Notifier les autres clients que ce client a quitté
			std::string quit_message = ":" + client->getNickname() + " QUIT :" + (params.empty() ? "has quit." : params) + "\r\n";
			channel->broadcastMessage(quit_message, client);

			// Mettre à jour la liste des noms pour tous les clients restants
			std::string names_list = "= " + channel->getName() + " :";
			const std::vector<Client*>& clients = channel->getClients();
			for (std::vector<Client*>::const_iterator remainingIt = clients.begin(); remainingIt != clients.end(); ++remainingIt) {
				names_list += (*remainingIt)->getNickname() + " ";
			}
			names_list += "\r\n";
		}
		(*it)->removeClient(client->getFd());
	}

	client->setLogged(false); // Indique que le client est authentifie
	removeClient(client);

	std::cout << "Client " << client->getNickname() << " QUIT : " << (params.empty() ? "has quit." : params) << std::endl;

	client->setNickname("");
	const char *msg = "You quit.\r\n";
	send(socket, msg, strlen(msg), 0);
	close(socket);
}