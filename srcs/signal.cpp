/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*    signal.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: elias <elias@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 19:11:13 by elias             #+#    #+#             */
/*   Updated: 2024/10/18 19:14:55 by elias            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Server.hpp"


// void Server::closeDescriptors() {
//     if (epoll_fd != -1) {
//         close(epoll_fd);
//     }
//     if (server_socket_fd != -1) {
//         close(server_socket_fd);
//     }
//     // Fermez d'autres descripteurs si nécessaire
// }

void signal_handler(int signal)
{
    int sig = (int)signal;
    if (sig == SIGINT || sig == SIGTERM || sig == SIGTSTP || sig == SIGQUIT)
    {
        std::cout << "Signal reçu. Arrêt du serveur." << std::endl;
        // Fermeture propre des connexions, libération des ressources, etc.
        Server *server = Server::getServer();
		// if (server){
		// 	server->closeDescriptors();
		// }
        server->closing_sockets();
        delete(server);
        exit(0);
    }
    // if (signal == SIGHUP)
    //     std::cout << "Signal SIGHUP reçu. Rechargement de la configuration." << std::endl;
    //     // Rechargement de la configuration du serveur
        // Gérer la déconnexion du client
    //  std::cout << "Signal inconnu reçu." << std::endl;
}

void catch_signal()
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGPIPE, SIG_IGN);
	signal(SIGQUIT, signal_handler);
	signal(SIGTSTP, signal_handler);
}