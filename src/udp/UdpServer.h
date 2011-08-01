#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "Udp.h"
#include "Server.h"
#include "Logger.h"
#include "Event.h"
#include "UdpEvent.h"

using boost::asio::ip::udp;

class UdpServer;
class UdpEvent;

/**
 * This class represents a UDP server, which inherits basic UDP handling functionality from <code>Udp</code>, and
 * 	defines additional functionality to bind to a port and accept incoming connections.
 */
class UdpServer: public Udp, public Server
{
	public:

		/**
		 * A constructor that creates a UDP server using IPv4. There are two constructors to satisfy Boost's requirement that sockets are
		 * 	initialized are the initialization list, and to provide the option for using either IPv4 and IPv6. This initializes a UDP server
		 * 	but does not start it listening for incoming connections.
		 *
		 * 	@param	port				The port on which this UDP server should listen
		 * 	@param	io_service			The I/O service to use for asynchronous I/O requests
		 */
		UdpServer(int port, boost::asio::io_service & io_service);

		/**
		 * A constructor that creates a UDP server using IPv6. There are two constructors to satisfy Boost's requirement that sockets are
		 * 	initialized are the initialization list, and to provide the option for using either IPv4 and IPv6. This initializes a UDP server
		 * 	but does not start it listening for incoming connections.
		 *
		 * 	@param	port				The port on which this UDP server should listen
		 * 	@param	io_service			The I/O service to use for asynchronous I/O requests
		 *  @param options				A map of additional options to configure this UDP server
		 */
		UdpServer(int port, boost::asio::io_service & io_service, map<string, string> options);

		/**
		 * Deconstructs a UDP server, immediately closing the incoming socket and pending operations.
		 */
		~UdpServer();

		/**
		 * Called to start the the server listening for the first time, fires the 'open' event, and
		 * 	is exposed to the javascript. This should only be called once, and starts the server listening
		 * 	for incoming connections.
		 */
		virtual void start_listening();

		/**
		 * Gracefully shutdown this UDP server, waiting until all sends have completed before freeing all resources for
		 * 	this UDP server and shutting down any open connections. This function is exposed the javascript API.
		 */
		virtual void shutdown();

		/**
		 * Returns the port on which this server listens
		 */
		virtual int get_port();

		friend class UdpEvent;

	protected:

		/**
		 * Helper to fire an error event to javascript.
		 *
		 * 	@param	message	The error message
		 */
		virtual void fire_error_event(const string & message);

		/**
		 * Helper to fire data event to javascript.
		 *
		 * 	@param	data	The data received
		 * 	@param	socket	The socket on which to reply to this data
		 * 	@param	endpoint The connected endpoint to the remote host
		 */
		virtual void fire_data_event(const string data, boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint);

		/**
		 * Immediately closes the incoming socket & acceptor, and stops all pending operations.
		 */
		virtual void close();

	private:

		/**
		 * Disallows copying a UDP server.
		 */
		UdpServer(const UdpServer &other);

		/**
		 * Helper function to initialize this UDP server's object by exposing the <code>start_listening</code> method
		 * 	to javascript as 'listen'.
		 */
		void initialize(void);

		/**
		 * Helper function to listen for new data from incoming connections.
		 */
		virtual void listen(void);

		/**
		 * The socket for incoming communications to this server.
		 */
         boost::shared_ptr<udp::socket> socket;

        /** A flag to indicate this object is already listening */
        bool listening;
};

#endif
