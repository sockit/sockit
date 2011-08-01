/**********************************************************
 Auto-generated SockItAPI.cpp

 \**********************************************************/

#include "JSObject.h"
#include "variant_list.h"
#include "DOM/Document.h"

#include "SockItAPI.h"

SockItAPI::SockItAPI(const SockItPtr& plugin, const FB::BrowserHostPtr& host) :
	m_plugin(plugin), m_host(host)
{
	// Register root methods for creating network threads
	registerMethod("createThread", make_method(this, &SockItAPI::create_thread));

	// Register root methods for creating servers & clients
	registerMethod("createUdpClient", make_method(this, &SockItAPI::create_udp_client));
	registerMethod("createUdpServer", make_method(this, &SockItAPI::create_udp_server));
	registerMethod("createTcpClient", make_method(this, &SockItAPI::create_tcp_client));
	registerMethod("createTcpServer", make_method(this, &SockItAPI::create_tcp_server));

	// Register methods for converting to and from binary data
	registerMethod("toBinary", make_method(this, &SockItAPI::convert_to_binary));
	registerMethod("fromBinary", make_method(this, &SockItAPI::convert_from_binary));
}

SockItAPI::~SockItAPI()
{
}

SockItPtr SockItAPI::getPlugin()
{
	SockItPtr plugin(m_plugin.lock());
	if (!plugin)
	{
		throw FB::script_error("The plugin is invalid");
	}
	return plugin;
}

boost::shared_ptr<NetworkThread> SockItAPI::create_thread()
{
	return boost::shared_ptr<NetworkThread>(new NetworkThread());
}

boost::shared_ptr<TcpServer> SockItAPI::create_tcp_server(int port, boost::optional<map<string, string> > options)
{
	return default_thread.create_tcp_server(port, options);
}

boost::shared_ptr<TcpClient> SockItAPI::create_tcp_client(const string & host, int port, 
                boost::optional<map<string, string> > options)
{
	return default_thread.create_tcp_client(host, port, options);
}

boost::shared_ptr<UdpServer> SockItAPI::create_udp_server(int port, boost::optional<map<string, string> > options)
{
	return default_thread.create_udp_server(port, options);
}

boost::shared_ptr<UdpClient> SockItAPI::create_udp_client(const string &host, int port,
                boost::optional<map<string, string> > options)
{
	return default_thread.create_udp_client(host, port, options);
}

binary SockItAPI::convert_to_binary(const vector<byte> bytes)
{
	// Convert it to a character array and create a string from it
	binary data;

    for (int i = 0; i < bytes.size(); i++)
    {
        data.push_back((unsigned char) bytes[i]);
    }

	return data;
}

FB::VariantList SockItAPI::convert_from_binary(binary data)
{
	vector<byte> bytes;
	for(int i = 0; i < data.size(); i++)
	{
		bytes.push_back((unsigned char) data.data()[i]);
	}

	FB::VariantList fb_bytes;
	std::copy(bytes.begin(), bytes.end(), std::back_inserter(fb_bytes));
	return fb_bytes;
}
