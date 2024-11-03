#include "../inc/Server.hpp"


void Server::handleKick(int socket, const std::string& params) {
	std::cout << "Commande KICK reçue avec params: " << params << std::endl;

	std::vector<std::string> parsedParams = parseKickParams(params);
	if (parsedParams.size() < 2) {
		std::cerr << "Erreur : Paramètres insuffisants pour la commande KICK" << std::endl;
		return;
	}

	std::string channelName = parsedParams[0];
	std::string targetNickname = parsedParams[1];
	std::string reason = extractKickReason(params, parsedParams);

	Client* kicker = validateKicker(socket);
	if (!kicker) return;

	Channel* channel = validateChannel(channelName, kicker);
	if (!channel) return;

	Client* kickee = validateKickee(targetNickname, channel);
	if (!kickee) return;

	executeKick(kicker, kickee, channel, channelName, targetNickname, reason);
}

std::vector<std::string> Server::parseKickParams(const std::string& params) {
	std::istringstream iss(params);
	std::string word;
	std::vector<std::string> parsedParams;

	while (iss >> word) {
		parsedParams.push_back(word);
	}
	return parsedParams;
}

std::string Server::extractKickReason(const std::string& params, const std::vector<std::string>& parsedParams) {
	if (parsedParams.size() > 2) {
		return params.substr(params.find(parsedParams[2]));
	}
	return "";
}

Client* Server::validateKicker(int socket) {
	Client* kicker = getClient(socket);
	if (!kicker) {
		std::cerr << "Erreur : client émetteur introuvable pour le socket " << socket << std::endl;
	}
	return kicker;
}

Channel* Server::validateChannel(const std::string& channelName, Client* kicker) {
	Channel* channel = getChannel(channelName);
	if (!channel) {
		std::cerr << "Erreur : le canal " << channelName << " n'existe pas" << std::endl;
		return NULL;
	}
	if (!channel->isClient(*kicker)) {
		std::cerr << "Erreur : vous n'êtes pas membre du canal " << channelName << std::endl;
		return NULL;
	}
	if (!channel->isAdmin(*kicker)) {
		std::cerr << "Erreur : vous n'avez pas la permission d'expulser dans ce canal" << std::endl;
		return NULL;
	}
	return channel;
}

Client* Server::validateKickee(const std::string& targetNickname, Channel* channel) {
	Client* kickee = getClient(targetNickname);
	if (!kickee) {
		std::cerr << "Erreur : le client " << targetNickname << " n'existe pas" << std::endl;
		return NULL;
	}
	if (!channel->isClient(*kickee)) {
		std::cerr << "Erreur : " << targetNickname << " n'est pas dans le canal " << channel->getName() << std::endl;
		return NULL;
	}
	return kickee;
}

void Server::executeKick(Client* kicker, Client* kickee, Channel* channel, const std::string& channelName, const std::string& targetNickname, const std::string& reason) {
	std::string kickMessage = ":" + kicker->getNickname() + " KICK " + channelName + " " + targetNickname;
	if (!reason.empty()) {
		kickMessage += " :" + reason;
	}
	kickMessage += "\r\n";

	channel->broadcastMessage(kickMessage, kickee);
	channel->removeClient(kickee->getFd());

	std::cout << "Client " << targetNickname << " expulsé du canal " << channelName << " par " << kicker->getNickname();
	if (!reason.empty()) {
		std::cout << " pour la raison : " << reason;
	}
	std::cout << std::endl;
}