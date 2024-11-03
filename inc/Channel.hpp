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
	size_t limit;
	bool modeInviteOnly; // Mode +i
	bool modeTopicOp; // Mode +t
	bool modePasswordProtected; // Mode +k
	bool modeLimit; // Mode +l
	std::string name;
	std::string password;
	std::string topic_n;
	std::vector<Client*> clients;
	std::vector<Client> admins;

public:
	Channel();
	~Channel();
	Channel(Channel const &src);
	Channel &operator=(Channel const &src);


	void setTopic(int topic);
	void setTopic_n(std::string topic_n);
	void setName(std::string name);
	void setModeInviteOnly(bool value);
	void setModeTopicOp(bool value);
	void setModePasswordProtected(bool value);
	void setModeLimit(bool value);
	void setPassword(const std::string& pass);
	void setLimit(size_t limit);


	bool isModeInviteOnly() const { return modeInviteOnly; }
    bool isModeTopicOp() const { return modeTopicOp; }
    bool isModePasswordProtected() const { return modePasswordProtected; }
    bool isModeLimit() const { return modeLimit; }

	std::string getName();
	std::string getPassword() const;
	std::string getTopic_N();
	int getTopic();
	int getMax();
	int getClient_N();
	Client *getClient(int fd);
	Client *getAdmin(int fd);

	void addClient(Client* new_client);
	void addAdmin(Client new_client);
	void removeClient(int fd);
	void removeAdmin(int fd);
	bool isAdmin(const Client& client) const;
	bool isClient(const Client& client) const;
	void broadcastMessage(const std::string& message, Client* excludeClient);
	bool isFull() const;
	

	std::vector<Client*>& getClients();
};


#endif