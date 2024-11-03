/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rshay <rshay@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 09:14:43 by mmaric            #+#    #+#             */
/*   Updated: 2024/10/21 17:13:33 by rshay            ###   ########.fr       */
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
	std::string servername;
	std::string realname;
	std::string tmp_nick;
	std::string tmp_user;
	std::string ip_add;
	std::string buff;
	std::string hostname;
	bool isOperator;
	bool registered;
	bool logged_in;
	bool pswd_entered;
	bool nouveau;

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
	std::string getRealname();
	std::string getServername();
	std::string getNickname() const;
	int getFd() const;
	std::string getIp_add();
	std::string getBuff();
	std::string getHostname();
	std::string getTmpNick();
	std::string getTmpUser();
	bool estNouveau();

	/*##SETTERS##
	#nickname, fd, ip
	*/
	void setUsername(const std::string& username);
	void setServername(const std::string& servername);
	void setRealname(const std::string& realname);
	void setNickname(const std::string& nickname);
	void setTmpNick(const std::string& tmp_nick);
	void setTmpUser(const std::string& tmp_user);
	void setHostname(const std::string& hostname);
	void setIsOperator(bool isOperator);
	void setFd(int fd);
	void setIp_add(std::string ip_add);
	void setBuff(std::string buff);
	void setNouveau(bool b);
	/*#METHODS*/
	 // Nouvelle methode pour definir si le client est authentifie
	void setLogged(bool status);
	void setPswdEnterd(bool status);


	// Methode pour verifier si le client est authentifie
	bool isLogged() const;
	bool isPswdEnterd() const;
};

#endif
