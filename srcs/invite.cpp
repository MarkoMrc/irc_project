#include "../inc/Server.hpp"


void Server::handleInvite(int socket, const std::string& params) {
	std::cout << "===Commande INVITE reçue avec params: " << params << "===" << std::endl;

	std::vector<std::string> parsedParams = parseParams(params);
	if (!validateParams(parsedParams)){
		const char *msg = "ERR_NEEDMOREPARAMS (461) : Pas assez de paramètres fournis pour la commande INVITE\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	std::string targetNickname = parsedParams[0];
	std::string channelName = parsedParams[1];

	Client* inviter = getClient(socket);
	if (!validateInviter(inviter, socket)) return;

	Channel* channel = getChannel(channelName);
	if (!validateChannel(channel, channelName)){
		const char *msg = "ERR_NOSUCHCHANNEL (403) : Le channel spécifié n'existe pas\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}
	if (!canInvite(inviter, channel, socket))
		return;


	Client* invitee = getClient(targetNickname);
	if (!validateInvitee(invitee, targetNickname))
	{
		const char *msg = "ERR_NOSUCHNICK (401) : Le nickname n'existe pas.\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	if (channel->isClient(*invitee)) {
		std::cerr << "Erreur : " << targetNickname << " est déjà dans le canal " << channelName << std::endl;
		const char *msg = "ERR_USERONCHANNEL (443) : The target is already on channel\r\n";
		send(socket, msg, strlen(msg), 0);
		return;
	}

	channel->inviteClient(invitee);
	sendInvite(inviter, invitee, channelName);
	sendConfirmation(inviter, socket, targetNickname, channelName);
}

std::vector<std::string> Server::parseParams(const std::string& params) {
	std::vector<std::string> parsedParams;
	std::istringstream iss(params);
	std::string word;

	while (iss >> word) {
		parsedParams.push_back(word);
	}
	return parsedParams;
}

bool Server::validateParams(const std::vector<std::string>& parsedParams) {
	if (parsedParams.size() < 2) {
		std::cerr << "Erreur : Paramètres insuffisants pour la commande INVITE" << std::endl;
		return false;
	}
	return true;
}

bool Server::validateInviter(Client* inviter, int socket) {
	if (!inviter) {
		std::cerr << "Erreur : client émetteur introuvable pour le socket " << socket << std::endl;
		return false;
	}
	return true;
}

bool Server::validateChannel(Channel* channel, const std::string& channelName) {
	if (!channel) {
		std::cerr << "Erreur : le canal " << channelName << " n'existe pas" << std::endl;
		return false;
	}
	return true;
}

bool Server::canInvite(Client* inviter, Channel* channel, int socket) {
	if (!channel->isClient(*inviter)) {
		std::cerr << "Erreur : Client n'est pas membre du canal " << channel->getName() << std::endl;
		const char *msg = "ERR_NOTONCHANNEL (442) : Vous n'etes pas sur le channel.\r\n";
		send(socket, msg, strlen(msg), 0);
		return false;
	}
	if (!channel->isAdmin(*inviter)) {
		std::cerr << "Erreur : Client n'a pas la permission d'inviter dans ce canal" << std::endl;
		const char *msg = "ERR_CHANOPRIVSNEEDED (482) : L'utilisateur tente d'inviter sur un channel sans être opérateur.\r\n";
		send(socket, msg, strlen(msg), 0);
		return false;
	}
	return true;
}

bool Server::validateInvitee(Client* invitee, const std::string& targetNickname) {
	if (!invitee) {
		std::cerr << "Erreur : le client " << targetNickname << " n'existe pas" << std::endl;
		return false;
	}
	return true;
}

void Server::sendInvite(Client* inviter, Client* invitee, const std::string& channelName) {
	std::string inviteMessage = ":" + inviter->getNickname() + " INVITE " + invitee->getNickname() + " :" + channelName + "\r\n";
	send(invitee->getFd(), inviteMessage.c_str(), inviteMessage.length(), 0);
	std::cout << "Invitation envoyée de " << inviter->getNickname() << " à " << invitee->getNickname() << " pour rejoindre le canal " << channelName << std::endl;
}

void Server::sendConfirmation(Client* inviter, int socket, const std::string& targetNickname, const std::string& channelName) {
	std::string confirmationMessage = ":" + inviter->getNickname() + " INVITE " + targetNickname + " " + channelName + "\r\n";
	send(socket, confirmationMessage.c_str(), confirmationMessage.length(), 0);
}