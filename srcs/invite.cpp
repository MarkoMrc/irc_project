#include "../inc/Server.hpp"


void Server::handleInvite(int socket, const std::string& params) {
	std::cout << "Commande INVITE reçue avec params: " << params << std::endl;

	std::vector<std::string> parsedParams = parseParams(params);
	if (!validateParams(parsedParams)) return;

	std::string targetNickname = parsedParams[0];
	std::string channelName = parsedParams[1];

	Client* inviter = getClient(socket);
	if (!validateInviter(inviter, socket)) return;

	Channel* channel = getChannel(channelName);
	if (!validateChannel(channel, channelName)) return;

	if (!canInvite(inviter, channel)) return;

	Client* invitee = getClient(targetNickname);
	if (!validateInvitee(invitee, targetNickname)) return;

	if (channel->isClient(*invitee)) {
		std::cerr << "Erreur : " << targetNickname << " est déjà dans le canal " << channelName << std::endl;
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

bool Server::canInvite(Client* inviter, Channel* channel) {
	if (!channel->isClient(*inviter)) {
		std::cerr << "Erreur : vous n'êtes pas membre du canal " << channel->getName() << std::endl;
		return false;
	}
	if (!channel->isAdmin(*inviter)) {
		std::cerr << "Erreur : vous n'avez pas la permission d'inviter dans ce canal" << std::endl;
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