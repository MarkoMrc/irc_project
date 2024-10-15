/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaric <mmaric@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 09:14:43 by mmaric            #+#    #+#             */
/*   Updated: 2024/10/02 09:14:43 by mmaric           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp"
#include "Channel.hpp"

class Client
{
private:
	int fd;
	std::string username;
	std::string nickname;
	std::string ip_add;
	std::string buff;
	std::string hostname;
	bool isOperator;
	bool registered;
	bool logged_in;
	
public:
	Client();
	Client(std::string username, std::string nickname, int fd);
	~Client();
	Client(Client const &src);
	Client &operator=(Client const &src);

	/*##GETTERS##
	#nickname, ip, fd, hostname
	*/
	std::string getUsername();
	std::string getNickname();
	int getFd();
	std::string getIp_add();
	std::string getBuff();
	std::string getHostname();

	/*##SETTERS##
	#nickname, fd, ip
	*/
	void setUsername(const std::string& username);
	void setNickname(const std::string& nickname);
	void setHostname(const std::string& hostname);
	void setIsOperator(bool isOperator);
	void setFd(int fd);
	void setIp_add(std::string ip_add);
	void setBuff(std::string buff);
	/*#METHODS*/
	 // Nouvelle méthode pour définir si le client est authentifié
    void setLogged(bool status);

    // Méthode pour vérifier si le client est authentifié
    bool isLogged() const;
};

#endif