/*
 * Tcp.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: jtedesco
 */

#include "Tcp.h"

Tcp::Tcp(string host, int port, boost::asio::io_service & ioService) :
	host(host), port(port), waiting_to_shutdown(false), active_jobs(0), io_service(ioService), using_ipv6(false), failed(false)
{
	// Collect the set of errors classified as 'disconnect' type errors
	disconnect_errors.insert(boost::asio::error::connection_reset);
	disconnect_errors.insert(boost::asio::error::eof);
	disconnect_errors.insert(boost::asio::error::connection_aborted);
	disconnect_errors.insert(boost::asio::error::operation_aborted);
}

void Tcp::send_handler(const boost::system::error_code & error_code, std::size_t bytes_transferred, string data, string host, int port,
		boost::shared_ptr<tcp::socket> connection)
{
	// Check the error code
	if (error_code)
	{
		// Check for disconnection errors
		std::set<boost::system::error_code>::iterator find_result = disconnect_errors.find(error_code);
		if (find_result != disconnect_errors.end())
		{
			if (error_code == boost::asio::error::operation_aborted)
			{
				Logger::info("TCP send failed, aborted", port, host);
			}
			else
			{
				string message("TCP send failed, disconnected, error message: '" + error_code.message() + "'");
				Logger::info(message, port, host);
				fire_disconnect_event(message);
			}
			return;
		}

		string message(
				"TCP send failed, error message '" + error_code.message() + "', error code '" + boost::lexical_cast<string>(
						error_code.value()) + "' was encountered");
		Logger::error(message, port, host);
		fire_error_event(message);
		return;
	}

	string message("TCP send succeeded, sent " + boost::lexical_cast<string>(bytes_transferred) + " bytes, data is: " + data);
	Logger::info(message, port, host);

	// Check to see if this is for the last send to complete, and if we're waiting to shutdown
	active_jobs_mutex.lock();
	active_jobs--;
	active_jobs_mutex.unlock();
	int current_jobs = active_jobs;
	if (waiting_to_shutdown && current_jobs == 0)
	{
		close();
	}
}

void Tcp::receive_handler(const boost::system::error_code & error_code, std::size_t bytes_transferred,
		boost::shared_ptr<tcp::socket> connection, string host, int port)
{
	// Check for errors
	if (error_code)
	{
		// Check for disconnection errors
		if (disconnect_errors.find(error_code) != disconnect_errors.end())
		{
			if (error_code == boost::asio::error::operation_aborted)
			{
				Logger::info("TCP send failed, aborted", port, host);
				return;
			}
			else
			{
				string message("TCP receive failed, disconnected, error message: '" + error_code.message() + "'");
				Logger::info(message, port, host);
				fire_disconnect_event(message);
				return;
			}
		}

		// Otherwise, this is a more serious error, fail and log the error
		string message(
				"TCP receive failed, error message: '" + error_code.message() + "', value '" + boost::lexical_cast<string>(
						error_code.value()) + "'");
		Logger::error(message, port, host);
		fire_error_event(message);

		// Shutdown the socket we're receiving from
		if (connection.get() && connection->is_open())
		{
			try
			{
				connection->shutdown(connection->shutdown_receive);
			}
			catch (...)
			{
				Logger::warn("TCP receive failed, shutdown improperly, proceeding anyway", port, host);
			}
		}

		return;
	}

	// Pull out the data we received, and fire a data received event
	string data = string(receive_buffer.c_array(), bytes_transferred);

	// Log success
	string message(
			"Successfully received all " + boost::lexical_cast<string>(bytes_transferred) + " bytes of " + boost::lexical_cast<string>(
					bytes_transferred) + " total bytes. Data: '" + data + "'");

	Logger::info(message, port, host);

	try
	{
		fire_data_event(data, connection);
	}
	catch (const boost::bad_weak_ptr &p)
	{
		Logger::error("Event is going out of scope", port, host);
	}

	// Try to receive more data
	if (connection.get())
		connection->async_receive(boost::asio::buffer(receive_buffer),
				boost::bind(&Tcp::receive_handler, this, _1, _2, connection, host, port));
}

inline string Tcp::bool_option_to_string(optional<bool> &arg, string iftrue, string iffalse)
{
	if (arg && *arg)
		return iftrue;
	return iffalse;
}

template<class T>
inline string Tcp::option_to_string(optional<T> &arg)
{
	if (arg)
		return boost::lexical_cast<string>(*arg);
	return string("unset");
}

void inline Tcp::parse_string_bool_arg(map<string, string> &options, string arg, optional<bool> &arg_value)
{
	map<string, string>::iterator it;

	if ((it = options.find(arg)) != options.end())
	{
		if (it->second == std::string("true"))
			arg_value.reset(true);
		if (it->second == std::string("false"))
			arg_value.reset(false);
	}
}

