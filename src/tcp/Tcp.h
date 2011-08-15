/*
 * Tcp.h
 *
 *  Created on: Jun 8, 2011
 *      Author: jtedesco
 */

#ifndef TCP_H_
#define TCP_H_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <set>
#include <map>

#if defined(__UNIX__)

#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#elif defined(__OSX__)

#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#else

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <Windows.h>
#include <MSTcpIP.h>

#endif


class Tcp;

#include "TcpEvent.h"
#include "Logger.h"

using boost::optional;
using boost::asio::ip::tcp;
using boost::asio::buffer_size;
using std::map;

/**
 * Interface to abstract out the client-server model for use in the <code>Event</code>. Contains common
 * 	handlers used by both TCP servers and clients.
 *
 * 	@see Event
 */
class Tcp
{
	public:

		/**
		 * Builds a generic TCP object, and registers the 'shutdown' method on the API.
		 *
		 * 	@param	host	The hostname for this TCP object. For clients, this represents the remote host's
		 * 					hostname, and for servers, defaults to 'localhost'.
		 * 	@param	port	The port for this TCP object, which for clients, represents the port of the remote
		 * 					host to which to connect, and for servers, represents the port to which to bind and listen
		 * 					for incoming connections.
		 *	@param	io_service	The I/O service used for asynchronous I/O requests
		 */
		Tcp(string host, int port, boost::asio::io_service & io_service);

		/**
		 * Immediately calls the <code>close</code> function, freeing all resources for this TCP object and shutting down
		 * any open connections.
		 */
		virtual ~Tcp() {}

		friend class TcpEvent;

	protected:

        /**
         * Helper for setting tcp keep alive options (varies from platform to platform).
         *
         * @param   socket  The boost socket to set the tcp keep alive options on
         * @return  true if successful, otherwise false.
         */
        bool set_tcp_keepalive(boost::shared_ptr<tcp::socket> socket);

        /**
         * Helper for logging options passed in.
         *
         * @param   arg     optional bool option
         * @param   iftrue  if the arg has been set to true, return this
         * @param   iffalse if the arg hasn't been set, or is set to false, return this
         */
        inline string bool_option_to_string(optional<bool> &arg, string iftrue, string iffalse);

        /**
         * Helper for logging options passed in.
         *
         * @param   arg     the optional argument
         */
        template <class T> inline string option_to_string(optional<T> &arg);

        /**
         * Helper for parsing boolean arguments
         *
         * @param   options     A map of strings to string representing the options passed in and their values
         * @param   arg         The specific option to check for
         * @param   arg_value   This will hold the value of the argument, if found.
         */
        void inline parse_string_bool_arg(map<string, string> &options, string arg, optional<bool> &arg_value);

        /**
         * Helper for parsing integer arguments
         *
         * @param   options     A map of strings to string representing the options passed in and their values
         * @param   arg         The specific option to check for
         * @param   arg_value   This will hold the value of the argument, if found.
         */
        void inline parse_string_int_arg(map<string, string> &options, string arg, optional<int> &arg_value);

        /**
         * Parses any options (like whether or use IPv6 or IPv4) from the options. Supported options currently include:
         *
         * ipv6             if true, use ipv6. otherwise use ipv4.
         * keep alive       allow the socket to send keep-alives.
         * do not route		option to force TCP to use local interfaces only, prevents routing
         * no delay			option to disable Nagle algorithm for possibly improved performance
         *
         * @param options   A map of options to values.
         */
        void parse_args(map<string, string> options);

        /**
         * Write to the log the options used to create this connection.
         */
        void log_options();

		/**
		 * Closes down this TCP object immediately, by immediately ceasing to accept incoming connections, shutdown all
		 * 	necessary resources.
		 */
		virtual void close() = 0;

		/**
		 * Handler invoked when the data has been sent (or sending terminated in error).
		 *
		 * 	@param	error_code	The error code encountered when trying to send data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	bytes_transferred	The number of bytes successfully sent over the connection.
		 * 	@param	data		The data to be sent.
		 * 	@param	host		The hostname for this TCP object
		 * 	@param	port		The port for this TCP object
		 * 	@param	connection	The connection this data was sent on
		 */
		virtual void send_handler(const boost::system::error_code & error_code, std::size_t bytes_transferred,
				string data, string host, int port, boost::shared_ptr<tcp::socket> connection);

		/**
		 * Handler invoked when some data has been received.
		 *
		 * 	@param	error_code	The error code encountered when trying to receive data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	bytes_transferred	The number of bytes successfully received over the connection.
		 * 	@param	connection	The connection on which attempted to receive data.
		 * 	@param	host		The hostname for this TCP object
		 * 	@param	port		The port for this TCP object
		 */
		virtual void receive_handler(const boost::system::error_code & error_code, std::size_t bytes_transferred,
				boost::shared_ptr<tcp::socket> connection, string host, int port);

		/**
		 * Helper to fire an error event to javascript.
		 *
		 * 	@param	message	The error message
		 */
		virtual void fire_error_event(const string & message) = 0;

		/**
		 * Helper to fire an disconnect error event to javascript.
		 *
		 * 	@param	message	The error message
		 */
		virtual void fire_disconnect_event(const string & message) = 0;

		/**
		 * Helper to fire data event to javascript.
		 *
		 * 	@param	data	The data received
		 * 	@param	connection	The connection on which attempted to receive data.
		 */
		virtual void fire_data_event(const string data, boost::shared_ptr<tcp::socket> connection) = 0;

		/**
		 * A constant representing the size of the buffer in which to receive data.
		 */
		static const int BUFFER_SIZE = 4096;

		/**
		 * A constant representing the maximum number of bytes that a TCP message can be in boost.
		 * If our message is larger than this, keep trying to send it until all successful bytes were sent.
		 */
		static const int MAX_DATA_SIZE = 16436;

		/**
		 * A buffer for receiving data from the remote host.
		 */
		boost::array<char, BUFFER_SIZE> receive_buffer;

		/**
		 * The I/O service for perform nonblocking actions
		 */
		boost::asio::io_service & io_service;

		/**
		 * A flag to say whether or not this server is waiting to shutdown
		 */
		bool waiting_to_shutdown;

		/**
		 * Flag to say whether or not this TCP object uses IPv6
		 */
		optional<bool> using_ipv6;

		/**
		 * Option to disable Nagle algorithm for possibly improved performance
		 */
		optional<bool> no_delay;

		/**
		 * Option to force TCP to use local interfaces only, prevents routing
		 */
		optional<bool> do_not_route;

		/**
		 * Allow the socket to send keep-alives
		 */
		optional<bool> keep_alive;

		/**
		 * Allow setting the socket's keep-alive timeout
		 */
		optional<int> keep_alive_timeout;

		/**
		 * The current count of active jobs on the socket
		 */
		int active_jobs;

		/**
		 * A mutex around the current count of active jobs on the socket
		 */
		boost::mutex active_jobs_mutex;

		/**
		 * The hostname for this TCP object ('localhost' for servers, or the hostname of the remote host for clients)
		 */
		string host;

		/**
		 * The port (the port bound to by servers, port on which to connect to the remote host for clients)
		 */
		int port;

		/**
		 * The set of <code>boost::system::error_code</code> errors considered to be 'disconnection' errors.
		 *
		 * 	@see boost::system::error_code
		 */
		std::set<boost::system::error_code> disconnect_errors;

        /**
         * A flag that is set to true whenever this TCP client or server encounters an unrecoverable error and cannot continue.
         */
        bool failed;
};

#endif /* TCP_H_ */
