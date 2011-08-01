/*
 * Client.cpp
 *
 *  Created on: Jun 10, 2011
 *      Author: jtedesco
 */

#include "Client.h"

Client::Client()
{
	registerMethod("send", make_method(this, &Client::send));
	registerMethod("sendBytes", make_method(this, &Client::send_bytes));
	registerMethod("getHost", make_method(this, &Client::get_host));
	registerMethod("getPort", make_method(this, &Client::get_port));
}
