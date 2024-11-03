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
			// close(socket);  // Fermer la connexion en cas de mot de passe incorrect
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
	std::cout << "Commande NICK reçue avec params: " << params << std::endl;
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
	std::cout << "Commande USER reçue avec params: " << params << std::endl;

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

void Server::handleMode(int socket, const std::string& params) {
	std::cout << "Commande MODE reçue avec params: " << params << std::endl;
	std::vector<std::string> words;
	std::istringstream iss(params);
	std::string word;

	while (iss >> word)
	{
		words.push_back(word);
	}
	Client *client = getClient(socket);
	Channel *channel = getChannel(words[0]);
	if (!channel) {
		std::cerr << "Erreur: le channel spécifié n'existe pas" << std::endl;
		return ;
	}
	if (!channel->isAdmin(*client)) {
		std::cerr << "Erreur: permission non accordée car vous n'êtes pas opérateur sur ce channel" << std::endl;
		return ;
	}
	if (words[1] == "+o" || words[1] == "-o") {
		if (words.size() < 3) {
			std::cerr << "Erreur: le mode +/- o nécessite 3 paramètres (MODE channel mode nom_d_utilisateur)" << std::endl;
		}
		else {
			Client *mod = getClient(words[2]);
			if (!channel->isClient(*mod)) {
				std::cerr << "le client" << words[2] << "n'appartient pas au channel spécifié" << std::endl;
			}
			else {
				if (words[1] == "+o") {
					if (channel->isAdmin(*mod)) {
						std::cerr << "Erreur: le client spécifié est déja opérateur" << std::endl;
					}
					else {
						channel->addAdmin(*mod);
					}
				}
				else {
					if (!channel->isAdmin(*mod)) {
						std::cerr << "Erreur : le client spécifié n'est pas opérateur" << std::endl;
					}
					else {
						channel->removeAdmin(mod->getFd());
					}
				}
			}
		}
	}
	else if (words[1] == "+i") {
		if (words.size() != 2) {
			std::cerr << "le mode +/- i nécessite 2 paramètres (channel mode)" << std::endl;
		}
		else {
			if (channel->isModeInviteOnly()) {
				std::cerr << "le channel spécifié est déjà sur invitation uniquement" << std::endl;
			}
			else {
				channel->setModeInviteOnly(true);
			}
		}
	}
	else if (words[1] == "-i") {
		if (words.size() != 2) {
			std::cerr << "Erreur: le mode +/- i nécessite 2 paramètres (MODE channel mode)" << std::endl;
		}
		else {
			if (!channel->isModeInviteOnly()) {
				std::cerr << "Erreur: le channel spécifié n'est pas sur invitation uniquement" << std::endl;
			}
			else {
				channel->setModeInviteOnly(false);
			}
		}
	}
	else if (words[1] == "+t") {
		if (words.size() != 2) {
			std::cerr << "Erreur: le mode +t nécessite 2 paramètres (channel mode)" << std::endl;
		}
		else {
			channel->setModeTopicOp(true);
		}
	}
	else if (words[1] == "-t") {
		if (words.size() != 2) {
			std::cerr << "Erreur: le mode +t nécessite 2 paramètres (channel mode)" << std::endl;
		}
		else {
			if (!channel->isModeTopicOp()) {
				std::cerr << "Erreur: le channel spécifié n'a pas de restriction sue la commande TOPIC" << std::endl;
			}
			else {
				channel->setModeTopicOp(false);
			}
		}
	}
	else if (words[1] == "+k") {
		if (words.size() != 3) {
			std::cerr << "Erreur: le mode +k nécessite 3 paramètres (channel mode password)" << std::endl;
		}
		else {
			if (!channel->isModePasswordProtected()) {
				channel->setModePasswordProtected(true);
			}
			channel->setPassword(words[2]);
		}
	}
	else if (words[1] == "-k") {
		if (words.size() != 2) {
			std::cerr << "Erreur: le mode -k nécessite 2 paramètres (channel mode)" << std::endl;
		}
		else {
			if (!channel->isModePasswordProtected()) {
				std::cerr << "Erreur: ce channel n'a pas de mot de passe" << std::endl;
			}
			else {
				channel->setModePasswordProtected(false);
			}
		}
	}
	else if (words[1] == "+l") {
		if (words.size() != 4) {
			std::cerr << "Erreur: le mode +l nécessite 3 paramètres (channel mode nbLimite)" << std::endl;
		}
		else {
			if (!channel->isModeLimit()) {

				channel->setModeLimit(true);
				channel->setLimit(atoi(words[2].c_str()));
			}
			std::istringstream iss(words[2]);
			size_t limit;
			if (!(iss >> limit)){
				std::cerr << "Erreur : valeur non valide pour la limite du nombre de clients." << std::endl;
			}
			else {
				channel->setLimit(limit);
			}
		}
	}
	else if (words[1] == "-l") {
		if (words.size() != 2) {
			std::cerr << "Erreur: le mode -l nécessite 2 paramètres (channel mode)" << std::endl;
		}
		else {
			if (!channel->isModeLimit()) {
				std::cerr << "Erreur: ce channel n'est pas limité" << std::endl;
			}
			else {
				channel->setModeLimit(false);
			}
		}
	}
	else {
		std::cerr << "Erreur: mode inconnu" << std::endl;
	}
	
}

