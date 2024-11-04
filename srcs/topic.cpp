#include "../inc/Server.hpp"

void Server::handleTopic(int socket, const std::string& params) {
	std::cout << "===Commande TOPIC reçue avec params: " << params << "===" << std::endl;
		
	Client* client = getClient(socket);
	if (!client) return;

	std::istringstream iss(params);
	std::string channel_name, topic;
	iss >> channel_name;
	getline(iss, topic);

	Channel* channel = getChannel(channel_name);
	if (channel) {
		if (!topic.empty()) {
			setChannelTopic(client, channel, channel_name, topic);
		} else {
			sendCurrentTopic(client, channel, channel_name);
		}
	} else {
		std::cerr << "Channel " << channel_name << " not found." << std::endl;
		const char *msg = "ERR_NOSUCHCHANNEL (403) : Le channel spécifié n'existe pas.\r\n";
		send(socket, msg, strlen(msg), 0);
	}
}

void Server::setChannelTopic(Client* client, Channel* channel, const std::string& channel_name, const std::string& topic) {
	channel->setTopic_n(topic);
	std::cout << "Topic for channel " << channel_name << " set to: " << topic << std::endl;

	std::string topic_message = ":" + client->getNickname() + " TOPIC " + channel_name + " :" + topic + "\r\n";
	sendMessageToChannelClients(channel, topic_message);
}

void Server::sendCurrentTopic(Client* client, Channel* channel, const std::string& channel_name) {
	std::string topic_name = channel->getTopic_N();
	std::string topic_message = ":" + client->getNickname() + " TOPIC " + channel_name + " :" + topic_name + "\r\n";
	sendMessageToChannelClients(channel, topic_message);
}

void Server::sendMessageToChannelClients(Channel* channel, const std::string& message) {
	const std::vector<Client*>& clients = channel->getClients();
	for (std::vector<Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		send((*it)->getFd(), message.c_str(), message.length(), 0);
	}
}