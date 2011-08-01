/*
 * File:   TcpClient.cpp
 * Author: jtedesco
 *
 * Created on May 26, 2011, 12:32 PM
 */

#include "TcpClient.h"

TcpClient::TcpClient(const string & host, int port, boost::asio::io_service & io_service) :
	Tcp(host, port, io_service), resolver(new tcp::resolver(io_service)), connection(new tcp::socket(io_service))
{
	// Check that the connection and resolver are valid, and fail gracefully if they are not
	if (!resolver.get() || !connection.get())
	{
		failed = true;
		string message("Failed to initialize TCP client, failed to initialize properly");
		Logger::error(message, port, host);
		fire_error(message);
		return;
	}

	init();
}

TcpClient::TcpClient(const string & host, int port, boost::asio::io_service & io_service, map<string, string> options) :
	Tcp(host, port, io_service), resolver(new tcp::resolver(io_service)), connection(new tcp::socket(io_service))
{
	// Check that the connection and resolver are valid, and fail gracefully if they are not
	if (!resolver.get() || !connection.get())
	{
		failed = true;
		string message("Failed to initialize TCP client, failed to initialize properly");
		Logger::error(message, port, host);
		fire_error(message);
		return;
	}

	parse_args(options);

	init();
}

void TcpClient::init()
{
	connected = false;

	Logger::info(
			"Initializing TCP client to host '" + boost::lexical_cast<string>(host) + "' on port " + boost::lexical_cast<string>(port),
			port, host);
	log_options();
	Logger::info(
			"Trying to resolve DNS information for host " + boost::lexical_cast<string>(host) + "', port " + boost::lexical_cast<string>(
					port), port, host);

	// Create a query to resolve this host & port
	if (using_ipv6 && *using_ipv6)
	{
		// Asynchronously resolve the remote host, and once the host is resolved, create a connection
		tcp::resolver::query query(tcp::v6(), host, boost::lexical_cast<string>(port),
				boost::asio::ip::resolver_query_base::numeric_service);
		if (resolver.get())
			resolver->async_resolve(query, boost::bind(&TcpClient::resolve_handler, this, _1, _2));
		else
		{
			failed = true;
			string message("TCP client failed to resolve, invalid resolver");
			Logger::error(message, port, host);
			fire_error(message);
		}
	}
	else
	{
		// Asynchronously resolve the remote host, and once the host is resolved, create a connection
		tcp::resolver::query query(tcp::v4(), host, boost::lexical_cast<string>(port),
				boost::asio::ip::resolver_query_base::numeric_service);
		if (resolver.get())
			resolver->async_resolve(query, boost::bind(&TcpClient::resolve_handler, this, _1, _2));
		else
		{
			failed = true;
			string message("TCP client failed to resolve, invalid resolver");
			Logger::error(message, port, host);
			fire_error(message);
		}
	}
}

void TcpClient::init_socket()
{
	// Set the socket options for this client's TCP socket
	if (do_not_route)
	{
		boost::asio::socket_base::do_not_route option(*do_not_route);
		connection->set_option(option);
	}
	if (keep_alive)
	{
		boost::asio::socket_base::keep_alive option(*keep_alive);
		connection->set_option(option);
	}
	if (no_delay)
	{
		boost::asio::ip::tcp::no_delay option(*no_delay);
		connection->set_option(option);
	}
	if (keep_alive_timeout)
	{
		// Set the TCP keep-alive timeout - ignores return value
		set_tcp_keepalive(connection);
	}
}

TcpClient::~TcpClient()
{
	close();
}

void TcpClient::close()
{
	waiting_to_shutdown = true;
	resolver->cancel();

	// Shutdown the IO service, cancel any transfers on the socket, and close the socket
	if (connection->is_open())
	{
		try
		{
			connection->shutdown(connection->shutdown_both);
		}
		catch (...)
		{
			Logger::warn("Socket shutdown improperly, proceeding anyway", port, host);
		}

		connection->close();
	}
	else
	{
		Logger::warn("Failed to cleanly shutdown TCP client connection, continuing anyways", port, host);
	}
}

void TcpClient::shutdown()
{
	if (!failed)
	{
		active_jobs_mutex.lock();
		int current_jobs = active_jobs;
		active_jobs_mutex.unlock();
		waiting_to_shutdown = true;
		if (current_jobs == 0)
		{
			fire_close();
			close();
		}
	}
	else
	{
		// Log & fire an error
		Logger::error("Trying to start the server listening, but the server has permanently failed!", port, host);
	}
}

void TcpClient::send_bytes(const vector<byte> & bytes)
{
	string data;

	for (int i = 0; i < bytes.size(); i++)
	{
		data.push_back((unsigned char) bytes[i]);
	}

	send(data);
}

