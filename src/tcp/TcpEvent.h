/*
 * File:   TcpEvent.h
 * Author: jtedesco
 *
 * Created on May 27, 2011, 11:10 AM
 */

#ifndef TCPREPLIER_H
#define	TCPREPLIER_H

#include <boost/asio.hpp>

#include "JSAPIAuto.h"
#include "Event.h"
#include "Tcp.h"

using boost::asio::ip::tcp;
using std::string;

/**
 * A TCP implementation of a event to allow javascript to respond to data on a TCP connection.
 *
 * 	@see Event
 */
class TcpEvent: public Event
{
	public:

		/**
		 * Constructs a new <code>TcpEvent</code> object containing the TCP connection on which to reply.
		 *
		 * 	@param	tcp			The TCP server or client associated with this event
		 * 	@param	connection	The TCP connection on which to reply
		 * 	@param	data		The data received when this event was fired
		 */
		TcpEvent(Tcp * tcp, boost::shared_ptr<tcp::socket> connection, string data);

		/**
		 * Deconstructs the TCP event object, after a single reply.
		 */
		virtual ~TcpEvent();

		/**
		 * Replies on the TCP connection with some data.
		 *
		 *	@param	data	The data with which to reply
		 */
		virtual void send(const string & data);

		/**
		 * Replies on the TCP connection with some data.
		 *
		 *	@param	bytes	The bytes of data with which to reply
		 */
		virtual void send_bytes(const vector<byte> & bytes);

		/**
		 * Reads the string data that belongs to this event.
		 *
		 * 	@return	The string data received when this event was fired
		 */
		virtual string read() const;

		/**
		 * Reads the byte data that belongs to this event
		 *
		 * 	@return	The byte data received when this event was fired
		 */
		virtual FB::VariantList read_bytes() const;

		/**
		 * Gets the hostname of the remote endpoint of the TCP connection for this <code>TcpEvent</code>.
		 *
		 * 	@return The remote hostname of the TCP connection for this <code>TcpEvent</code>
		 */
		virtual string get_host();

		/**
		 * Gets the port of the remote endpoint of the TCP connection for this <code>TcpEvent</code>.
		 *
		 * 	@return The remote port of the TCP connection for this <code>TcpEvent</code>
		 */
		virtual unsigned short get_port();

	private:

		/**
		 * The data received when this event was fired
		 */
		string data;

		/**
		 * A flag to prevent this event from blowing up if was initialized improperly
		 */
		bool failed;

		/**
		 * The TCP server or client associated with this event
		 */
		Tcp * tcp_object;

		/**
		 * The remote hostname of the TCP connection for this <code>TcpEvent</code>
		 */
		string host;

		/**
		 * The remote port of the TCP connection for this <code>TcpEvent</code>.
		 */
		int port;

		/**
		 * The TCP connection on which to reply
		 */
		boost::shared_ptr<tcp::socket> connection;
};
#endif	/* TCPREPLIER_H */

