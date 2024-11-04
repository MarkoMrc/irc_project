#ifndef SIGNAL_HPP
# define SIGNAL_HPP

#include "../inc/Server.hpp"

void catch_signal();


struct signal
{
    Server *server;
};


#endif