// void Server::handleQuit(int socket, const std::string& params) {
// 	std::cout << "Commande QUIT reçue" << std::endl;
// 	Client *client = getClient(socket);
// 	// std::cout << "socket " << getClient(socket) << std::endl;
// 	if (!client){
// 		std::cout << "erreur !client" << std::endl;
// 		return;
// 	}
// 	// else
// 		// std::cout << "tout good" << std::endl;

// 	// remove le client de tous les channels auxquels il est connecte
// 	for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it){
// 		(*it)->removeClient(client->getFd());
// 	}

// 	if (params.empty())
// 		std::cout << "Client " << client->getNickname() << " QUIT :" << " has quit." << std::endl;
// 	else
// 		std::cout << "Client " << client->getNickname() << " QUIT :" << params << std::endl;
// 	const char *msg = "You quit.\r\n";
// 	send(socket, msg, strlen(msg), 0);
// 	close(socket);
// }

void Server::handleQuit(int socket, const std::string& params) {
	std::cout << "Commande QUIT reçue" << std::endl;
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

			// Envoyer la mise à jour de la liste des noms à tous les clients restants
			for (std::vector<Client*>::const_iterator remainingIt = clients.begin(); remainingIt != clients.end(); ++remainingIt) {
				send((*remainingIt)->getFd(), names_list.c_str(), names_list.length(), 0);
			}
		}
		(*it)->removeClient(client->getFd());
	}

	std::cout << "Client " << client->getNickname() << " QUIT : " << (params.empty() ? "has quit." : params) << std::endl;
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

	const std::vector<Client*>& clients = channel->getClients();
	std::cout << "Clients dans le canal " << channel_name << ":" << std::endl;

	// Vérifiez la taille du vecteur
	std::cout << "Nombre de clients dans le canal: " << clients.size() << std::endl;

	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << "Client FD: " << (*it)->getFd() << ", Nickname: " << (*it)->getNickname() << std::endl;
	}

	// Vérification si le client est déjà dans le canal
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << "Je rentre dans le check" << std::endl;
		std::cout << "client= " << client->getFd() << std::endl;
		std::cout << "it = " << (*it)->getFd() << std::endl;

		if ((*it)->getFd() == client->getFd()) {
			std::cerr << client->getNickname() << " est déjà dans le canal " << channel_name << std::endl;
			return;
		}
	}
	// const std::vector<Client*>& clients = channel->getClients();
	// for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
	//	 std::cout << "Client FD: " << (*it)->getFd() << ", Nickname: " << (*it)->getNickname() << std::endl;
	// }
	// for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
	// 	std::cout << "je rentre dans le check" << std::endl;
	// 	std::cout << "client= " << client->getFd() << std::endl;
	// 	std::cout << "it = " << (*it)->getFd() << std::endl;
	// 	if ((*it)->getFd() == client->getFd()) {
	// 		std::cout << "it = " << (*it)->getFd() << std::endl;
	// 		std::cerr << client->getNickname() << " is already in channel " << channel_name << std::endl;
	// 		return;
	// 	}
	// }
	// std::cout << "channel : " << channel->isModePasswordProtected() << " size : " << arg.size() << " password : " << channel->getPassword() << " arg : " << arg[1] << std::endl;
	// verification du mot de pass
	if (channel->isModePasswordProtected() && (arg.size() < 2 || arg[1] != channel->getPassword()))
	{
		std::cout << "error password" << std::endl;
		return;
	}
	// if (channel->isModePasswordProtected()) {
		// std::cout << "Mode protégé activé pour le canal" << std::endl;
		// if (arg.size() < 2) {
		//	 std::cout << "Mot de passe manquant pour rejoindre le canal." << std::endl;
		// } else {
		//	 std::cout << "Mot de passe fourni : " << arg[1] << ", mot de passe attendu : " << channel->getPassword() << std::endl;
		// }
		// channel->addClient(client);
	// }
	if (channel->isModeLimit()){
		if (channel->isFull()) {
			std::cerr << "Le canal " << channel_name << " est plein, impossible de rejoindre." << std::endl;
			std::string error_message = "Le canal est plein, vous ne pouvez pas rejoindre.\r\n";
			send(socket, error_message.c_str(), error_message.length(), 0);
			return;
		}
	}

	channel->addClient(client);
	std::cout << "Clients après ajout:" << std::endl;
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		std::cout << "Client FD: " << (*it)->getFd() << ", Nickname: " << (*it)->getNickname() << std::endl;
	}


	std::cout << client->getNickname() << " joined channel " << channel_name << std::endl;

	// message de confirmation
	std::string join_message = ":" + client->getNickname() + " JOIN " + channel_name + "\r\n";
	send(socket, join_message.c_str(), join_message.length(), 0);

	// // Envoyer la liste des utilisateurs actuels du channel au client qui vient de rejoindre
	std::string names_list = "= " + channel_name + " :";
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		names_list += (*it)->getNickname() + " ";
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
	const std::vector<Client*>& clients = channel->getClients();
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
	send((*it)->getFd(), topic_message.c_str(), topic_message.length(), 0);
	}
	} else { // Si aucun nouveau sujet n'est fourni, envoyer le sujet actuel
	std::string topic_name = channel->getTopic_N();
	std::string topic_message = ":" + client->getNickname() + " TOPIC " + channel_name + " :" + topic_name + "\r\n";

	// Envoyer le message à tous les clients dans le canal
	const std::vector<Client*>& clients = channel->getClients();
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
	send((*it)->getFd(), topic_message.c_str(), topic_message.length(), 0);
	}
	}
	} else {
	std::cerr << "Channel " << channel_name << " not found." << std::endl;
	}
}


