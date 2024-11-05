/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mmaric <mmaric@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 19:11:13 by elias             #+#    #+#             */
/*   Updated: 2024/11/05 14:30:30 by mmaric           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../inc/Server.hpp"


void signal_handler(int signal)
{
    int sig = (int)signal;
    if (sig == SIGINT || sig == SIGTERM || sig == SIGTSTP || sig == SIGQUIT)
    {
        std::cout << "Signal reçu. Arrêt du serveur." << std::endl;
        Server *server = Server::getServer();
        server->closing_sockets();
        delete(server);
        exit(0);
    }
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