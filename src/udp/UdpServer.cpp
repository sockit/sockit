/* Udpserver.cpp
 *
 * Listens to connections over UDP. This class is directly exposed to the Javascript. Will not listen to the port
 * specified until the listen() method is invoked. This is so the handlers can be attached beforehand.
 *
 */

#include "UdpServer.h"


UdpServer::UdpServer(int port, boost::asio::io_service & io_service) 
    : Udp("SERVER", port, io_service), socket(new udp::socket(io_service))
{
	initialize();
}


UdpServer::UdpServer(int port, boost::asio::io_service & io_service, map<string, string> options) 
    : Udp("SERVER", port, io_service), socket(new udp::socket(io_service))
{
    parse_args(options);

	initialize();
}


void UdpServer::shutdown()
{
	if(!failed)
	{
		should_close = true;

		if (pending_sends == 0) {
			fire_close();
			close();
		}
	}
	else
	{
		// Log & fire an error
		string message("Trying to shutdown a permanently failed UDP server!");
		Logger::error(message, port, host);
	}

}

void UdpServer::close()
{
	/*
	if (socket->is_open())
	{
		if(multicast && multicast_group)
		{
			socket->set_option(
					boost::asio::ip::multicast::leave_group(
						boost::asio::ip::address::from_string(*multicast_group)));
		}

		socket->close();
	}
	*/
	if(socket->is_open())
		socket->close();
}


void UdpServer::initialize()
{
    log_options();

    listening = false;

    if(using_ipv6 && *using_ipv6)
        socket->open(udp::v6());
    else
    	socket->open(udp::v4());

    if(!socket->is_open())
    {
    	string message("UDP server failed to initialize, could not open socket");
    	Logger::error(message, port, host);
    	fire_error(message);
    }

	// synchronize the buffer size of the socket with this class's buffer size
	boost::asio::socket_base::receive_buffer_size buf_size_option(BUFFER_SIZE);
	socket->set_option(buf_size_option);

    if(do_not_route)
    {
        boost::asio::socket_base::do_not_route option(*do_not_route);
		socket->set_option(option);
    }

    if(reuse_address)
    {
        boost::asio::socket_base::reuse_address option(*reuse_address);
        socket->set_option(option);
    }

	if(multicast)
	{
        /* force this to be set */
        boost::asio::socket_base::reuse_address option(true);
        socket->set_option(option);
    }

    if(multicast_ttl)
    {
        boost::asio::ip::multicast::hops option(*multicast_ttl);
        socket->set_option(option);
    }

	// register these methods so they can be invoked from the Javascript
	registerMethod("listen", make_method(this, &UdpServer::start_listening));
}


UdpServer::~UdpServer()
{
	close();
}


void UdpServer::start_listening()
{
	if(failed)
	{
		// Log & fire an error
		string message("Trying to start a UDP server that has permanently failed!");
		Logger::error(message, port, host);
        return;
    }

	if(listening) return;

	try
	{
		if(using_ipv6 && *using_ipv6)
			socket->bind(udp::endpoint(udp::v6(), port));
		else
			socket->bind(udp::endpoint(udp::v4(), port));
	}
	catch (boost::system::system_error &e)
	{
		// Catch this error, and fail gracefully
		string message(string("Caught error initializing UDP server: '") + e.what() + "'");
		Logger::error(message, port, host);

		// Stop this server from ever doing anything again
		failed = true;
		return;
	}

	Logger::info("bind!", port, host);

	if(multicast)
	{
		if(!multicast_group)
        {
			Logger::error("UdpServer: Multicast set, but no multicast group given", port, host);
        }
		else
        {
			// Try to join the multicast group, and fail gracefully if we can't join it
			try
			{
				socket->set_option(boost::asio::ip::multicast::join_group(
							boost::asio::ip::address::from_string(*multicast_group)));
			}
		    catch(boost::system::system_error & error)
		    {
			    // Catch this error, and fail gracefully
			    string message(string("Caught error initializing UDP joinging multicast group: '") + error.what() + "'");
			    Logger::error(message, port, host);

			    failed = true;
			    return;
		    }
        }
	}

	listen();
	fire_open();

	listening = true;
}


void UdpServer::listen()
{
	Logger::info("udpserver: starting to listen", port, host);

    if(remote_endpoint && remote_endpoint.get())
        socket->async_receive_from(boost::asio::buffer(receive_buffer), *remote_endpoint,
                boost::bind(&UdpServer::receive_handler, this, _1, _2, socket, remote_endpoint, host, port));
    else
        Logger::warn("remote endpoint is null", port, host);
}

int UdpServer::get_port()
{
	return port;
}


void UdpServer::fire_error_event(const string & message)
{
	fire_error(message);
}


void UdpServer::fire_data_event(const string data, boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint)
{
	fire_data(boost::make_shared<UdpEvent>(this, socket, endpoint, data));
}
