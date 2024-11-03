/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lebronen <lebronen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 19:11:13 by elias             #+#    #+#             */
/*   Updated: 2024/11/03 22:20:49 by lebronen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/Server.hpp"

void signal_handler(int signal)
{
    int sig = (int)signal;
    if (sig == SIGINT || sig == SIGTERM)
    {
        std::cout << "Signal SIGINT reçu. Arrêt du serveur." << std::endl;
        // Fermeture propre des connexions, libération des ressources, etc.
        Server *server = Server::getServer();
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
}