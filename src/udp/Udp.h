/*
 * Udp.h
 *
 *  Created on: Jun 8, 2011
 *      Author: jtedesco
 */

#ifndef UDP_H_
#define UDP_H_

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <map>

class Udp;

#include "UdpEvent.h"
#include "Logger.h"

using boost::optional;
using boost::asio::buffer_size;
using boost::asio::buffer_cast;
using boost::asio::ip::udp;
using std::string;
using std::map;

typedef boost::asio::const_buffers_1 boost_buffer;

/**
 * Interface to abstract out the client-server model for use in the <code>Event</code>. Contains common
 * 	handlers used by both UDP servers and clients.
 *
 * 	@see Event
 */
class Udp
{
	public:

		/**
		 * Builds a generic UDP object, and registers the 'shutdown' method on the API.
		 *
		 * 	@param	host	The hostname for this UDP object. For clients, this represents the remote host's
		 * 					hostname, and for servers, defaults to 'localhost'.
		 * 	@param	port	The port for this UDP object, which for clients, represents the port of the remote
		 * 					host to which to connect, and for servers, represents the port to which to bind and listen
		 * 					for incoming connections.
		 *	@param	io_service	The I/O service used for asynchronous I/O requests
		 */
		Udp(string host, int port, boost::asio::io_service & io_service);

		/**
		 * Immediately calls the <code>close</code> function, freeing all resources for this UDP object and shutting down
		 * any open connections.
		 */
		virtual ~Udp() {}

		friend class UdpEvent;

	protected:

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
		template<class T> inline string option_to_string(optional<T> &arg);

		/**
		 * Helper for parsing arguments
		 *
		 * @param   options     A map of strings to string representing the options passed in and their values
		 * @param   arg         The specific option to check for
		 * @param   arg_value   This will hold the value of the argument, if found.
		 */
		void inline parse_string_bool_arg(map<string, string> &options, string arg, optional<bool> &arg_value);

		/**
		 * Parses any options (like whether or use IPv6 or IPv4) from the options. Supported options currently include:
		 *
		 * ipv6                 if true, use ipv6. otherwise use ipv4.
		 * multicast            enable udp multicasting
		 * multicast group      the multicast group to join
		 * multicast ttl        number of multicast hops to make;
		 * do not route         prevent routing, use local interfaces
		 * reuse address        allow the socket to bind to an address already in use
		 * keep alive           allow the socket to send keep-alives.
		 *
		 * @param options       A map of options to values.
		 */
		void parse_args(map<string, string> options);

		/**
		 * Write to the log the options used to create this connection.
		 */
		void log_options();

		/**
		 * Helper function to listen for incoming data.
		 */
		virtual void listen() = 0;

		/**
		 * Immediately frees all resources for this UDP object and shutting down any open connections. This function
		 * 	should never be invoked by the base class, and logs an error if this ever occurs.
		 */
		virtual void close() = 0;

		/**
		 * Handler invoked when the data has been sent (or sending terminated in error).
		 *
		 * 	@param	err		The error code encountered when trying to send data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	bytes_transferred	The number of bytes successfully sent over the connection.
		 * 	@param	msg		The data to be sent.
		 * 	@param	host	The hostname for this UDP object
		 * 	@param	port	The port for this UDP object
		 */
		void send_handler(const boost::system::error_code &err, std::size_t bytes_transferred,
				string msg, string host, int port);

		/**
		 * Handler invoked when some data has been received.
		 *
		 * 	@param	err			The error code encountered when trying to receive data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	bytes_transferred	The number of bytes successfully received over the connection.
		 * 	@param	socket		The connection on which attempted to receive data.
		 * 	@param	endpoint	The connected endpoint to the remote host
		 * 	@param	host	The hostname for this UDP object
		 * 	@param	port	The port for this UDP object
		 */
		void receive_handler(const boost::system::error_code &err, std::size_t bytes_transferred,
						boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint, string host,
						int port);

		/**
		 * Helper to fire an error event to javascript.
		 *
		 * 	@param	message	The error message
		 */
		virtual void fire_error_event(const string & message) = 0;

		/**
		 * Helper to fire data event to javascript.
		 *
		 * 	@param	data	The data received
		 * 	@param	socket	The socket on which to reply to this data
		 * 	@param	endpoint The connected endpoint to the remote host
		 */
		virtual void fire_data_event(const string data, boost::shared_ptr<udp::socket> socket,
				boost::shared_ptr<udp::endpoint> endpoint) = 0;

		/** The I/O service for perform nonblocking actions */
		boost::asio::io_service & io_service;

		/** A constant representing the size of the buffer in which to receive data. This is adjust to support
		 the largest packet possible according to the UDP protocol. */
		static const int BUFFER_SIZE = 2048;

		/** A buffer for receiving data from the remote host. */
		boost::array<char, BUFFER_SIZE> receive_buffer;

		/** The number of asynchronous I/O requests that are pending completion */
		int pending_sends;

		/** Mutex for accessing the number of pending sends */
		boost::mutex pending_sends_mutex;

		/** A flag indicating whether or not this object has been requested to shutdown. */
		bool should_close;

		/** Flag for using IPv6, if true, this UDP object uses IPv6, otherwise, it is using IPv4. */
		optional<bool> using_ipv6;

		/** Flag representing whether or not multicast is enabled for this UDP object. */
		optional<bool> multicast;

		/** If multicast is enabled, use this for the group to join */
		optional<string> multicast_group;

		/** If multicast is enabled, this is the TTL or hops */
		optional<int> multicast_ttl;

		/** Flag to prevent routing, use local interfaces only */
		optional<bool> do_not_route;

		/** Flag to allow the socket to be bound to an address that is already in use. */
		optional<bool> reuse_address;

		/** The hostname for this UDP object ('SERVER' for servers, or the hostname of the remote host for clients) */
		string host;

		/** The port (the port bound to by servers, port on which to connect to the remote host for clients) */
		int port;

		/** A connected endpoint to the remote host. */
		boost::shared_ptr<udp::endpoint> remote_endpoint;

		/** A flag to say if this UDP object has permanently failed and cannot continue for some reason or another */
		bool failed;
};

#endif /* UDP_H_ */
