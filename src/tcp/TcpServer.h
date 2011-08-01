/*
 * File:   TcpServer.h
 * Author: jtedesco
 *
 * Created on May 26, 2011, 12:33 PM
 */

#ifndef TCPSERVER_H
#define	TCPSERVER_H

#include<set>

#include "Tcp.h"
#include "Server.h"
#include "Logger.h"

using boost::asio::ip::tcp;
using std::set;

/**
 * This class represents a TCP server, which inherits basic TCP handling functionality from <code>Tcp</code>, and
 * 	defines additional functionality to bind to a port and accept incoming connections.
 */
class TcpServer: public Tcp, public Server
{

	public:

		/**
		 * Builds a TCP server to listen on the specified port, but does not start the server listening on it. Using
		 * 	this constructor uses IPv4, given an I/O service to use for asynchronous I/O.
		 *
		 * 	@param	port		The port on which the server should listen
		 * 	@param	io_service	The I/O service to use for background I/O requests
		 */
		TcpServer(int port, boost::asio::io_service & io_service);

		/**
		 * Builds a TCP server to listen on the specified port, but does not start the server listening on it. Using
		 * 	this constructor creates a server using IPv6, given an I/O service to use for asynchronous I/O.
		 *
		 * 	@param	port		The port on which the server should listen
		 * 	@param	io_service	The I/O service to use for background I/O requests
         * 	@param  options     A map of options specifying the behavior of the socket
		 */
		TcpServer(int port, boost::asio::io_service & io_service, map<string, string> options);

		/**
		 * Deconstructs this TCP server, by immediately ceasing to accept incoming connections, shutdown all necessary
		 * 	resources.
		 */
		virtual ~TcpServer();

		/**
		 * Called to start the the server listening for the first time, fires the 'open' event, and
		 * 	is exposed to the javascript. This should only be called once, and starts the server listening
		 * 	for incoming connections.
		 */
		virtual void start_listening();

		/**
		 * Gracefully shutdown this TCP server, waiting until all sends have completed before freeing all resources for
		 * 	this TCP server and shutting down any open connections. This function is exposed the javascript API.
		 */
		virtual void shutdown();

		/**
		 * Returns the port on which this server listens
		 */
		virtual int get_port();

		/**
		 * The javascript event fired on a disconnect-type network error. This is fired on the following <code>boost</code> errors:
		 * 	<ul>
		 * 		<li><code>boost::asio::error::connection_reset</code></li>
		 * 		<li><code>boost::asio::error::eof</code></li>
		 * 		<li><code>boost::asio::error::connection_aborted</code></li>
		 * 		<li><code>boost::asio::error::operation_aborted</code></li>
		 * 	</ul>
		 */
		FB_JSAPI_EVENT(disconnect, 1, (const string &));

		/**
		 * The javascript event fired once a client has connected to its remote endpoint, or once a server has accepted a connection successfully (but has not yet receieved data), in this class, the former.
		 */
		FB_JSAPI_EVENT(connect, 0, ());

	protected:


		/**
		 * Inherited from Tcp. Handler invoked when the data has been sent (or sending terminated in error).
		 *
		 */
        void receive_handler(const boost::system::error_code & error_code, std::size_t bytesTransferred,
                boost::shared_ptr<tcp::socket> connection, string host, int port);

		/**
		 * Helper to fire an error event to javascript.
		 *
		 * 	@param	message	The error message
		 */
		virtual void fire_error_event(const string & message);

		/**
		 * Helper to fire an disconnect error event to javascript.
		 *
		 * 	@param	message	The error message
		 */
		virtual void fire_disconnect_event(const string & message);

		/**
		 * Helper to fire data event to javascript.
		 *
		 * 	@param	data	The data received
		 * 	@param	connection	The connection on which to reply to the data
		 */
		virtual void fire_data_event(const string data, boost::shared_ptr<tcp::socket> connection);

		/**
		 * Closes down this TCP server immediately, by immediately ceasing to accept incoming connections, shutdown all
		 * 	necessary resources.
		 */
		virtual void close();

	private:

		/**
		 * Helper function to initialize the acceptor. If this was not successfully initialized, <code>fail</code> is true.
		 */
		void init();

        /**
         * Initialize the properties of this socket
         */
        void init_socket(boost::shared_ptr<tcp::socket> connection);

        /**
		 * Disallows copying a TCP server
		 */
		TcpServer(const TcpServer &other);

		/**
		 * Handler invoked when a new connection is made to this server. This handler tries to start receiving data from
		 * 	the new incoming connection if there was no error accepting the connection.
		 *
		 * 	@param	error_code	The error code encountered when trying to receive data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	connection	The new connection created by this accept, from which this function will attempt to
		 * 						receive data.
		 * 	@param	host		The host from which this accept was attempted
		 * 	@param	port		The port from which this accept was attempted
		 */
		void accept_handler(const boost::system::error_code & error_code, boost::shared_ptr<tcp::socket> connection, string host, int port);

		/**
		 * The acceptor for incoming connections for this server
		 */
        boost::shared_ptr<tcp::acceptor> acceptor;
        
        /** A set of established connections. */
        set<boost::shared_ptr<tcp::socket> > connections;
};

#endif	/* TCPSERVER_H */