void Server::handleKick(int socket, const std::string& params) {
	std::cout << "Commande KICK reçue avec params: " << params << std::endl;
	std::vector<std::string> parsedParams;
	std::istringstream iss(params);
	std::string word;

	// Séparer les paramètres par espace
	while (iss >> word) {
		parsedParams.push_back(word);
	}

	// Vérification du nombre minimum de paramètres attendus (deux : canal et pseudo de l'utilisateur)
	if (parsedParams.size() < 2) {
		std::cerr << "Erreur : Paramètres insuffisants pour la commande KICK" << std::endl;
		return;
	}

	// Extraction des paramètres
	std::string channelName = parsedParams[0];
	std::string targetNickname = parsedParams[1];
	std::string reason;

	// Si une raison est fournie, on la reconstruit à partir des éléments restants
	if (parsedParams.size() > 2) {
		reason = params.substr(params.find(parsedParams[2]));  // Recréer la raison à partir du troisième mot
	}

	// Récupérer le client émetteur de la commande KICK
	Client *kicker = getClient(socket);
	if (!kicker) {
		std::cerr << "Erreur : client émetteur introuvable pour le socket " << socket << std::endl;
		return;
	}

	// Récupérer le canal
	Channel *channel = getChannel(channelName);
	if (!channel) {
		std::cerr << "Erreur : le canal " << channelName << " n'existe pas" << std::endl;
		return;
	}

	// Vérifier si l'émetteur est membre du canal et a les droits nécessaires pour expulser
	if (!channel->isClient(*kicker)) {
		std::cerr << "Erreur : vous n'êtes pas membre du canal " << channelName << std::endl;
		return;
	}
	if (!channel->isAdmin(*kicker)) {
		std::cerr << "Erreur : vous n'avez pas la permission d'expulser dans ce canal" << std::endl;
		return;
	}

	// Récupérer le client cible de l'expulsion
	Client *kickee = getClient(targetNickname);
	if (!kickee) {
		std::cerr << "Erreur : le client " << targetNickname << " n'existe pas" << std::endl;
		return;
	}

	// Vérifier si le client est bien dans le canal
	if (!channel->isClient(*kickee)) {
		std::cerr << "Erreur : " << targetNickname << " n'est pas dans le canal " << channelName << std::endl;
		return;
	}

	// Créer le message de kick à envoyer à tous les membres du canal
	std::string kickMessage = ":" + kicker->getNickname() + " KICK " + channelName + " " + targetNickname;
	if (!reason.empty()) {
		kickMessage += " :" + reason;
	}
	kickMessage += "\r\n";

	// Envoyer le message à tous les membres du canal
	channel->broadcastMessage(kickMessage, kickee);

	// Retirer le client du canal
	channel->removeClient(kickee->getFd());
	std::cout << "Client " << targetNickname << " expulsé du canal " << channelName << " par " << kicker->getNickname();
	if (!reason.empty()) {
		std::cout << " pour la raison : " << reason;
	}
	std::cout << std::endl;
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
		std::vector<Client*>::iterator it;
		for (it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
			if ((*it)->getFd() != client->getFd()) {
				send((*it)->getFd(), formatted_message.c_str(), formatted_message.length(), 0);
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
	std::cout << "Commande OPER reçue avec params: " << params << std::endl;
	(void) socket;
}

void Server::handleInvite(int socket, const std::string& params) {
	std::cout << "Commande INVITE reçue avec params: " << params << std::endl;

	// Parsing de params avec un vecteur
	std::vector<std::string> parsedParams;
	std::istringstream iss(params);
	std::string word;

	// Séparer les paramètres par espace
	while (iss >> word) {
		parsedParams.push_back(word);
	}

	// Vérification du nombre de paramètres attendus (deux : le pseudo cible et le canal)
	if (parsedParams.size() < 2) {
		std::cerr << "Erreur : Paramètres insuffisants pour la commande INVITE" << std::endl;
		return;
	}

	// Extraction des paramètres
	std::string targetNickname = parsedParams[0];
	std::string channelName = parsedParams[1];

	// Récupérer le client émetteur de la commande INVITE
	Client *inviter = getClient(socket);
	if (!inviter) {
		std::cerr << "Erreur : client émetteur introuvable pour le socket " << socket << std::endl;
		return;
	}

	// Récupérer le canal
	Channel *channel = getChannel(channelName);
	if (!channel) {
		std::cerr << "Erreur : le canal " << channelName << " n'existe pas" << std::endl;
		return;
	}

	// Vérifier si l'inviteur est dans le canal et a les droits nécessaires
	if (!channel->isClient(*inviter)) {
		std::cerr << "Erreur : vous n'êtes pas membre du canal " << channelName << std::endl;
		return;
	}
	if (!channel->isAdmin(*inviter)) {
		std::cerr << "Erreur : vous n'avez pas la permission d'inviter dans ce canal" << std::endl;
		return;
	}

	// Récupérer le client cible de l'invitation
	Client *invitee = getClient(targetNickname);
	if (!invitee) {
		std::cerr << "Erreur : le client " << targetNickname << " n'existe pas" << std::endl;
		return;
	}

	// Vérifier si l'invité est déjà dans le canal
	if (channel->isClient(*invitee)) {
		std::cerr << "Erreur : " << targetNickname << " est déjà dans le canal " << channelName << std::endl;
	return;
	}

	// Envoyer l'invitation au client cible
	std::string inviteMessage = ":" + inviter->getNickname() + " INVITE " + targetNickname + " :" + channelName + "\r\n";
	send(invitee->getFd(), inviteMessage.c_str(), inviteMessage.length(), 0);
	std::cout << "Invitation envoyée de " << inviter->getNickname() << " à " << targetNickname << " pour rejoindre le canal " << channelName << std::endl;

	// Confirmer l'invitation à l'invitant
	std::string confirmationMessage = ":" + inviter->getNickname() + " INVITE " + targetNickname + " " + channelName + "\r\n";
	send(socket, confirmationMessage.c_str(), confirmationMessage.length(), 0);
}

