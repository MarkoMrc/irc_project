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

int main(int argc, char **argv)
{
	Server server;
	if (argc != 3)
	{
		std::cout << argv[0] << " port number password" << std::endl;
		return 1;
	}
	std::cout << "-------SERVER-------" << std::endl;
	server.serv_init(std::atoi(argv[1]), argv[2]);
	server.run();
	return EXIT_SUCCESS;
}
