/*
 * Server.h
 *
 *  Created on: Jun 10, 2011
 *      Author: jtedesco
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "NetworkObject.h"

/**
 * An abstract interface for a server object. This registers the generic server-specific events to javascript, and registers
 * 	the 'start_listening' method to the javascript API.
 */
class Server: public NetworkObject
{
	public:

		/**
		 * Default constructor for a server object, which registers the 'start_listening' function to the javascript API.
		 */
		Server();

		/**
		 * Deconstructs a server object
		 */
		virtual ~Server() {}

		/**
		 * Called to start the the server listening for the first time, fires the 'open' event, and
		 * 	is exposed to the javascript. This should only be called once, and starts the server listening
		 * 	for incoming connections.
		 */
		virtual void start_listening() = 0;

		/**
		 * Returns the port on which this server listens
		 */
		virtual int get_port() = 0;

		/**
		 * A javascript event to be fired when the server has started listening for incoming connections successfully.
		 */
		FB_JSAPI_EVENT(open, 0, ());
};


#endif /* SERVER_H_ */
