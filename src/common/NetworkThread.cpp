/*
 * NetworkThread.cpp
 *
 *  Created on: Jun 3, 2011
 *      Author: jtedesco
 */

#include "NetworkThread.h"

NetworkThread::NetworkThread() :
	logger_category("NETWORK THREAD")
{
	// No initialization required for the logger
	Logger::info("Network thread initialized", Logger::NO_PORT, logger_category);

	// Register root methods for creating servers & clients
	registerMethod("createUdpClient", make_method(this, &NetworkThread::create_udp_client));
	registerMethod("createUdpServer", make_method(this, &NetworkThread::create_udp_server));
	registerMethod("createTcpClient", make_method(this, &NetworkThread::create_tcp_client));
	registerMethod("createTcpServer", make_method(this, &NetworkThread::create_tcp_server));

	// Start the I/O service running in the background
	background_thread = boost::thread(boost::bind(&NetworkThread::run, this));
}

NetworkThread::~NetworkThread()
{
	// Stop the IO service, and wait for the background thread to exit, log (but don't do anything) if there's an error
	try
	{
		io_service.stop();
		background_thread.join();
	}
	catch (std::exception & error)
	{
		Logger::warn("Warning, improperly shutdown network thread IO service: '" + string(error.what()) + "'",
				Logger::NO_PORT, logger_category);
	}
	
	tcp_clients.clear();
	tcp_servers.clear();
	udp_clients.clear();
	udp_servers.clear();
}

boost::shared_ptr<TcpServer> NetworkThread::create_tcp_server(int port, boost::optional<map<string, string> > options)
{
	Logger::info("Spawning TCP server on port = " + boost::lexical_cast<string>(port), Logger::NO_PORT,
			logger_category);

	if (options)
	{
		boost::shared_ptr<TcpServer> new_server(new TcpServer(port, io_service, *options));
		tcp_servers.insert(new_server);
		return new_server;
	}

	boost::shared_ptr<TcpServer> new_server(new TcpServer(port, io_service));
	tcp_servers.insert(new_server);
	return new_server;
}

boost::shared_ptr<TcpClient> NetworkThread::create_tcp_client(const string & host, int port,
		boost::optional<map<string, string> > options)
{
	Logger::info(
			"Spawning TCP client to '" + boost::lexical_cast<string>(host) + ":" + boost::lexical_cast<string>(port)
					+ "'", Logger::NO_PORT, logger_category);

	if (options)
	{
		boost::shared_ptr<TcpClient> new_client(new TcpClient(host, port, io_service, *options));
		tcp_clients.insert(new_client);
		return new_client;
	}

	boost::shared_ptr<TcpClient> new_client(new TcpClient(host, port, io_service));
	tcp_clients.insert(new_client);
	return new_client;
}

boost::shared_ptr<UdpServer> NetworkThread::create_udp_server(int port, boost::optional<map<string, string> > options)
{
	Logger::info("Spawning UDP server on port = " + boost::lexical_cast<string>(port), Logger::NO_PORT,
			logger_category);

	if (options)
	{
		boost::shared_ptr<UdpServer> new_server(new UdpServer(port, io_service, *options));
		udp_servers.insert(new_server);
		return new_server;
	}

	boost::shared_ptr<UdpServer> new_server(new UdpServer(port, io_service));
	udp_servers.insert(new_server);
	return new_server;
}

boost::shared_ptr<UdpClient> NetworkThread::create_udp_client(const string & host, int port,
		boost::optional<map<string, string> > options)
{
	Logger::info(
			"Spawning TCP client to '" + boost::lexical_cast<string>(host) + ":" + boost::lexical_cast<
					string>(port) + "'", Logger::NO_PORT, logger_category);

	if (options)
	{
		boost::shared_ptr<UdpClient> new_client(new UdpClient(host, port, io_service, *options));
		udp_clients.insert(new_client);
		return new_client;
	}

	boost::shared_ptr<UdpClient> new_client(new UdpClient(host, port, io_service));
	udp_clients.insert(new_client);
	return new_client;
}

void NetworkThread::run()
{
	boost::asio::io_service::work work(io_service);

	try
	{
		io_service.run();
	}
	catch (std::exception & error)
	{
		// Catch any exception and dump it to the log
		Logger::error("Error running network thread: '" + string(error.what()) + "'", Logger::NO_PORT, logger_category);
	}
}
