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
	int server_socket_fd;
	int epoll_fd;
	bool firstConnexion;
	void handleCapLs(int socket);
	void handlePass(int socket, const std::string& params, bool firstConnexion, std::string nick, std::string user);
	void handleNick(int socket, const std::string& params);
	void handleUser(int socket, const std::string& params);
	void handleOper(int socket, const std::string& params);
	void handleMode(int socket, const std::string& params);
	void handleQuit(int socket, const std::string& params);
	void handleJoin(int socket, const std::string& params);
	void handlePart(int socket, const std::string& params);
	void handleTopic(int socket, const std::string& params);
	void handleKick(int socket, const std::string& params);
	void handlePrivmsg(int socket, const std::string& params);
	void handleInvite(int socket, const std::string& params);
	// std::vector<std::string> split(const std::string& str, char delimiter);
	std::vector<std::string> parsJoin(const std::string& params);
	bool checkNick(const std::string& params);


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

	void setFd(int fd_socket);
	void setPort(int port);
	void setFirstConnexion(bool fc);
	bool isNewClient(int client_socket) const;
	void setPassword(std::string password);
	void addClient(Client* new_client);
	void addChannel(Channel* new_channel);

	void serv_init(int port, std::string password);

	/* methodes test de elias*/
	bool askPassword(int socket);
	void setSocketBlockingMode(int socket);


	/*methodes pour les cmd, le parsing*/
	void handleConnection(int socket);
	void run();
	void acceptClient();
	bool isRegistered(int socket);
	void authenticateClient(int socket, const std::string& password, const std::string& nickname, const std::string& username);

	// methode pour fermer tous les fds
	void closing_sockets();
	void removeClient(Client* client);
};

#endif