void TcpClient::send(const string & data)
{
	if (failed)
	{
		// Log & fire an error
		string message("Trying to send data on a TCP client that has permanently failed!");
		Logger::error(message, port, host);
		return;
	}

	// If we're not already connected, then queue this data to be send
	connected_mutex.lock();
	bool connected_now = connected;
	connected_mutex.unlock();

	if (!connected_now)
	{
		active_jobs_mutex.lock();
		active_jobs++;
		active_jobs_mutex.unlock();
		data_queue_mutex.lock();
		data_queue.push(data);
		data_queue_mutex.unlock();
	}
	else
	{
		// Check if the queue needs to be flushed, and if so, flush it
		flush();

		// Send the data, and record that we've started a new send job
		active_jobs_mutex.lock();
		active_jobs++;
		active_jobs_mutex.unlock();
		connection->async_send(boost::asio::buffer(data.data(), data.size()),
				boost::bind(&TcpClient::send_handler, this, _1, _2, data, host, port, connection));
	}
}

void TcpClient::resolve_handler(const boost::system::error_code & error_code, tcp::resolver::iterator endpoint_iterator)
{
	// If we encountered an error
	if (error_code)
	{
		// Check for disconnection errors
		std::set<boost::system::error_code>::iterator find_result = disconnect_errors.find(error_code);
		if (find_result != disconnect_errors.end())
		{
			if (error_code == boost::asio::error::operation_aborted)
			{
				Logger::warn("TCP resolve was aborted");
				return;
			}
			else
			{
				string message("TCP resolve failed, disconnected: '" + error_code.message() + "'");
				Logger::warn(message, port, host);
				fire_disconnect(message);
				return;
			}
		}

		// We failed permanently, fail permanently and log it
		string message("Failed to resolve host with error: " + error_code.message());
		Logger::error(message, port, host);
		fire_error(message);
		return;
	}

	if (connection.get())
	{
		// Attempt to connect to the endpoint, using IPv6 if specified
		tcp::endpoint receiver_endpoint = *endpoint_iterator;
		if (using_ipv6 && *using_ipv6)
		{
			connection->open(tcp::v6());
		}
		else
		{
			connection->open(tcp::v4());
		}

		// Initialize the socket if it's open
		if (!connection->is_open())
			init_socket();

		// Log success
		Logger::info("Host has been resolved, attempting to connect to host", port, host);
		fire_resolve();

		// Try to asynchronously establish a connection to the host
		connection->async_connect(receiver_endpoint, boost::bind(&TcpClient::connect_handler, this, _1, endpoint_iterator));
	}
	else
	{
		failed = true;
		string message("TCP client failed to resolve, invalid connection");
		Logger::error(message, port, host);
		fire_error(message);
	}
}

void TcpClient::connect_handler(const boost::system::error_code & error_code, tcp::resolver::iterator endpoint_iterator)
{
	// If there was an error to connect, log the error and abort connection
	if (error_code)
	{
		// Check for disconnection errors
		std::set<boost::system::error_code>::iterator find_result = disconnect_errors.find(error_code);
		if (find_result != disconnect_errors.end())
		{
			if (error_code == boost::asio::error::operation_aborted)
			{
				Logger::warn("TCP connect was aborted");
				return;
			}
			else
			{
				string message("TCP connect failed, disconnected: '" + error_code.message() + "'");
				Logger::warn(message, port, host);
				fire_disconnect(message);
				return;
			}
		}

		// Check if this was not the last possible connection in the iterator
		tcp::resolver::iterator end;
		if (endpoint_iterator != end && error_code == boost::asio::error::host_not_found)
		{
			// Create a query to resolve this host & port
			tcp::resolver::query query(host, boost::lexical_cast<string>(port));

			// Try the next possible endpoint
			resolver->async_resolve(query, boost::bind(&TcpClient::resolve_handler, this, _1, endpoint_iterator++));
		}
		else
		{
			// If we've tried every possible endpoint, fail permanently
			string message("Failed to connect to host, with message: '" + error_code.message() + "'");
			Logger::error(message, port, host);
			fire_error(message);
			return;
		}
	}

	// Log success, and record that we are now connected
	Logger::info("Connection established to host", port, host);
	fire_connect();

	// Start receiving data on this connection
	connection->async_receive(boost::asio::buffer(receive_buffer),
			boost::bind(&TcpClient::receive_handler, this, _1, _2, connection, host, port));

	// Flush data from the queue if necessary
	flush();

	connected_mutex.lock();

	connected = true;
	connected_mutex.unlock();
}

void TcpClient::flush()
{
	data_queue_mutex.lock();
	while (!data_queue.empty())
	{
		data_queue_mutex.unlock();

		// Pull the next data chunk to be sent off the queue
		data_queue_mutex.lock();
		string data = data_queue.front();
		data_queue.pop();
		data_queue_mutex.unlock();

		// Asynchronously send the data across the connection
		connection->async_send(boost::asio::buffer(data.data(), data.size()),
				boost::bind(&TcpClient::send_handler, this, _1, _2, data, host, port, connection));
	}
	data_queue_mutex.unlock();
}

int TcpClient::get_port()
{
	return port;
}

string TcpClient::get_host()
{
	return host;
}

void TcpClient::fire_error_event(const string & message)
{
	fire_error(message);
}

void TcpClient::fire_disconnect_event(const string & message)
{
	fire_disconnect(message);
}

void TcpClient::fire_data_event(const string data, boost::shared_ptr<tcp::socket> connection)
{
	fire_data(boost::make_shared<TcpEvent>(this, connection, data));
}

