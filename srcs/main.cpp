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
	std::string mdp = server.getPassword();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Créer un socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
        perror("Échec de la création du socket");
        exit(EXIT_FAILURE);
    }

    // Configurer le socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
        perror("Échec de setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server.getPort());

    // Attacher le socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
        perror("Échec du bind");
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_fd, 3) < 0)
	{
        perror("Échec de listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Serveur IRC démarré. En attente de connexions..." << std::endl;

    while (true)
	{
        // Accepter une nouvelle connexion
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		{
            perror("Échec d'acceptation");
            exit(EXIT_FAILURE);
        }
        server.setSocketBlockingMode(new_socket, 1);
		server.askPassword(new_socket);
        std::cout << "Nouvelle connexion acceptée, adresse IP: " << inet_ntoa(address.sin_addr) << std::endl;
        
        // Envoyer un message de bienvenue
        const char *welcome_message = "Bienvenue sur ce serveur IRC!\n";
        send(new_socket, welcome_message, strlen(welcome_message), 0);

        // Fermer la connexion pour cet exemple simple
        // close(new_socket);
    }

    return 0;
}
