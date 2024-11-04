/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lebronen <lebronen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 09:14:39 by mmaric            #+#    #+#             */
/*   Updated: 2024/11/03 12:23:28 by lebronen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sstream>
#include <poll.h>
#include <sys/epoll.h>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include "Channel.hpp"
#include "Client.hpp"
#include <map>
#include <algorithm>
// #include "Signal.hpp"

#define MAX_EVENTS 10

class Client;
class Channel;

class Server
{
private:
	int port;
	std::string password;
	std::vector<Client*> clients;
	std::vector<Channel*> channels;
	static Server *server;
	int server_socket_fd;
	int epoll_fd;
	bool firstConnexion;
	void handleCapLs(int socket);
	void handlePass(int socket, const std::string& params, bool firstConnexion, std::string nick, std::string user);
	void handleNick(int socket, const std::string& params);
	void handleUser(int socket, const std::string& params);
	void handleOper(int socket, const std::string& params);
	// void handleMode(int socket, const std::string& params);
	void handleQuit(int socket, const std::string& params);
	void handleJoin(int socket, const std::string& params);
	void handleTopic(int socket, const std::string& params);
	void handleKick(int socket, const std::string& params);
	void handlePrivmsg(int socket, const std::string& params);
	void handleInvite(int socket, const std::string& params);
	void handleOperatorMode(Channel* channel, const std::vector<std::string>& words);
    void handleInviteOnlyMode(Channel* channel, const std::vector<std::string>& words);
    void handleTopicProtectionMode(Channel* channel, const std::vector<std::string>& words);
    void handlePasswordMode(Channel* channel, const std::vector<std::string>& words);
    void handleLimitMode(Channel* channel, const std::vector<std::string>& words);
	// std::vector<std::string> split(const std::string& str, char delimiter);
	std::vector<std::string> parsJoin(const std::string& params);
	bool checkNick(const std::string& params);

	//HANDLE
	void processReceivedData(int socket, Client* client, char* buffer, int valread);
	void extractCommands(std::string& received_data, std::vector<std::string>& commands);
	void handleCommands(int socket, Client* client, const std::vector<std::string>& commands, std::string& pass);
	void processCommand(int socket, const std::string& command_line, std::string& pass);
	void handlePassCommand(int socket, const std::vector<std::string>& paramList, Client* client, std::string& pass);
	void handleNickCommand(int socket, const std::vector<std::string>& paramList, Client* client);
	void handleUserCommand(int socket, const std::string& params, Client* client);
	void handleModeCommand(int socket, const std::vector<std::string>& paramList);
	void handleUnknownCommand(int socket, Client* client, const std::string& command);
	void handleClientDisconnection(int socket, Client* client);
	void notifyClientsOfDisconnection(Channel* channel, Client* client);
	void handleReceiveError(int socket);


	//JOIN
	 std::vector<std::string> parseJoinParams(const std::string& params);
    bool isValidChannelName(const std::string& channel_name);
    Channel* getOrCreateChannel(const std::string& channel_name, Client* client, const std::vector<std::string>& args);
    void checkChannelMembership(Channel* channel, Client* client, const std::string& channel_name);
    bool validateChannelAccess(Channel* channel, Client* client, const std::vector<std::string>& args, int socket);
    void addClientToChannel(Channel* channel, Client* client, int socket);
    void sendJoinMessages(Channel* channel, Client* client, const std::string& channel_name, int socket);

	//TOPIC
	// void handleTopic(int socket, const std::string& params);
	void setChannelTopic(Client* client, Channel* channel, const std::string& channel_name, const std::string& topic);
	void sendCurrentTopic(Client* client, Channel* channel, const std::string& channel_name);
	void sendMessageToChannelClients(Channel* channel, const std::string& message);

	//KICK
	// void handleKick(int socket, const std::string& params);
	std::vector<std::string> parseKickParams(const std::string& params);
	std::string extractKickReason(const std::string& params, const std::vector<std::string>& parsedParams);
	Client* validateKicker(int socket);
	Channel* validateChannel(const std::string& channelName, Client* kicker);
	Client* validateKickee(const std::string& targetNickname, Channel* channel);
	void executeKick(Client* kicker, Client* kickee, Channel* channel, const std::string& channelName, const std::string& targetNickname, const std::string& reason);

	//PRIVMSG
	// void handlePrivmsg(int socket, const std::string& params);
	bool validateClient(Client* client, int socket);
	std::string parseTarget(const std::string& params);
	std::string parseMessage(const std::string& params, const std::string& target);
	bool isChannelMessage(const std::string& target);
	void handleChannelMessage(Client* client, const std::string& target, const std::string& message);
	// bool validateChannel(Channel* channel, const std::string& target);
	void broadcastChannelMessage(Channel* channel, Client* client, const std::string& message);
	void handleDirectMessage(Client* client, const std::string& target, const std::string& message);
	bool validateTargetClient(Client* target_client, const std::string& target);

	//INVITE
	// void handleInvite(int socket, const std::string& params);
	std::vector<std::string> parseParams(const std::string& params);
	bool validateParams(const std::vector<std::string>& parsedParams);
	bool validateInviter(Client* inviter, int socket);
	bool validateChannel(Channel* channel, const std::string& channelName);
	bool canInvite(Client* inviter, Channel* channel);
	bool validateInvitee(Client* invitee, const std::string& targetNickname);
	void sendInvite(Client* inviter, Client* invitee, const std::string& channelName);
	void sendConfirmation(Client* inviter, int socket, const std::string& targetNickname, const std::string& channelName);

	//SIGNAL
	// void signal_handler(int signal);
	// void catch_signal();

public:
	Server(/* args */);
	~Server();
	Server(Server const &src);
	Server &operator= (Server const &src);

	int getFd();
	int getPort();
	bool getFirstConnexion();
	std::string getPassword();
	Client *getClient(int fd);
	Channel *getChannel(std::string name);
	Client *getClient(const std::string& nickname);
	std::vector<Client*>& getClients();
	static Server *getServer();

	void setFd(int fd_socket);
	void setPort(int port);
	void setFirstConnexion(bool fc);
	bool isNewClient(int client_socket) const;
	void setPassword(std::string password);
	void addClient(Client* new_client);
	void addChannel(Channel* new_channel);

	void serv_init(int port, std::string password);

	void createSocket();
    void configureSocket();
    void bindSocket();
    void startListening();
    void initializeEpoll();

	/* methodes test de elias*/
	bool askPassword(int socket);
	void setSocketBlockingMode(int socket);

	//MODE
	void handleMode(int socket, const std::string& params);
    // std::vector<std::string> parseParams(const std::string& params);
    void processModeCommand(Channel* channel, Client* client, const std::vector<std::string>& words);

    int createClientSocket();
    bool handleNewClient(int client_socket);
    void addSocketToEpoll(int client_socket);
    void createAndAddClient(int client_socket);

	/*methodes pour les cmd, le parsing*/
	void handleConnection(int socket);
	void processReceivedData(int socket, char* buffer, int valread, Client* client);
    // void extractCommands(std::string& received_data, std::vector<std::string>& commands);
    void handleCommand(int socket, const std::string& command, Client* client);
    void handleClientDisconnect(int socket);
    // void handleReceiveError(int socket);
	void run();
	void acceptClient();
	bool isRegistered(int socket);
	void authenticateClient(int socket, const std::string& password, const std::string& nickname, const std::string& username);

	// methode pour fermer tous les fds
	void closing_sockets();
	void removeClient(Client* client);
	void signal_handler(int signal);
	void catch_signal();

	void closeDescriptors();

	
};

#endif
