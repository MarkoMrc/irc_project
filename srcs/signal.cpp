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

void Server::signal_handler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        std::cout << "Signal SIGINT reçu. Arrêt du serveur." << std::endl;
        // Fermeture propre des connexions, libération des ressources, etc.
        exit(0);
    }
    // if (signal == SIGHUP)
    //     std::cout << "Signal SIGHUP reçu. Rechargement de la configuration." << std::endl;
    //     // Rechargement de la configuration du serveur
        // Gérer la déconnexion du client
    //  std::cout << "Signal inconnu reçu." << std::endl;
}

void Server::catch_signal()
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);
    signal(SIGPIPE, SIG_IGN);
}