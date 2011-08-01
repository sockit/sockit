/*
 * Udp.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: jtedesco
 */

#include "Udp.h"

Udp::Udp(string host, int port, boost::asio::io_service & io_service) :
	host(host), port(port), pending_sends(0), should_close(false), io_service(io_service), failed(false)
{
	remote_endpoint = boost::shared_ptr<udp::endpoint>(new udp::endpoint());
}

void Udp::send_handler(const boost::system::error_code &error_code, size_t bytes_transferred, string data, string host, int port)
{
	if (error_code)
	{
		if (error_code == boost::asio::error::operation_aborted)
		{
			Logger::info("UDP send failed, aborted", port, host);
		}
		else
		{
			string message(
					"UDP send failed, error message: '" + error_code.message() + "', error code '" + boost::lexical_cast<string>(
							error_code.value()) + "' was encountered");
			Logger::error(message, port, host);
			fire_error_event(message);
		}
		return;
	}

	// Did not successfully send all the data, don't try and resend the rest (this is UDP)
	if (bytes_transferred != data.size())
	{
		string message(
				string("UDP send failed, data was not successfully sent, only ") + boost::lexical_cast<string>(bytes_transferred) + " of "
						+ boost::lexical_cast<string>(message.size()) + " total bytes were sent");
		Logger::error(message, port, host);
		fire_error_event(message);

		return;
	}

	string message("UDP send succeeded, sent " + boost::lexical_cast<string>(bytes_transferred) + " bytes, message is: " + data);
	Logger::info(message, port, host);

	pending_sends_mutex.lock();
	pending_sends--; // just handled sending
	int pending_sends_now = pending_sends;
	pending_sends_mutex.unlock();

	if (pending_sends_now == 0 && should_close)
		close();
}

void Udp::receive_handler(const boost::system::error_code &error_code, std::size_t bytes_transferred,
		boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint, string host, int port)
{
	// Check for errors
	if (error_code)
	{
		if (error_code == boost::asio::error::operation_aborted)
		{
			Logger::info("UDP receive failed, aborted", port, host);
		}
		else
		{
			string message(
					"UDP receive failed, error message: '" + error_code.message() + "', code: '" + boost::lexical_cast<string>(
							error_code.value()) + "'");
			Logger::error(message, port, host);
			fire_error_event(message);
		}

		return;
	}

	// Get the data && fire a data event
	string data(receive_buffer.c_array(), bytes_transferred);
	fire_data_event(data, socket, endpoint);

	// Pull out the data we received, and fire a data received event
	Logger::info("UDP receive succeeded, received " + boost::lexical_cast<string>(bytes_transferred) + " bytes, data is: " + data, port,
			host);

	// Listen for more messages if we're not trying to close down this UDP object
	if (!should_close)
		listen();
}

void inline Udp::parse_string_bool_arg(map<string, string> &options, string arg, optional<bool> &arg_value)
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

void Udp::parse_args(map<string, string> options)
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

	/* Lets just pretend you didn't see any of this */
	parse_string_bool_arg(transformed_options, "ipv6", using_ipv6);
	parse_string_bool_arg(transformed_options, "multicast", multicast);
	parse_string_bool_arg(transformed_options, "donotroute", do_not_route);
	parse_string_bool_arg(transformed_options, "reuseaddress", reuse_address);

	if ((it = transformed_options.find("multicastgroup")) != transformed_options.end())
		multicast_group.reset(it->second);

	if ((it = transformed_options.find("multicastttl")) != transformed_options.end())
		multicast_ttl.reset(boost::lexical_cast<int>(it->second));
}

inline string Udp::bool_option_to_string(optional<bool> &arg, string iftrue, string iffalse)
{
	if (arg && *arg)
		return iftrue;
	return iffalse;
}

template<class T>
inline string Udp::option_to_string(optional<T> &arg)
{
	if (arg)
		return boost::lexical_cast<string>(*arg);
	return string("unset");
}

void Udp::log_options()
{
	string options("These arguments were passed in: ");

	options.append(bool_option_to_string(using_ipv6, "ipv6, ", "ipv4, "));
	options.append(bool_option_to_string(multicast, "multicast enabled, ", "multicast disabled, "));
	options.append(bool_option_to_string(do_not_route, "no routing, ", "use routing, "));
	options.append(bool_option_to_string(reuse_address, "reuse address, ", "don't reuse address, "));

	options.append("multicast group: ");
	options.append(option_to_string<string> (multicast_group));

	options.append(", multicast ttl: ");
	options.append(option_to_string<int> (multicast_ttl));

	Logger::info(options, port, host);
}