void inline Tcp::parse_string_int_arg(map<string, string> &options, string arg, optional<int> &arg_value)
{
	map<string, string>::iterator it;

	if ((it = options.find(arg)) != options.end())
	{
		arg_value.reset(boost::lexical_cast<int>(it->second));
	}
}

void Tcp::parse_args(map<string, string> options)
{
	map<string, string>::iterator it;
	map<string, string> transformed_options;

	string t("true");
	string f("false");

	// transform the entire map to lower case
	for (it = options.begin(); it != options.end(); it++)
	{
		string k = it->first;
		string v = it->second;

		std::transform(k.begin(), k.end(), k.begin(), ::tolower);
		std::transform(v.begin(), v.end(), v.begin(), ::tolower);

		k.erase(std::remove_if(k.begin(), k.end(), ::isspace), k.end());
		v.erase(std::remove_if(v.begin(), v.end(), ::isspace), v.end());

		transformed_options.insert(std::pair<string, string>(k, v));
	}

	parse_string_bool_arg(transformed_options, "ipv6", using_ipv6);
	parse_string_bool_arg(transformed_options, "donotroute", do_not_route);
	parse_string_bool_arg(transformed_options, "keepalive", keep_alive);
	parse_string_bool_arg(transformed_options, "nodelay", no_delay);
	parse_string_int_arg(transformed_options, "keepalivetimeout", keep_alive_timeout);

	log_options();
}

void Tcp::log_options()
{
	string options("These arguments were passed in: ");

	options.append(bool_option_to_string(using_ipv6, "ipv6, ", "ipv4, "));
	options.append(bool_option_to_string(do_not_route, "no routing, ", "use routing, "));
	options.append(bool_option_to_string(no_delay, "no delay, ", "allow delay, "));
	options.append(bool_option_to_string(keep_alive, "keep alive", "don't keep alive"));
	options.append(", keep alive timeout is ");
	options.append(option_to_string<int> (keep_alive_timeout));

	Logger::info(options, port, host);
}

bool Tcp::set_tcp_keepalive(boost::shared_ptr<tcp::socket> socket)
{
#ifdef __UNIX__

	// For *n*x systems
	int native_fd = socket->native();
	int timeout = *keep_alive_timeout;
	int intvl = 1;
	int probes = 10;
	int on = 1;

	int ret_keepalive = setsockopt(native_fd, SOL_SOCKET, SO_KEEPALIVE, (void*) &on, sizeof(int));
	int ret_keepidle = setsockopt(native_fd, SOL_TCP, TCP_KEEPIDLE, (void*) &timeout, sizeof(int));
	int ret_keepintvl = setsockopt(native_fd, SOL_TCP, TCP_KEEPINTVL, (void*) &intvl, sizeof(int));
	int ret_keepinit = setsockopt(native_fd, SOL_TCP, TCP_KEEPCNT, (void*) &probes, sizeof(int));

	if(ret_keepalive || ret_keepidle || ret_keepintvl || ret_keepinit)
	{
		string message("Failed to enable keep alive on TCP client socket!");
		Logger::error(message, port, host);
		return false;
	}

#elif defined(__OSX__)

	int native_fd = socket->native();
	int timeout = *keep_alive_timeout;
	int intvl = 1;
	int on = 1;

	// Set the timeout before the first keep alive message
	int ret_sokeepalive = setsockopt(native_fd, SOL_SOCKET, SO_KEEPALIVE, (void*) &on, sizeof(int));
	int ret_tcpkeepalive = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*) &timeout, sizeof(int));
	int ret_tcpkeepintvl = setsockopt(native_fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, (void*) &intvl, sizeof(int));

	if(ret_sokeepalive || ret_tcpkeepalive || ret_tcpkeepintvl)
	{
		string message("Failed to enable keep alive on TCP client socket!");
		Logger::error(message, port, host);
		return false;
	}

#else
	// Partially supported on windows
	struct tcp_keepalive keepalive_options;
	keepalive_options.onoff = 1;
	keepalive_options.keepalivetime = *keep_alive_timeout * 1000;
	keepalive_options.keepaliveinterval = 2000;

	BOOL keepalive_val = true;
	SOCKET native = socket->native();
	DWORD bytes_returned;

	int ret_keepalive = setsockopt(native, SOL_SOCKET, SO_KEEPALIVE, (const char *) &keepalive_val, sizeof(keepalive_val));
	int ret_iotcl = WSAIoctl(native, SIO_KEEPALIVE_VALS, (LPVOID) & keepalive_options, (DWORD) sizeof(keepalive_options), NULL, 0,
			(LPDWORD) & bytes_returned, NULL, NULL);

	if (ret_keepalive || ret_iotcl)
	{
		string message("Failed to set keep alive timeout on TCP client socket!");
		Logger::error(message, port, host);
		return false;
	}
#endif
	return true;
}
