/*
 * File:   TcpClient.h
 * Author: jtedesco
 *
 * Created on May 26, 2011, 12:32 PM
 */

#ifndef TCPCLIENT_H
#define	TCPCLIENT_H

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <string.h>

#include "Client.h"
#include "Logger.h"
#include "Tcp.h"

using boost::asio::ip::tcp;

/**
 * This class represents a TCP client, which inherits basic TCP handling functionality from <code>Tcp</code>,
 * 	and defines additional functionality to resolve and connect to a remote host.
 */
class TcpClient: public Tcp, public Client
{

	public:

		/**
		 * Builds a TCP client to connect to a specified remote host and port, and begins asynchronously resolving and
		 * 	connecting to the remote host, given an I/O service to asynchronously perform I/O requests.
		 *
		 * 	@param	host		The hostname of the remote host to which this client will connect
		 * 	@param	port		The port of the remote host to which this client will connect
		 * 	@param	io_service	The I/O service to use to perform asynchronous I/O requests
		 */
		TcpClient(const string & host, int port, boost::asio::io_service & io_service);

		/**
		 * Builds a TCP client to connect to a specified remote host and port, and begins asynchronously resolving and
		 * 	connecting to the remote host, given an I/O service to asynchronously perform I/O requests.
		 *
		 * 	@param	host		The hostname of the remote host to which this client will connect
		 * 	@param	port		The port of the remote host to which this client will connect
		 * 	@param	io_service	The I/O service to use to perform asynchronous I/O requests
         * 	@param  options     A map of options specifying the behavior of the socket
		 */
		TcpClient(const string & host, int port, boost::asio::io_service & io_service, map<string, string> options);

		/**
		 * Deconstructs a TCP client, immediately calling <code>close</code> to shutdown this client's socket and stop
		 * 	listening for responses.
		 */
		virtual ~TcpClient();

		/**
		 * Asynchronously sends data to the remote host to which this client is connected.
		 *
		 * 	@param	data	The data to send across the wire
		 */
		virtual void send(const string & data);

		/**
		 * Asynchronously sends bytes to the remote host to which this client is connected.
		 *
		 * 	@param	bytes	The bytes of data to send across the wire
		 */
		virtual void send_bytes(const vector<byte> & bytes);

		/**
		 * Gracefully shutdown this TCP client, waiting until all sends have completed before freeing all resources for
		 * 	this TCP client and shutting down any open connections. This function is exposed the javascript API.
		 */
		virtual void shutdown();

		/**
		 * Returns the port of the remote host on which this client connects
		 */
		virtual int get_port();

		/**
		 * Returns the host to which this client connects
		 */
		virtual string get_host();

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
		 * Immediately cancels any pending operations and closes this client's socket.
		 */
		virtual void close();

	private:

		/**
		 * Disallows copying a TCP client
		 */
		TcpClient(const TcpClient &other);

        /**
         * Helper function to initialize this client
         */
        void init();

        /**
         * Initialize the properties of this socket
         */
        void init_socket();

		/**
		 * I/O handler invoked when the remote host is resolved. This handler attempts to asynchronously establish
		 * 	a TCP connection with the remote host.
		 *
		 * 	@param	error_code	The error code encountered when trying to receive data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	endpoint_iterator	Allows the client to iterate through resolvers to retry the connection if it
		 * 								initially fails to connect.
		 */
		void resolve_handler(const boost::system::error_code & error_code, tcp::resolver::iterator endpoint_iterator);

		/**
		 * I/O handler invoked when this client has attempted to connect to its remote host. If the connection failed,
		 * 	this handler will reattempt the connection by using another resolver if possible, and otherwise fail permanently.
		 *
		 * 	@param	error_code	The error code encountered when trying to receive data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	endpoint_iterator	Allows the client to iterate through resolvers to retry the connection if it
		 * 								initially fails to connect.
		 */
		void connect_handler(const boost::system::error_code & error_code, tcp::resolver::iterator endpoint_iterator);

		/**
		 * Helper function that will listen for incoming data on the TCP connection for the client, specifically responses
		 * 	to data already sent.
		 */
		void listen();

		/**
		 * Helper function to flush the queue of data if sends are requested before the client is connected
		 */
		void flush();

		/**
		 * A shared reference to the socket used to connect to the remote host
		 */
		boost::shared_ptr<tcp::socket> connection;

		/**
		 * Resolver object provided by <code>boost</code> to resolve the remote hostname and port
		 */
		boost::shared_ptr<tcp::resolver> resolver;

		/**
		 * A flag recording whether this client is connected yet or not, used to queue send requests made before this client
		 *  is connected if necessary.
		 */
		bool connected;

		/**
		 * A mutex used to access the queue for pending jobs
		 */
		boost::mutex connected_mutex;

		/**
		 * A queue of data to be sent, which fills as 'send' or 'send_bytes' commands occur before the client is connected
		 */
		queue<string> data_queue;

		/**
		 * A mutex used to access the queue for pending jobs
		 */
		boost::mutex data_queue_mutex;
};

#endif	/* TCPCLIENT_H */

