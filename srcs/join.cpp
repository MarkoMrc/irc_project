#include "../inc/Server.hpp"

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
	std::cout << "===Commande JOIN reçue avec params: " << params << "===" << std::endl;

	std::vector<std::string> args = parseJoinParams(params);
	if (args.empty()) {
		std::cerr << "Paramètre vide " << socket << std::endl;
		const char *msg = "ERR_NEEDMOREPARAMS (461) : Pas assez de paramètres fournis pour la commande INVITE\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	Client* client = getClient(socket);
	if (!client) {
		std::cerr << "Client non trouvé pour le socket: " << socket << std::endl;
		return;
	}

	std::string channel_name = args[0];
	if (!isValidChannelName(channel_name)){
		const char *msg = "ERR_NOSUCHCHANNEL (403) : Le channel spécifié n'existe pas.\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	Channel* channel = getOrCreateChannel(channel_name, client, args);
	if (!channel) return;

	if (checkChannelMembership(channel, client, channel_name, socket)) return;
	if (!validateChannelAccess(channel, client, args, socket)) return;

	addClientToChannel(channel, client, socket);
	sendJoinMessages(channel, client, channel_name, socket);
}

// Fonction pour analyser les paramètres de join
std::vector<std::string> Server::parseJoinParams(const std::string& params) {
	std::vector<std::string> args;
	std::istringstream iss(params);
	std::string word;
	while (iss >> word) args.push_back(word);
	return args;
}

// Fonction pour vérifier si le nom du canal est valide
bool Server::isValidChannelName(const std::string& channel_name) {
	if (channel_name.empty() || channel_name[0] != '#') {
		std::cerr << "Nom de canal invalide: " << channel_name << std::endl;
		return false;
	}
	return true;
}

// Fonction pour obtenir ou créer un canal si nécessaire
Channel* Server::getOrCreateChannel(const std::string& channel_name, Client* client, const std::vector<std::string>& args) {
	Channel* channel = getChannel(channel_name);
	if (!channel) {
		Channel* new_channel = new Channel();
		new_channel->setName(channel_name);
		new_channel->addAdmin(*client);
		addChannel(new_channel);

		if (args.size() == 2) {
			new_channel->setModePasswordProtected(true);
			new_channel->setPassword(args[1]);
		}
		channel = new_channel;
		std::cout << "Création du nouveau canal: " << channel_name << std::endl;
	}
	return channel;
}

// Fonction pour vérifier si le client est déjà dans le canal
bool Server::checkChannelMembership(Channel* channel, Client* client, const std::string& channel_name, int socket) {
	const std::vector<Client*>& clients = channel->getClients();
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if ((*it)->getFd() == client->getFd()) {
			std::string error_message = "ERR_USERONCHANNEL (443) : client is already on channel\r\n";
			send(socket, error_message.c_str(), error_message.length(), 0);
			std::cerr << client->getNickname() << " est déjà dans le canal " << channel_name << std::endl;
			return true;
		}
	}
	return false;
}


// Fonction pour valider l'accès au canal en fonction des règles de mot de passe et d'invitation
bool Server::validateChannelAccess(Channel* channel, Client* client, const std::vector<std::string>& args, int socket) {
	if (channel->isModeInviteOnly() && !channel->isInvited(client)) {
		std::cerr << client->getNickname() << " n'est pas invité dans le canal " << channel->getName() << std::endl;
		std::string error_message = "ERR_INVITEONLYCHAN (473) : Le channel est en mode invite-only et l'utilisateur n'a pas été invité.\r\n";
		send(socket, error_message.c_str(), error_message.length(), 0);
		std::string error_message1 = "Vous devez être invité pour rejoindre ce canal.\r\n";
		send(socket, error_message1.c_str(), error_message.length(), 0);
		return false;
	}

	if (channel->isModePasswordProtected() && (args.size() < 2 || args[1] != channel->getPassword())) {
		std::cerr << "Mot de passe incorrect pour le canal " << channel->getName() << std::endl;
		std::string error_message = "ERR_BADCHANNELKEY (475) : Mauvais mot de passe pour rejoindre un channel privé/protégé.\r\n";
		send(socket, error_message.c_str(), error_message.length(), 0);
		return false;
	}

	if (channel->isModeLimit() && channel->getClients().size() >= channel->getLimit()) {
		std::cerr << "Le canal " << channel->getName() << " est plein, impossible de rejoindre." << std::endl;
		std::string error_message = "ERR_CHANNELISFULL (471) : Le channel est plein, impossible de joindre.\r\n";
		send(socket, error_message.c_str(), error_message.length(), 0);
		return false;
	}
	return true;
}

// Fonction pour ajouter le client au canal
void Server::addClientToChannel(Channel* channel, Client* client, int socket) {
	(void) socket;
	channel->addClient(client);
	std::cout << client->getNickname() << " a rejoint le canal " << channel->getName() << std::endl;
}

// Fonction pour envoyer les messages de confirmation de join
void Server::sendJoinMessages(Channel* channel, Client* client, const std::string& channel_name, int socket) {
	std::string join_message = ":" + client->getNickname() + " JOIN " + channel_name + "\r\n";
	send(socket, join_message.c_str(), join_message.length(), 0);

	std::string names_list = "= " + channel_name + " :";
	const std::vector<Client*>& clients = channel->getClients();
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		names_list += (*it)->getNickname() + " ";
	}
	names_list += "\r\n";
	send(socket, names_list.c_str(), names_list.length(), 0);

	std::string topic_message = ":" + channel_name + " TOPIC :" + channel->getTopic_N() + "\r\n";
	send(socket, topic_message.c_str(), topic_message.length(), 0);
}
