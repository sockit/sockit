/** \mainpage
 *
 * \section intro_sec Introduction
 *
 * SockIt is an NPAPI plugin that allows Javascript to perform asynchronous, low-level networking functions. Our API allows web developers to create servers and clients which perform asynchronous network I/O by using Javascript events to perform callbacks. The API provides access to both TCP and UDP protocols, as well as basic concurrency control for tweaking performance.
 *
 * \section install_sec Installation
 *
 * Our plugin is currently packaged for 64-bit Chrome and Firefox on both Windows and Linux. To install our plugin, visit our <a href="http://sockit.github.com/sockit/downloads.html">downloads</a> page.
 *
 * \section features_sec Features
 *
 * Our plugin allows creation of TCP and UDP clients and servers, exposes a basic API for controlling each, and allows developers to perform asynchronous I/O with this clients and servers using Javascript event callbacks. Likewise, our API allows some concurrency control to improve performance, provides hooks to handle additional events such as errors, connection, disconnection, and asynchronously logs errors on the client machine for debugging.
 *
 * \section tutorial_sec Tutorial
 *
 * <p>To see a quick example of our plugin, see our <a href="http://sockit.github.com/sockit/tutorial.html">quick tutorial</a> or our <a href="http://sockit.github.com/sockit/tutorial/full_tutorial.html">full tutorial</a>.</p>
 * <p>To learn about everything that our plugin can do, see our <a href="http://sockit.github.com/sockit/api_documentation.html">API documentation</a>.</p>
 *
 * \section install_sec Building from Source
 * To get familiar with our code, browse this full source documentation.
 * <p>
 *	The build process is automated on both Windows and Linux, but has several dependencies. For full instructions on installation can be found <a href="../index.html">here</a>.
 * </p>
 */

/*
 * NetworkObject.h
 *
 *  Created on: Jun 10, 2011
 *      Author: jtedesco
 */

#ifndef NETWORKOBJECT_H_
#define NETWORKOBJECT_H_

#include <string>
#include <map>
#include <vector>
#include <queue>

#include "JSAPIAuto.h"

using std::string;
using std::vector;
using std::map;
using std::queue;
using std::wstring;

typedef uint16_t byte;
typedef wstring binary;

class NetworkObject: public FB::JSAPIAuto
{
	public:

		/**
		 * Constructs a generic network object by registering common events and API functions to javascript.
		 */
		NetworkObject();

		/**
		 * Deconstructs this network object
		 */
		virtual ~NetworkObject() {}

		/**
		 * Gracefully shutdown this server object, waiting until all sends have completed before freeing all resources for
		 * 	this server object and shutting down any open conections. This function is exposed the javascript API.
		 */
		virtual void shutdown() = 0;

		/**
		 * The javascript event fired when an error occurs, which sends the error message when fired.
		 */
		FB_JSAPI_EVENT(error, 1, (const string &));

		/**
		 * The javascript event fired when this object shuts down.
		 */
		FB_JSAPI_EVENT(close, 0, ());

		/**
		 * The javascript event fired when the client data. This event sends with it
		 * 	the data received, and an object, a <code>Event</code> on which to reply to the message.
		 *
		 * 	@see Event
		 */
		FB_JSAPI_EVENT(data, 1, (FB::JSAPIPtr));
};

#endif /* NETWORKOBJECT_H_ */
