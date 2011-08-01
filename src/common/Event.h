/**
 * Interface for a general event
 *
 * File:   Event.h
 * Author: jtedesco
 *
 * Created on May 27, 2011, 11:29 AM
 */

#ifndef REPLIER_H
#define	REPLIER_H

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "JSAPIAuto.h"

using std::string;
using std::vector;
using std::wstring;

typedef uint16_t byte;
typedef wstring binary;

/**
 * Generic interface used to reply to data on some connection from Javascript.
 */
class Event: public FB::JSAPIAuto
{
	public:

		/**
		 * Constructor that register the Javascript API for event objects
		 */
		Event();

		/**
		 * Deconstructs a event object
		 */
		virtual ~Event();

		/**
		 * Reply on this <code>Event</code>'s connection with some data.
		 *
		 * 	@param	data	The data to send
		 */
		virtual void send(const string & data) = 0;

		/**
		 * Reply on this <code>Event</code>'s connection with some data, given as bytes.
		 *
		 *	@param	bytes	The bytes of data with which to reply
		 */
		virtual void send_bytes(const vector<byte> & bytes) = 0;

		/**
		 * Reads the string data that belongs to this event.
		 *
		 * 	@return	The string data received when this event was fired
		 */
		virtual string read() const = 0;

		/**
		 * Reads the byte data that belongs to this event
		 *
		 * 	@return	The byte data received when this event was fired
		 */
		virtual FB::VariantList read_bytes() const = 0;

		/**
		 * Get the address, or hostname of the remote endpoint for this event.
		 * 	@return	The hostname of the remote endpoint for this event.
		 */
		virtual string get_host() = 0;

		/**
		 * Get the port associated with the remote endpoint of this event.
		 *  @return port The port associated with the remote endpoint of this event.
		 */
		virtual unsigned short get_port() = 0;

		/**
		 * The javascript event fired when an error occurs, which sends the error message when fired.
		 */
		FB_JSAPI_EVENT(error, 1, (const string &));
};
#endif

