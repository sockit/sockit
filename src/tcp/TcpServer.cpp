    /*
 * File:   TcpServer.cpp
 * Author: jtedesco
 *
 * Created on May 26, 2011, 12:33 PM
 */

#include "TcpServer.h"

TcpServer::TcpServer(int port, boost::asio::io_service & io_service) :
	Tcp("SERVER", port, io_service)
{
	init();
}

TcpServer::TcpServer(int port, boost::asio::io_service & io_service, map<string, string> options) :
	Tcp("SERVER", port, io_service)
{
	parse_args(options);
	init();
}

TcpServer::~TcpServer()
{
	close();
}

void TcpServer::close()
{
	// Shutdown the IO service, cancel any transfers on the socket, and close the socket
	waiting_to_shutdown = true;

    set<boost::shared_ptr<tcp::socket> >::iterator it;
    for(it = connections.begin(); it != connections.end(); it++)
    {
        try
        {
            if((*it) && (*it).get() && (*it)->is_open())
            {
                (*it)->close();
            }
        }
        catch(boost::system::error_code &e)
        {
            Logger::warn("Error in TcpServer deconstructor: " + e.message(), port, host);
        }
        catch(std::exception &er)
        {
            Logger::warn("Error in TcpServer deconstructor: " + std::string(er.what()), port, host);
        }
        catch(...)
        {
            Logger::warn("Error occured that could not be caught");
        }
    }

	connections.clear();

	if (acceptor && acceptor->is_open())
	{
		acceptor->close();
	}
	else
	{
		// Don't fire an error, otherwise the plugin will crash
		string message("Could not cleanly shut down acceptor in TCP server, desctructing anyways");
		Logger::warn(message, port, host);
	}
}

void TcpServer::init()
{
	// Bind the acceptor to the correct port & set the options on this acceptor
	try
	{
		if (using_ipv6 && *using_ipv6)
		{
			acceptor = boost::shared_ptr<tcp::acceptor>(new tcp::acceptor(io_service, tcp::endpoint(tcp::v6(), port)));
		}
		else
		{
			acceptor = boost::shared_ptr<tcp::acceptor>(new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), port)));
		}
	}
	catch (boost::system::system_error &e)
	{
		// Catch this error, and fail gracefully
		string message(string("Caught error initializing TCP server: '") + e.what() + "'");
		Logger::error(message, port, host);

		// Stop this server from ever doing anything again
		failed = true;
	}

	// Check that acceptor was created successfully
	if (!acceptor.get())
	{
		// Fail gracefully and stop this server from ever doing anything again
		string message("Failed to initialized TCP server acceptor");
		Logger::error(message, port, host);
		failed = true;
	}
}

void TcpServer::init_socket(boost::shared_ptr<tcp::socket> connection)
{
	// Set the socket options for this client's TCP socket
	if (do_not_route)
	{
		boost::asio::socket_base::do_not_route option(*do_not_route);
		connection->set_option(option);
	}

	// Toggle keep alive (enabled/disabled)
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

/**
 * Helper function to shutdown and deallocate the memory for a socket.
 *
 * 	@param   socket   The socket to deallocate.
 */
void socket_deallocate(tcp::socket * socket)
{
	// may already have been deallocated
	if(!socket) return;

    try
    {
        if(socket && socket->is_open())
        {
            //s->shutdown(s->shutdown_both);
            socket->close();
        }
    }
    catch(boost::system::error_code &e)
    {
        Logger::error("socket deallocate: " + e.message());
    }
    catch(std::exception &er)
    {
        Logger::error("socket deallocate: " + std::string(er.what()));
    }

    delete socket;
    socket = 0;
}

void TcpServer::start_listening()
{
	if(failed)
	{
		// Log & fire an error
		string message("Trying to start the server listening, but the server has permanently failed!");
		Logger::error(message, port, host);
    }

	// Log listening
	Logger::info("TCP server about to start listening for incoming connections on port "
            + boost::lexical_cast<string>(port), port, host);

	// Prepare to accept a new connection and asynchronously accept new incoming connections
    boost::shared_ptr < tcp::socket > connection(new tcp::socket(io_service), socket_deallocate);
    connections.insert(connection);

	// Try to accept any new connection
	if(acceptor.get())
	{
		acceptor->async_accept(*connection, boost::bind(&TcpServer::accept_handler, this, _1, connection, host, port));
	}
	else
	{
		// Fail gracefully and stop this server from ever doing anything again
		string message("TCP server failed to accept, acceptor invalid");
		Logger::error(message, port, host);
		failed = true;
		return;
	}

	// Callback acknowledging that the server has opened
	fire_open();
}

void TcpServer::shutdown()
{
	if(!failed)
	{
		waiting_to_shutdown = true;
		if (active_jobs == 0)
		{
			fire_close();
			close();
		}
	}
	else
	{
		// Log & fire an error
		string message("Trying to shutdown the TCP server, but the server has permanently failed!");
		Logger::error(message, port, host);
	}
}

void TcpServer::accept_handler(const boost::system::error_code & error_code, boost::shared_ptr<tcp::socket> connection, string host, int port)
{
	// Log error & return if there is an error
	if (error_code)
	{
		// Check for disconnection errors
		std::set<boost::system::error_code>::iterator find_result = disconnect_errors.find(error_code);
		if (find_result != disconnect_errors.end())
		{
			if (error_code == boost::asio::error::operation_aborted)
			{
				// If we're waiting to shutdown, this is part of the normal process
				if(waiting_to_shutdown)
				{
					Logger::info("TCP server stopped listening for new connections", port, host);
				}
				else
				{
					Logger::warn("TCP accept was aborted", port, host);
				}
			}
			else
			{
				string message("TCP accept failed, disconnected: '" + error_code.message() + "'");
				Logger::warn(message, port, host);
				fire_disconnect(message);
			}
			return;
		}

		string message("Error accepting incoming connection: '" + error_code.message() + "'");
		Logger::error(message, port, host);
		fire_error(message);
		return;
	}

	// Initialize the socket options before we start using it
	init_socket(connection);

	// Log that we've successfully accepted a new connection, and fire the 'onconnect' event
	string message("TCP server accepted new connection from " + connection->remote_endpoint().address().to_string() + " port "
			+ boost::lexical_cast<string>(connection->remote_endpoint().port()));
	Logger::info(message, port, host);
	fire_connect();

	if (connection.get())
		connection->async_receive(boost::asio::buffer(receive_buffer),
				boost::bind(&TcpServer::receive_handler, this, _1, _2, connection, host, port));

	// Start listening for new connections, if we're not waiting to close
	if (!waiting_to_shutdown)
	{
		start_listening();
	}
}


void TcpServer::receive_handler(const boost::system::error_code & error_code, std::size_t bytesTransferred,
		boost::shared_ptr<tcp::socket> connection, string host, int port)
{
    Tcp::receive_handler(error_code, bytesTransferred, connection, host, port);

    if(connection && connection.get() && !connection->is_open())
        connections.erase(connection);
}

int TcpServer::get_port()
{
	return port;
}

void TcpServer::fire_error_event(const string & message)
{
	fire_error(message);
}

void TcpServer::fire_disconnect_event(const string & message)
{
	fire_disconnect(message);
}

void TcpServer::fire_data_event(const string data, boost::shared_ptr<tcp::socket> connection)
{
	fire_data(boost::make_shared<TcpEvent>(this, connection, data));
}
