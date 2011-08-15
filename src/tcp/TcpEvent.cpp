/*
 * File:   TcpEvent.cpp
 * Author: jtedesco
 *
 * Created on May 27, 2011, 11:10 AM
 */

#include "TcpEvent.h"

#include <stdio.h>

TcpEvent::TcpEvent(Tcp * _tcp_object, boost::shared_ptr<tcp::socket> _connection, string _data) :
	tcp_object(_tcp_object), connection(_connection), failed(false), data(_data)
{
	// Check to see if any the parameters are null, and log and fail if this occurs
	if(tcp_object && connection)
	{
		// Initialize only if it's safe
		port = connection->remote_endpoint().port();
		host = connection->remote_endpoint().address().to_string();
	}
	else
	{
		// Fail permanently and log it
		failed = true;

		string message("TCP event was not properly initialized, permanently failed.");
		Logger::error(message, port, host);
		fire_error(message);
	}
}

TcpEvent::~TcpEvent()
{
    // do not free socket, endpoint or tcp here
}

void TcpEvent::send_bytes(const vector<byte> & bytes)
{
    string data;

    for (int i = 0; i < bytes.size(); i++)
    {
        data.push_back((unsigned char) bytes[i]);
    }

	send(data);
}

void TcpEvent::send(const string & data)
{
	// Don't send if we've permanently failed
	if(failed)
	{
		string message("TCP event failed to send, event already failed permanently");
		Logger::error(message, port, host);
		fire_error(message);
    }

	if(!tcp_object->failed)
	{
		tcp_object->active_jobs++;
        boost::asio::async_write(*connection, boost::asio::buffer(data.data(), data.size()),
				boost::bind(&Tcp::send_handler, tcp_object, _1, _2, data, host, port, connection));
	}
	else
	{
		string message("TCP event failed trying to reply on a permanently failed TCP object");
		Logger::error(message, port, host);
		fire_error(message);
	}
}

string TcpEvent::read() const
{
	return data;
}

FB::VariantList TcpEvent::read_bytes() const
{
    FB::VariantList fb_bytes;

    for (int i = 0; i < data.size(); i++)
    {
        fb_bytes.push_back((unsigned char) (data.data())[i]);
    }

	return fb_bytes;
}

string TcpEvent::get_host()
{
	return host;
}

unsigned short TcpEvent::get_port()
{
	return port;
}
