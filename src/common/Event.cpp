/*
 * Event.cpp
 *
 *  Created on: Jun 24, 2011
 *      Author: jtedesco
 */

#include "Event.h"

Event::Event()
{
	// Expose these methods to the Javascript
	registerMethod("send", make_method(this, &Event::send));
	registerMethod("sendBytes", make_method(this, &Event::send_bytes));
	registerMethod("read", make_method(this, &Event::read));
	registerMethod("readBytes", make_method(this, &Event::read_bytes));
	registerMethod("getHost", make_method(this, &Event::get_host));
	registerMethod("getPort", make_method(this, &Event::get_port));
}


Event::~Event()
{
	// Nothing to do here
}
