#include "../inc/Server.hpp"
#include "../inc/Signal.hpp"

int main(int argc, char **argv)
{
	Server *server = Server::getServer();
	if (argc != 3)
	{
		std::cout << argv[0] << " port number password" << std::endl;
		return 1;
	}
	if (std::atoi(argv[1]) <= 1024) {
		std::cerr << "Erreur : le port doit être supérieur à 1024." << std::endl;
		return 1;
	}
	std::cout << "-------SERVER-------" << std::endl;
	// std::signal(SIGINT, signalHandler);
	catch_signal();
	server->serv_init(std::atoi(argv[1]), argv[2]);
	server->run();
	return EXIT_SUCCESS;
}
