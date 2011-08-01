/*
 * Server.cpp
 *
 *  Created on: Jun 10, 2011
 *      Author: jtedesco
 */

#include "Server.h"

Server::Server()
{
	registerMethod("listen", make_method(this, &Server::start_listening));
	registerMethod("getPort", make_method(this, &Server::get_port));
}
