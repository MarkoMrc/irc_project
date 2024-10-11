/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: emoreau <emoreau@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 09:14:39 by mmaric            #+#    #+#             */
/*   Updated: 2024/10/11 19:53:45 by emoreau          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <sstream>
#include <poll.h>
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

class Client;
class Channel;

class Server
{
private:
	int port;
	std::string password;
	std::vector<Client> clients;
	std::vector<Channel> channels;
	int server_socket_fd;

public:
	Server(/* args */);
	~Server();
	Server(Server const &src);
	Server &operator= (Server const &src);

	int getFd();
	int getPort();
	std::string getPassword();
	Client *getClient(int fd);
	Channel *getChannel(std::string name);

	void setFd(int fd_socket);
	void setPort(int port);
	void setPassword(std::string password);
	void addClient(Client new_client);
	void addChannel(Channel new_channel);
	
	void serv_init(int port, std::string password);

	/* methodes test de elias*/
	bool askPassword(int socket);
	void setSocketBlockingMode(int socket, bool blocking);


	/*methodes pour les cmd, le parsing*/
};

#endif