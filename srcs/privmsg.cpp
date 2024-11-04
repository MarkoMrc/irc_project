#include "../inc/Server.hpp"

void Server::handlePrivmsg(int socket, const std::string& params) {
	std::cout << "===Commande PRIVMSG reçue avec params: " << params <<  "===" << std::endl;

	Client* client = getClient(socket);
	if (!validateClient(client, socket)) return;

	std::string target = parseTarget(params);
	std::string message = parseMessage(params, target);

	if (isChannelMessage(target)) {
		handleChannelMessage(client, target, message);
	} else {
		handleDirectMessage(client, target, message);
	}
}

bool Server::validateClient(Client* client, int socket) {
	if (!client) {
		std::cerr << "Client not found for socket: " << socket << std::endl;
		return false;
	}
	return true;
}

std::string Server::parseTarget(const std::string& params) {
	std::istringstream iss(params);
	std::string target;
	iss >> target;
	return target;
}

std::string Server::parseMessage(const std::string& params, const std::string& target) {
	return params.substr(target.length() + 1);
}

bool Server::isChannelMessage(const std::string& target) {
	return !target.empty() && target[0] == '#';
}

void Server::handleChannelMessage(Client* client, const std::string& target, const std::string& message) {
	Channel* channel = getChannel(target);
	if (!validateChannel(channel, target)) return;
	if (!channel->isClient(*client)) {
		std::cerr << "le client n'appartient pas au channel spécifié" << std::endl;
		return;
	}

	std::string formatted_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
	broadcastChannelMessage(channel, client, formatted_message);

	std::cout << "Message from " << client->getNickname() << " to channel " << target << ": " << message << std::endl;
}

void Server::broadcastChannelMessage(Channel* channel, Client* client, const std::string& message) {
	const std::vector<Client*>& channelClients = channel->getClients();
	for (std::vector<Client*>::const_iterator it = channelClients.begin(); it != channelClients.end(); ++it) {
		Client* channelClient = *it;
		if (channelClient->getFd() != client->getFd()) {
			send(channelClient->getFd(), message.c_str(), message.length(), 0);
		}
	}
}

void Server::handleDirectMessage(Client* client, const std::string& target, const std::string& message) {
	Client* target_client = getClient(target);
	if (!validateTargetClient(target_client, target)) return;

	std::string formatted_message = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
	send(target_client->getFd(), formatted_message.c_str(), formatted_message.length(), 0);

	std::cout << "Message from " << client->getNickname() << " to " << target << ": " << message << std::endl;
}

bool Server::validateTargetClient(Client* target_client, const std::string& target) {
	if (!target_client) {
		std::cerr << "Client " << target << " not found." << std::endl;
		return false;
	}
	return true;
}