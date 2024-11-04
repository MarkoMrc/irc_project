#include "../inc/Server.hpp"

void Server::handleMode(int socket, const std::string& params) {
	std::cout << "===Commande MODE reçue avec params: " << params << std::endl;

	std::vector<std::string> words = parseParams(params);
	Client* client = getClient(socket);
	Channel* channel = getChannel(words[0]);

	if (!channel) {
		std::cerr << "Erreur: le channel spécifié n'existe pas" << std::endl;
		const char *msg = "ERR_NOSUCHCHANNEL (403) : Le channel spécifié n'existe pas.\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	if (!channel->isAdmin(*client)) {
		std::cerr << "Erreur: permission non accordée car vous n'êtes pas opérateur sur ce channel" << std::endl;
		const char *msg = "ERR_CHANOPRIVSNEEDED (482) : L'utilisateur tente de changer un mode sur un channel sans être opérateur.\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	processModeCommand(channel, client, words, socket);
}


// Routeur des modes
void Server::processModeCommand(Channel* channel, Client* client, const std::vector<std::string>& words, int socket) {
	(void) client;
	if (words[1] == "+o" || words[1] == "-o") {
		handleOperatorMode(channel, words, socket);
	} else if (words[1] == "+i" || words[1] == "-i") {
		handleInviteOnlyMode(channel, words, client, socket);
	} else if (words[1] == "+t" || words[1] == "-t") {
		handleTopicProtectionMode(channel, words, client, socket);
	} else if (words[1] == "+k" || words[1] == "-k") {
		handlePasswordMode(channel, words, client, socket);
	} else if (words[1] == "+l" || words[1] == "-l") {
		handleLimitMode(channel, words, client, socket);
	} else {
		std::cerr << "Erreur: mode inconnu" << std::endl;
	}
}

// Gestion du mode opérateur
void Server::handleOperatorMode(Channel* channel, const std::vector<std::string>& words, int socket) {
	if (words.size() < 3) {
		const char *msg = "ERR_NEEDMOREPARAMS (461) : Pas assez de paramètres fournis pour la commande INVITE\r\n";
		send(socket, msg, strlen(msg), 0);
		std::cerr << "Erreur: le mode +/- o nécessite 3 paramètres (MODE channel mode nom_d_utilisateur)" << std::endl;
		return;
	}
	Client* mod = getClient(words[2]);
	if (!channel->isClient(*mod)) {
		const char *msg = "ERR_NOSUCHNICK (401) : Le nickname n'existe pas.\r\n";
		send(socket, msg, strlen(msg), 0);
		std::cerr << "le client " << words[2] << " n'appartient pas au channel spécifié" << std::endl;
		return;
	}
	if (words[1] == "+o") {
		if (channel->isAdmin(*mod)) {
			std::cerr << "Erreur: le client spécifié est déjà opérateur" << std::endl;
			const char *msg = "Erreur: le client spécifié est déjà opérateur\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->addAdmin(*mod);
			std::cout << "Ce client est désormais admin" << std::endl;
			const char *msg = "Ce client est désormais admin\r\n";
			send(socket, msg, strlen(msg), 0);
		}
	} else {
		if (!channel->isAdmin(*mod)) {
			std::cerr << "Erreur : le client spécifié n'est pas opérateur" << std::endl;
			const char *msg = "Erreur : le client spécifié n'est pas opérateur\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->removeAdmin(mod->getFd());
			std::cout << "Ce client n'est plus admin" << std::endl;
			const char *msg = "Ce client n'est plus admin\r\n";
			send(socket, msg, strlen(msg), 0);
		}
	}
}

// Gestion du mode invitation uniquement
void Server::handleInviteOnlyMode(Channel* channel, const std::vector<std::string>& words, Client* client, int socket) {
	if (words[1] == "+i") {
		if (channel->isModeInviteOnly()) {
			std::cerr << "le channel spécifié est déjà sur invitation uniquement" << std::endl;
			const char *msg = "le channel spécifié est déjà sur invitation uniquement\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->setModeInviteOnly(true);
			std::cout << "Ce channel est désormais sur invitation only" << std::endl;
			const char *msg = "Ce channel est désormais sur invitation only\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
	} else {
		if (!channel->isModeInviteOnly()) {
			std::cerr << "Erreur: le channel spécifié n'est pas sur invitation uniquement" << std::endl;
			const char *msg = "Erreur: le channel spécifié n'est pas sur invitation uniquement\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->setModeInviteOnly(false);
			std::cout << "Ce channel n'est plus sur invitation Only" << std::endl;
			const char *msg = "Ce channel n'est plus sur invitation Only\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
	}
}

// Gestion du mode protection de sujet
void Server::handleTopicProtectionMode(Channel* channel, const std::vector<std::string>& words, Client* client, int socket) {
	if (words[1] == "+t") {
		channel->setModeTopicOp(true);
		std::cout << "Ce channel est désormais protégé sur le TOPIC" << std::endl;
		const char *msg = "Ce channel est désormais protégé sur le TOPIC\r\n";
		broadcastChannelMessage(channel, client, msg);
	} else {
		if (!channel->isModeTopicOp()) {
			std::cerr << "Erreur: le channel spécifié n'a pas de restriction sur la commande TOPIC" << std::endl;
			const char *msg = "Erreur: le channel spécifié n'a pas de restriction sur la commande TOPIC\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->setModeTopicOp(false);
			std::cout << "Ce channel n'est plus protégé sur le TOPIC" << std::endl;
			const char *msg = "Ce channel n'est plus protégé sur le TOPIC\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
	}
}

// Gestion du mode mot de passe
void Server::handlePasswordMode(Channel* channel, const std::vector<std::string>& words, Client* client, int socket) {
	if (words[1] == "+k") {
		if (words.size() != 3) {
			const char *msg = "ERR_NEEDMOREPARAMS (461) : Pas assez de paramètres fournis pour la commande INVITE\r\n";
			send(socket, msg, strlen(msg), 0);
			std::cerr << "Erreur: le mode +k nécessite 3 paramètres (channel mode password)" << std::endl;
			return;
		}
		if (!channel->isModePasswordProtected()) {
			channel->setModePasswordProtected(true);
			std::cout << "Ce channel est désormais sous mot de passe" << std::endl;
			const char *msg = "Ce channel est désormais sous mot de passe\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
		channel->setPassword(words[2]);
	} else {
		if (!channel->isModePasswordProtected()) {
			std::cerr << "Erreur: ce channel n'a pas de mot de passe" << std::endl;
			const char *msg = "Erreur: ce channel n'a pas de mot de passe\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->setModePasswordProtected(false);
			std::cout << "Ce channel n'est plus sous mot de passe" << std::endl;
			const char *msg = "Ce channel n'est plus sous mot de passe\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
	}
}

// Gestion du mode limite
void Server::handleLimitMode(Channel* channel, const std::vector<std::string>& words, Client* client, int socket) {
	if (words[1] == "+l") {
		if (words.size() != 3) {
			const char *msg = "ERR_NEEDMOREPARAMS (461) : Pas assez de paramètres fournis pour la commande INVITE\r\n";
			send(socket, msg, strlen(msg), 0);
			std::cerr << "Erreur: le mode +l nécessite 3 paramètres (channel mode nbLimite)" << std::endl;
			return;
		}
		if (!channel->isModeLimit()) {
			channel->setModeLimit(true);
			std::cout << "Ce channel est limité" << std::endl;
			const char *msg = "Ce channel est limité\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
		int limit = atoi(words[2].c_str());
		if (limit <= 0) {
			std::cerr << "Erreur : valeur non valide pour la limite du nombre de clients." << std::endl;
			const char *msg = "Erreur : valeur non valide pour la limite du nombre de clients.\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->setLimit(limit);
		}
	} else {
		if (!channel->isModeLimit()) {
			std::cerr << "Erreur: ce channel n'est pas limité" << std::endl;
			const char *msg = "Erreur: ce channel n'est pas limité.\r\n";
			send(socket, msg, strlen(msg), 0);
		} else {
			channel->setModeLimit(false);
			std::cout << "Ce channel n'est plus limité" << std::endl;
			const char *msg = "Ce channel n'est plus limité.\r\n";
			broadcastChannelMessage(channel, client, msg);
		}
	}
}