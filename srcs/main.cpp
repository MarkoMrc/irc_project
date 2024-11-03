#include "../inc/Server.hpp"

// int main(int argc, char **argv)
// {
// 	Server server;
// 	if (argc != 3)
// 	{
// 		std::cout << argv[0] << " port number password" << std::endl;
// 		return 1;
// 	}
// 	std::cout << "-------SERVER-------" << std::endl;
// 	server.serv_init(std::atoi(argv[1]), argv[2]);
// 	int test = server.getPort();
// 	std::string mdp = server.getPassword();
// 	std::cout << "test " << test << " " << mdp << std::endl;
// }

void signalHandler(int signum) {
	if (signum == SIGINT)
	{
		// std::cout << "controle C" << std::endl;
		exit(1);
	}
}



int main(int argc, char **argv)
{
	Server server;
	if (argc != 3)
	{
		std::cout << argv[0] << " port number password" << std::endl;
		return 1;
	}
	if (std::atoi(argv[1]) <= 1024) {
        std::cerr << "Erreur : le port doit être supérieur à 1024." << std::endl;
        return 1; // Fin du programme
    }
	std::cout << "-------SERVER-------" << std::endl;
	std::signal(SIGINT, signalHandler);
	server.serv_init(std::atoi(argv[1]), argv[2]);
	server.run();
	return EXIT_SUCCESS;
}
