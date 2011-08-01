/* UdpClient.cpp
 *
 * The UDP client can connect to services over udp. This class was intended to be exposed to the javascript.
 * The client currently looks up the host and port everytime a message is sent; the performance hit is currently
 * being measured to see if this information should be cached locally.
 *
 * Javascript API related to the UDP Client:
 *
 * attach/detachListener (implemented in firebreath)
 * send(msg, callback_function)
 * close()
 */

#include "UdpClient.h"

UdpClient::UdpClient(const string &host, int port, boost::asio::io_service & ioService) :
	Udp(host, port, ioService), resolver(new udp::resolver(io_service)), socket(new udp::socket(io_service)), resolved_endpoint(false)
{
	// Check that the connection and resolver are valid, and fail gracefully if they are not
	if (!resolver.get() || !socket.get())
	{
		failed = true;
		string message("Failed to initialize UDP client, failed to initialize properly");
		Logger::error(message, port, host);
		fire_error(message);
		return;
	}
}

UdpClient::UdpClient(const string &host, int port, boost::asio::io_service & ioService, map<string, string> options) :
	Udp(host, port, ioService), resolver(new udp::resolver(io_service)), socket(new udp::socket(io_service)), resolved_endpoint(false)
{
	// Check that the connection and resolver are valid, and fail gracefully if they are not
	if (!resolver.get() || !socket.get())
	{
		failed = true;
		string message("Failed to initialize UDP client, failed to initialize properly");
		Logger::error(message, port, host);
		fire_error(message);
		return;
	}

	parse_args(options);
}

void UdpClient::init_socket()
{
	Logger::info(
			"Initializing UDP client to host '" + boost::lexical_cast<string>(host) + "' on port " + boost::lexical_cast<string>(port),
			port, host);
	log_options();

	if (using_ipv6 && *using_ipv6)
		socket->open(udp::v6());
	else
		socket->open(udp::v4());

	if (!socket->is_open())
	{
		string message("Failed to open UDP client socket");
		Logger::error(message, port, host);
		fire_error(message);
	}

	// synchronize the buffer size of the socket with this class's buffer size
	boost::asio::socket_base::receive_buffer_size buf_size_option(BUFFER_SIZE);
	socket->set_option(buf_size_option);

	// set multicast ttl and out going interface
	if (multicast && *multicast)
	{
		if (multicast_ttl)
		{
			boost::asio::ip::multicast::hops option(*multicast_ttl);
			socket->set_option(option);
		}
	}

	if (do_not_route)
	{
		boost::asio::socket_base::do_not_route option(*do_not_route);
		socket->set_option(option);
	}

	if (reuse_address)
	{
		boost::asio::socket_base::reuse_address option(*reuse_address);
		socket->set_option(option);
	}
}

UdpClient::~UdpClient()
{
	close();
}

void UdpClient::close()
{
	should_close = true;

	if (socket->is_open())
	{
		socket->close();
	}
}

void UdpClient::shutdown()
{
	if (!failed)
	{
		should_close = true;

		pending_sends_mutex.lock();
		int pending_sends_now = pending_sends;
		pending_sends_mutex.unlock();

		if (pending_sends_now == 0)
		{
			fire_close();
			close();
		}
	}
}

void UdpClient::send_bytes(const vector<byte> & bytes)
{
	string data;

	for (int i = 0; i < bytes.size(); i++)
	{
		data.push_back((unsigned char) bytes[i]);
	}

	send(data);
}

