
/**
 * This is the constructor for a WebSocket object, which takes in a url and 
 * array of string representing the protocols for this web socket.
 *
 * Note that this implementation does not care about:
 *  1) Secure web sockets
 *  2) The response handshake from the web server
 *     (as a consequence, this implementation automatically
 *     serializes multiple simultaneous web socket connections)
 *  3) Cookies, this implementation does not consider cookies
 *  4) Throwing proper DOM exceptions
 *
 *  @param  url         The URL that should be of the form 'ws://<url>:<port>'
 *  @param  protocols   The array of strings representing the protocols ??
 */
function WebSocket (url,  protocols)
{
    /**
     * Helper function to parse out the given URL according to web socket standards
     *
     *  @param  url The websocket-compliant URL to parse
     */
    function parseUrl(url)
    {
        var port = 80;
        var host = "";
        var resourceName = "/";
        var secure = false;

        // Pull out the protocol, prefix, should be 'ws'
        var url = url.toLowerCase();
        var hostDelimiterIndex = url.indexOf("://");

        // First, let's parse the url given, throw an exception with the error message if an error occurs
        if(hostDelimiterIndex == -1)
        {
            throw "Error: Parsing url, could not find delimeter '://'";
        }

        var protocol = url.substr(0, hostDelimiterIndex);

        // Check that the protocol is supported
        if(protocol == "ws")
        {
            secure = false;
        }
        else if(protocol == "wss")
        {
            //secure = true;
            //port = 443;
            throw "Error: Secure websockets are not supported at this time";
        }
        else
        {
            throw "Error: Parsing protocol, url must begin with 'ws://' or 'wss://'";
        }

        // Pull out the resourceName on the server
        var resourceNameDelimiterIndex = url.indexOf("/", hostDelimiterIndex+3);
        if(resourceNameDelimiterIndex != -1)
        {
            resourceName = url.substr(resourceNameDelimiterIndex);
        }
        else
        {
            resourceName = "/";
        }

        // Parse out the hostname
        var hostEndDelimiterIndex = -1;
        var portDelimiterIndex = url.indexOf(":", hostDelimiterIndex+3);
        if(portDelimiterIndex != -1)
        {
            hostEndDelimiterIndex = portDelimiterIndex;

            // Try to parse out the port
            port = url.substr(portDelimiterIndex+1, resourceNameDelimiterIndex-portDelimiterIndex-1);
        }
        else
        {
            hostEndDelimiterIndex = resourceNameDelimiterIndex;
        }

        host = url.substr(hostDelimiterIndex+3, hostEndDelimiterIndex - hostDelimiterIndex - 3);

        try
        {
            port = parseInt(port);
        }
        catch (error)
        {
            throw "Error: Could not parse port from URL";
        }
        return [host, port, resourceName, secure];
    };

    function parseProtocols(protocols)
    {
        if (protocols instanceof Array)
        {
            var protocols_string = "";
            for (protocol in protocols)
            {
                if (protocols_string != "")
                {
                    protocols_string += ",";
                }
                protocols_string += protocols[protocol]; 
            }
            return protocols_string;
        }
        else
        {
            return String(protocols);
        }
    };

    /* Just in case */
    function toUtf8( s )
    {
        return unescape( encodeURIComponent( s ) );
    }

    function fromUtf8( s )
    {
        return decodeURIComponent( escape( s ) );
    }

    // Helper function to connect
    function connect ( host, port, resourceName, origin, protocols)
    {
        var req = toUtf8( "GET " + resourceName + " HTTP/1.1\r\n" )
            var fields = ["Upgrade: WebSocket", "Connection: Upgrade"];
        var hostport = host.toLowerCase();

        if( port != 80 )
        {
            hostport += ":" + port;
        }

        fields.push( toUtf8( "Host: " + hostport ) );
        fields.push( toUtf8( "Origin: " + origin) ); 

        if(protocols)
        {
            var parsedProtocols = parseProtocols(protocols);
            fields.push( toUtf8( "Sec-WebSocket-Protocol: " + parsedProtocols ) );
        }

        // Some garbage we made up
        var key1 = "P388 O503D& ul7{K%gX (%715";
        var key2 = "1N?|kUT0o r3o4I97 N5-S 3O31";

        fields.push( toUtf8( "Sec-WebSocket-Key1: " + key1 ) );
        fields.push( toUtf8( "Sec-WebSocket-Key2: " + key2 ) );

        var headers = "";

        for( i in fields )
        {
            headers += fields[i] + toUtf8( "\r\n" );
        }

        headers += toUtf8( "\r\n");

        var key3 = toUtf8( "bcdefghi" );

        req += headers + key3

            return req;
    };

    /**
     * Handler invoked when any data is received over the connection 
     *
     *  @param  data    The data we received
     *  @param  socket  The socket on which we can reply
     */
    var receiveData = (function ( event )
    {
        // If this web socket is still in the process of connecting, this is 
        //  the handshake response
        if(state == This.CONNECTING)
        {
            // Consider the web socket connected, and trigger the associated callback
            state = This.OPEN;
            if(This.onopen != null)
                This.onopen();
        }
        else if(state == This.OPEN)
        {
            // Parse the messages from the received data
            var messages = parseWebSocketMessage(event.readBytes());

            // If there's no delay between callbacks, perform a callback immediately with all data
            if(This.callbackInterval == 0)
            {
                // Fire a message event for every message
                for (messageIndex in messages)
                {
                    if (This.onmessage != null)
                    {
                        var messageEvent = new MessageEvent(messages[messageIndex], event.getHost() + ":" + event.getPort());
                        This.onmessage(messageEvent);
                    }
                }
            }
            else
            {
                // Otherwise, append it to the queue of callback data
                for(messageIndex in messages)
                {
                    var messageEvent = new MessageEvent(messages[messageIndex], event.getHost() + ":" + event.getPort());
                    callbackQueue.push(messageEvent);
                }
    
                // Flush the queue after some number of millis 
                if(!callbacksFlushing)
                {
                    setTimeout(function() {flushCallbackQueue();}, This.callbackInterval);
                }
            }
        }
        else
        {
            throw "Error: received data on a closed connection!"
        }
    });


    /**
     * A helper function to send the received messages that have been queued for callbavks
     *  back at the specified callback interval.
     */
    var flushCallbackQueue = (function()
    {
        // Flush the queue of callback messages
        callbacksFlushing = true;
        for(i in callbackQueue) {
            This.onmessage(callbackQueue[i]);
        }
        callbackQueue = new Array();

        // If we still have a callback delay set, run this function again soon
        if(This.callbackInterval > 0 )
        {
            setTimeout(function() {flushCallbackQueue();}, This.callbackInterval);
        }
        else 
        {
            callbacksFlushing = false;
        }
    });


    /**
     * A function registered for error callbacks from our TCP client, that handles any errors
     *  that occur on the connection, and propagates them appropriately if necessary
     */
    var errorHandler = (function (message)
    {
        // If we're in the process of connecting, close the connection and fail permanently
        if (state == This.CONNECTING)
        {
            state = This.CLOSED;
            return;
        }

        if (This.onerror != null)
            This.onerror();
        console.error(message)
    });

    /**
     * Sends some data across a connected web socket
     *
     *  @param  data    The data to send
     */
    this.send = function ( data )
    {
        // Silently convert this data into a string if its not already a string
        data = String(data);

        // If this WebSocket is still connecting, throw an error!
        if (state == this.CONNECTING)
        {
            throw "Error: Cannot send data on a connecting WebSocket!";
        }

        // If this WebSocket is disconnected, buffer it instead of sending it
        if (state == this.CLOSED)
        {
            this.bufferedAmount += data.length;
            buffer[bufferIndex] = data;
            bufferIndex++;

            return false;
        }
        else
        {
            // Otherwise, it must be connected, frame and send the data
            client.sendBytes([0x00]);
            client.send(toUtf8(data)); 
            client.sendBytes([0xFF]);
            return true;
        }
    }

    /**
     * Function exposed to the API that closes this websocket, according to the web socket specification
     */
    this.close = function ()
    {
        // If the websocket is already closed, do nothing
        if (state == this.CLOSED)
        {
            return;
        }
        else
        {
            // Close the connection with the websocket server
            client.close();
            state = this.CLOSED;

            // Fire the callback for closing, if it's been defined
            if (this.onclose != null)
                this.onclose();
        }
    }

    // Try to load the SockIt plugin, make sure it's loaded before we continue
    try
    {
        var sockit = loadSockitPlugin();
    }
    catch (error)
    {
        throw "Error: Creating WebSocket, SockIt plugin is not loaded!";
    }

    /**
     * The interface for message event objects
     *
     *  @param  data    The data to be stored for this message event
     *  @param  origin  The origin from which the message event has arrived
     */
    function MessageEvent (data, origin)
    {
        this.__defineGetter__("data", function() { return data; })
        this.__defineGetter__("origin", function() { return origin; })
    };

    /**
     * Helper function to parse out the data from a websocket data packet
     *
     *  @param  data    The raw data received from a websocket (passed as a byte array)
     *  @param  The content of the websocket message
     */
    var parseWebSocketMessage = (function(data)
    {
        // Stores the results, only full messages
        var results = new Array();

        // Check if there is some data in the buffer & this is a new message
        if (inboundBuffer.length != 0 && data[0] == 0x00)
        {
            results.push(inboundBuffer.join(''));
            inboundBuffer = new Array();
        }

        // Dump the data onto the buffer
        inboundBuffer = inboundBuffer.concat(data);

        var inMessage = false;
        var lastIndex = 0;
        var tmpString = new Array();
        for (var i = 0; i < inboundBuffer.length; i++)
        {
            if(inMessage && inboundBuffer[i] == 0xFF)
            {
                lastIndex = i+1;
                var tmpArray = new Array();
                for (var j in tmpString)
                {
                    tmpArray.push(String.fromCharCode(tmpString[j]));
                }
                results.push(tmpArray.join(''));
                inMessage = false;
                tmpString = new Array();
            }
            else if (!inMessage && inboundBuffer[i] == 0x00)
            {
                inMessage = true;
            }
            else if(inMessage)
            {
                tmpString.push(inboundBuffer[i]);
            }
        }

        // If there is some remaining text, keep it on the buffer
        inboundBuffer = inboundBuffer.slice(lastIndex); 
        return results;
    });

    // Parse out the url and port
    var params = parseUrl(url);
    var host = params[0];
    var port = params[1];
    var resourceName = params[2];
    var secure = params[3] // unused;

    // Set some default callback handlers
    this.onopen = null;
    this.onclose = null;
    this.onmessage = null; 
    this.onerror = null; 

    // Configure delay between callback
    this.callbackInterval = 0;
    var callbacksFlushing = false;

    // The queue for data to be send back on a delayed callback,
    //  which holds a queue of message events
    var callbackQueue = new Array();

    // Define constants on this, and the URL passed in
    this.URL = url;
    this.CONNECTING = 0;
    this.OPEN = 1;
    this.CLOSED = 2;

    // Set the 'readyState' of this web socket, which is read only
    var state = this.CONNECTING;
    this.__defineGetter__("readyState", function() { return state; })

    // This represents the number of bytes found in the buffer, waiting to be sent
    this.bufferedAmount = 0;

    // Buffer of messages to send
    var buffer = new Array();
    var bufferIndex = 0;

    // Buffer for receiving data
    var inboundBuffer = new Array(); 

    // The hostname of this client
    var origin = "null";
    if (location.hostname != "")
    {
        origin = location.hostname;
    }

    // Start trying to connect
    var handshake = connect( host, port, resourceName, origin, protocols );

    // Create a private TCP connection using the SockIt plugin to do the heavy lifting
    var client = sockit.createTcpClient( host, port );

    // Register to perform callbacks if data is received or if there is an error
    client.addEventListener('data', receiveData);
    client.addEventListener('error', errorHandler);

    // Register callbacks to close the connection if the TCP connection is closed
    client.addEventListener('close', this.close);
    client.addEventListener('disconnect', this.close);

    // Send the opening handshake to the server
    client.send(handshake);

    // Yes, this is a terrible hack
    var This = this;
};
