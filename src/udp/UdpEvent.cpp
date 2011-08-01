/*
 * UdpEvent.cpp
 *
 * This object is passed as an argument to a Javascript callback when an object gets a message. It enables the
 * Javascript to send a reply.
 */

#include "UdpEvent.h"

UdpEvent::UdpEvent(Udp * udp_object, boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint, string data) :
	port(-1), socket(socket), endpoint(endpoint), udp_object(udp_object), failed(false), data(data)
{
	// Check to see if any the parameters are null, and log and fail if this occurs
	if (endpoint && udp_object)
	{
		// Initialize only if it's safe
		host = boost::lexical_cast<string>(endpoint->address());
		port = endpoint->port();
	}
	else
	{
		// Fail permanently and log it
		failed = true;

		string message("UDP Event was not properly initialized, permanently failed.");
		Logger::error(message, port, host);
		fire_error(message);
	}
}

UdpEvent::~UdpEvent()
{
	// do not free socket, endpoint or udp here
}

void UdpEvent::send_bytes(const vector<byte> & bytes)
{
    string data;

    for (int i = 0; i < bytes.size(); i++)
    {
        data.push_back((unsigned char) bytes[i]);
    }

	send(data);
}

void UdpEvent::send(const string & data)
{
	// Don't send if we've permanently failed
	if (failed)
	{
		string message("UDP event failed to send, Event already failed permanently");
		Logger::error(message, port, host);
		fire_error(message);
        return;
    }

	if (udp_object && udp_object->failed)
	{
		// Log & fire an error
		string message("UDP event failed trying to reply to a UDP object that has permanently failed!");
		Logger::error(message, port, host);
		fire_error(message);
        return;
	}

	if (socket && endpoint)
	{
		udp_object->pending_sends++;
		socket->async_send_to(boost::asio::buffer(data.data(), data.size()), *endpoint, 
                boost::bind(&Udp::send_handler, udp_object, _1, _2, data, host, endpoint->port()));
	}
}

string UdpEvent::read() const
{
	return data;
}

FB::VariantList UdpEvent::read_bytes() const
{
	FB::VariantList fb_bytes;

    for (int i = 0; i < data.size(); i++)
    {
        fb_bytes.push_back((unsigned char) (data.data())[i]);
    }

	return fb_bytes;
}

string UdpEvent::get_host(void)
{
	return host;
}

unsigned short UdpEvent::get_port(void)
{
	return port;
}