void UdpClient::send(const string &msg)
{
	if (failed)
	{
		// Log & fire an error
		string message("Trying to send from a UDP client that has permanently failed!");
		Logger::error(message, port, host);
		return;
	}

	if (should_close)
		return;

	Logger::info("udpclient: sending a msg of size: " + boost::lexical_cast<std::string>(msg.size()) + " which is: " + msg, port, host);

	pending_sends_mutex.lock();
	pending_sends++;
	pending_sends_mutex.unlock();

	if (!resolved_endpoint)
	{
		Logger::info("udpclient: resolving " + host + ":" + boost::lexical_cast<string>(port), port, host);

		queue_mtx.lock();
		msgs_not_sent.push(msg);
		queue_mtx.unlock();

		// Create a query to resolve this host & port
		if (using_ipv6 && *using_ipv6)
		{
			// Asynchronously resolve the remote host, and once the host is resolved, create a connection
			udp::resolver::query query(udp::v6(), host, boost::lexical_cast<string>(port),
					boost::asio::ip::resolver_query_base::numeric_service);
			resolver->async_resolve(query, boost::bind(&UdpClient::resolve_handler, this, _1, _2));
		}
		else
		{
			// Asynchronously resolve the remote host, and once the host is resolved, create a connection
			udp::resolver::query query(udp::v4(), host, boost::lexical_cast<string>(port),
					boost::asio::ip::resolver_query_base::numeric_service);
			resolver->async_resolve(query, boost::bind(&UdpClient::resolve_handler, this, _1, _2));
		}
	}
	else
	{
		flush(); // any pending messages? send them.

		Logger::info("udpclient: attempting to send " + boost::lexical_cast<string>(msg.size()) + " bytes of data: " + msg, port, host);

		// send the message
		if (socket->is_open() && remote_endpoint.get())
		{
			socket->async_send_to(boost::asio::buffer(msg.data(), msg.size()), *remote_endpoint,
					boost::bind(&UdpClient::send_handler, this, _1, _2, msg, host, port));

			Logger::info("udpclient: (async) send called", port, host);

			if (!should_close)
				listen(); // listen for responses
		}
	}
}

void UdpClient::resolve_handler(const boost::system::error_code &err, udp::resolver::iterator endpoint_iterator)
{
	if (err)
	{
		udp::resolver::iterator end;
		if (endpoint_iterator != end && err == boost::asio::error::host_not_found)
		{
			// If we haven't tried resolving using all resolvers, try with another
			udp::resolver::query query(host, boost::lexical_cast<string>(port));
			resolver->async_resolve(query, boost::bind(&UdpClient::resolve_handler, this, _1, endpoint_iterator++));
		}
		else
		{ // We have tried and cannot recover, fail permanently
			string message("Error: resolving host " + host + ":" + boost::lexical_cast<string>(port) + " error was " + err.message());

			Logger::error(message, port, host);
			fire_error(message);

			return;
		}
	}

	fire_resolve();

	Logger::info("udpclient: resolved, going to send", port, host);

	// We succeeded resolving the endpoint, continue
	remote_endpoint = boost::make_shared<udp::endpoint>(*endpoint_iterator);

	if (!socket->is_open())
		init_socket();

	// This endpoint has now been initialized
	resolved_endpoint = true;

	flush();
}

void UdpClient::flush()
{
	queue_mtx.lock();

	while (!msgs_not_sent.empty())
	{
		string msg = msgs_not_sent.front();
		msgs_not_sent.pop();
		queue_mtx.unlock();
		send(msg);
		queue_mtx.lock();
	}

	queue_mtx.unlock();
}

void UdpClient::listen()
{
	if (remote_endpoint && remote_endpoint.get())
		socket->async_receive_from(boost::asio::buffer(receive_buffer), *remote_endpoint,
				boost::bind(&UdpClient::receive_handler, this, _1, _2, socket, remote_endpoint, host, port));
	else
		Logger::warn("remote endpoint is null", port, host);
}

string UdpClient::get_host()
{
	return host;
}

int UdpClient::get_port()
{
	return port;
}

void UdpClient::fire_error_event(const string & message)
{
	if (should_close)
		return;

	fire_error(message);
}

void UdpClient::fire_data_event(const string data, boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint)
{
	if (should_close)
		return;

	fire_data(boost::make_shared<UdpEvent>(this, socket, endpoint, data));
}
