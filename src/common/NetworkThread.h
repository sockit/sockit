/*
 * NetworkThread.h
 *
 *  Created on: Jun 3, 2011
 *      Author: jtedesco
 */

#ifndef NETWORKTHREAD_H_
#define NETWORKTHREAD_H_

#include <boost/weak_ptr.hpp>
#include <boost/lexical_cast.hpp>

#include <map>
#include <set>
#include <string>

#include "JSAPIAuto.h"

#include "TcpClient.h"
#include "TcpEvent.h"
#include "TcpServer.h"
#include "UdpClient.h"
#include "UdpEvent.h"
#include "UdpServer.h"

using std::map;
using std::string;

/**
 * Class representing a high-level thread of computation for the plugin. This class a single background thread
 * associated with it to perform asynchronous I/O for all clients and servers created from this (high-level) thread.
 *
 * <p>This class also exposes four methods to Javascript to allow creating servers and clients whose I/O will be performed
 * on this <code>NetworkThread</code>.
 */
class NetworkThread: public FB::JSAPIAuto
{
	public:

		/**
		 * Creates a new network thread, registers its functions to the Javascript, and starts up the background thread.
		 */
		NetworkThread();

		/**
		 * Destroys this network thread by stopping the background I/O service and joining on the background thread.
		 */
		virtual ~NetworkThread();

		/**
		 * Creates a new TCP server on this <code>NetworkThread</code>.
		 *
		 * 	@param	port		The port on which this new TCP server should listen
         * 	@param  options     A map of options to specify the behavior of this object.
		 * 	@return	A shared pointer to a newly created TCP server
		 */
		boost::shared_ptr<TcpServer> create_tcp_server(int port, boost::optional<map<string, string> > options);

		/**
		 * Creates a new TCP client on this <code>NetworkThread</code>.
		 *
		 * 	@param	host	The hostname to which this TCP client will connect
		 * 	@param	port	The port on the remote host to which this client should connect
         * 	@param  options A map of options to specify the behavior of this object.
		 * 	@return	A shared pointer to a newly created TCP client
		 */
		boost::shared_ptr<TcpClient> create_tcp_client(const string & host, int port,
                boost::optional<map<string, string> > options);

		/**
		 * Creates a new UDP server on this <code>NetworkThread</code>.
		 *
		 * 	@param	port			The port on which this new UDP server should listen
         * 	@param  options         A map of options to specify the behavior of this object.
		 * 	@return	A shared pointer to a newly created UDP server
		 */
		boost::shared_ptr<UdpServer> create_udp_server(int port, boost::optional<map<string, string> > options);

		/**
		 * Creates a new UDP client on this <code>NetworkThread</code>.
		 *
		 * 	@param	host	The hostname to which this UDP client will connect
		 * 	@param	port	The port on the remote host to which this client should connect
         * 	@param  options A map of options to specify the behavior of this object.
		 * 	@return	A shared pointer to a newly created UDP client
		 */
		boost::shared_ptr<UdpClient> create_udp_client(const string &host, int port,
                boost::optional<map<string, string> > options);

	private:

		/**
		 * The category string for a network thread
		 */
		string logger_category;

		/**
		 * A helper method in which the background thread will run. This method simply starts the I/O service, and
		 * returns once the I/O service is stopped.
		 */
		void run();

		/**
		 * The <code>boost</code> I/O service to be shared between all clients and servers created on this <code>NetworkThread</code>,
		 * which will be used to perform asynchronous I/O.
		 */
		boost::asio::io_service io_service;

		/**
		 * The background thread which launches the I/O service, and exits when the I/O service is stopped.
		 */
		boost::thread background_thread;
		
		/** Set of all udp clients 'on' this thread */
		set<boost::shared_ptr<UdpClient> > udp_clients;
		
		/** Set of all udp servers 'on' this thread */
		set<boost::shared_ptr<UdpServer> > udp_servers;
		
		/** Set of all tcp clients 'on' this thread */
		set<boost::shared_ptr<TcpClient> > tcp_clients;
		
		/** Set of all tcp servers 'on' this thread */
		set<boost::shared_ptr<TcpServer> > tcp_servers;
		
};

#endif /* NETWORKTHREAD_H_ */
