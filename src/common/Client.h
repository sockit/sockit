/*
 * Client.h
 *
 *  Created on: Jun 10, 2011
 *      Author: jtedesco
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "NetworkObject.h"

/**
 * An abstract interface for a client object. This registers the generic client-specific events to javascript, and registers
 * 	the 'send' method to the javascript API.
 */
class Client: public NetworkObject
{
	public:

		/**
		 * Constructs a default client object by registering the 'send' method for this client to the javascript.
		 */
		Client();

		/**
		 * Deconstructs a client object
		 */
		virtual ~Client() {}

		/**
		 * Asynchronously sends data to the remote host to which this client is connected.
		 *
		 * 	@param	data	The data to send across the wire
		 */
		virtual void send(const string & data) = 0;

		/**
		 * Asynchronously sends bytes to the remote host to which this client is connected.
		 *
		 * 	@param	bytes	The bytes of data to send across the wire
		 */
		virtual void send_bytes(const vector<byte> & bytes) = 0;

		/**
		 * Get host to which this client connects
		 *
		 * 	@return	The host to which this client connects
		 */
		virtual string get_host() = 0;

		/**
		 * Get the port on the remote host to this client connects
		 *
		 *	@return	The port on the remote host to which this client connects
		 */
		virtual int get_port() = 0;

		/**
		 * A javascript event to be fired when the client has successfully resolved the remote host name and is
		 * 	prepared to send data to the remote host.
		 */
		FB_JSAPI_EVENT(resolve, 0, ());
};


#endif /* CLIENT_H_ */
