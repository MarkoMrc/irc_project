#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Server.hpp"
#include "Client.hpp"

class Client;

class Channel
{
private:
	int topic;
	int key;
	int max;
	std::string name;
	std::string password;
	std::string topic_n;
	std::vector<Client> clients;
	std::vector<Client> admins;

public:
	Channel();
	~Channel();
	Channel(Channel const &src);
	Channel &operator=(Channel const &src);


	void setTopic(int topic);
	void setKey(int key);
	void setMax(int max);
	void setTopic_n(std::string topic_n);
	void setName(std::string name);
	void setPassword(std::string password);

	std::string getName();
	std::string getPassword();
	std::string getTopic_N();
	int getTopic();
	int getMax();
	int getClient_N();
	Client *getClient(int fd);
	Client *getAdmin(int fd);

	void addClient(Client new_client);
	void addAdmin(Client new_client);
	void removeClient(int fd);
	void removeAdmin(int fd);

	std::vector<Client>& getClients();
};


#endif