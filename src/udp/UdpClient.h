#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <algorithm>
#include <map>
#include <string>
#include <queue>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "Client.h"
#include "Logger.h"
#include "Event.h"
#include "Udp.h"

using boost::asio::ip::udp;
using std::vector;
using std::string;

class UdpClient: public Udp, public Client
{
	public:

		/**
		 * Asynchronously resolves the host and port number after creation. The client will not be able to send
		 * messages until after the resolve handler has run to completion.
		 *
		 * @param host          The host to connect to
		 * @param port          The port the host is listening on
		 * @param io_service	The I/O service to be used for asynchronous I/O requests
		 */
		UdpClient(const string &host, int port, boost::asio::io_service & io_service);

		/**
		 * Asynchronously resolves the host and port number after creation. The client will not be able to send
		 * messages until after the resolve handler has run to completion.
		 *
		 * @param host          The host to connect to
		 * @param port          The port the host is listening on
		 * @param io_service	The I/O service to be used for asynchronous I/O requests
		 * @param options		A map of additional options to configure this UDP client
		 */
		UdpClient(const string &host, int port, boost::asio::io_service & io_service, map<string, string> options);

		/**
		 * Deconstructs a UDP client, immediately calling <code>close</code> to shutdown this client's socket and stop
		 * 	listening for responses.
		 */
		virtual ~UdpClient();

		/**
		 * Asynchronously sends data to the remote host to which this client is connected.
		 *
		 * 	@param	data	The data to send across the wire
		 */
		virtual void send(const string & data);

		/**
		 * Asynchronously sends bytes to the remote host to which this client is connected.
		 *
		 * 	@param	bytes	The data to send across the wire
		 */
		virtual void send_bytes(const vector<byte> & bytes);

		/**
		 * Gracefully shutdown this UDP client, waiting until all sends have completed before freeing all resources for
		 * 	this UDP client and shutting down any open connections. This function is exposed the javascript API.
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

	protected:

		/**
		 * Initializes the socket according to the passed in options (if any).
		 */
		void init_socket();

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
		virtual void fire_data_event(const string data, boost::shared_ptr<udp::socket> socket,
				boost::shared_ptr<udp::endpoint> endpoint);

		/**
		 * Immediately cancels any pending operations and closes this client's socket.
		 */
		virtual void close();

	private:

		/**
		 * Disallows copying a UDP client
		 */
		UdpClient(const UdpClient &other);

		/**
		 * Helper function that will listen for incoming data on the UDP connection for the client, specifically responses
		 * 	to data already sent.
		 */
		virtual void listen();

		/**
		 * I/O handler invoked when the remote host is resolved. This handler attempts to asynchronously establish
		 * 	a UDP connection with the remote host.
		 *
		 * 	@param	err			The error code encountered when trying to receive data, if any occurred. On success,
		 * 						this value is zero, and nonzero on error.
		 * 	@param	endpoint_iterator	Allows the client to iterate through resolvers to retry the connection if it
		 * 								initially fails to connect.
		 */
		void resolve_handler(const boost::system::error_code &err, udp::resolver::iterator endpoint_iterator);

        /**
         * Flush all pending messages.
         */
        void flush();

		/**
		 * Resolver object provided by <code>boost</code> to resolve the remote hostname and port
		 */
		boost::shared_ptr<udp::resolver> resolver;

		/**
		 * A shared reference to the socket used to connect to the remote host
		 */
		boost::shared_ptr<udp::socket> socket;

        /**
         * Mutex for the message queue.
         */
        boost::mutex queue_mtx;

        /**
         * Messages waiting to be sent, since the host hasn't been resolved yet.
         */
        std::queue<string> msgs_not_sent;

		/**
		 * A flag representing whether the remote host for this UDP client has already been resolved.
		 */
		bool resolved_endpoint;
};

#endif
