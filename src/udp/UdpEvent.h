/*
 * File:   UdpEvent.h
 * Author: jtedesco
 *
 * Created on May 27, 2011, 10:47 AM
 */

#ifndef UDPREPLIER_H
#define	UDPREPLIER_H

#include "Event.h"
#include "Udp.h"

using boost::asio::ip::udp;

/**
 * A UDP implementation of a event to allow javascript to respond to data on a UDP connection.
 *
 * 	@see Event
 */
class UdpEvent : public Event
{
	public:

		/**
		 * Constructs a new <code>UdpEvent</code> object containing the UDP connection on which to reply.
		 *
		 * 	@param	udp			The UDP server or client associated with this event
		 * 	@param	socket		The UDP connection on which to reply
		 * 	@param	endpoint	The remote endpoint for this UDP event, from which we can find information about
		 * 						the remote host
		 * 	@param	data		The data this event was fired from
		 */
		UdpEvent(Udp * udp, boost::shared_ptr<udp::socket> socket, boost::shared_ptr<udp::endpoint> endpoint, string data);

		/**
		 *  Deconstructs the UDP event object, after a single reply.
		 */
		virtual ~UdpEvent();

		/**
		 * Replies on the UDP connection with some data.
		 *
		 *	@param	data	The data with which to reply
		 */
		virtual void send(const string & data);

		/**
		 * Replies on the UDP connection with some data.
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
		 * Gets the hostname of the remote endpoint of the UDP connection for this <code>UdpEvent</code>.
		 *
		 * 	@return The remote hostname of the UDP connection for this <code>UdpEvent</code>
		 */
		virtual string get_host();

		/**
		 * Gets the port of the remote endpoint of the UDP connection for this <code>UdpEvent</code>.
		 *
		 * 	@return The remote port of the UDP connection for this <code>UdpEvent</code>
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
		 * The remote hostname of the UDP connection for this <code>UdpEvent</code>
		 */
		string host;

		/**
		 * The remote port of the UDP connection for this <code>UdpEvent</code>
		 */
        int port;

		/**
		 * The socket corresponding to this UDP 'connection', on which to reply
		 */
        boost::shared_ptr<udp::socket> socket;

		/**
		 * The remote endpoint for this event
		 */
        boost::shared_ptr<udp::endpoint> endpoint;

		/**
		 * The UDP server or client associated with this event
		 */
		Udp * udp_object;
};

#endif	/* UDPREPLIER_H */
