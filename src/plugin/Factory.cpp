/**********************************************************\
 
 Auto-generated Factory.cpp
 
 This file contains the auto-generated factory methods 
 for the SockIt project
 
 \**********************************************************/

#include "FactoryBase.h"
#include "SockIt.h"
#include <boost/make_shared.hpp>

using std::string;

class PluginFactory: public FB::FactoryBase
{
	public:

		/**
		 * Creates a plugin object matching the provided mimetype. If mimetype is empty, returns the default plugin.
		 * 	@return A plugin object matching the project's mimetype.
		 */
		FB::PluginCorePtr createPlugin(const string& mimetype)
		{
			return boost::make_shared<SockIt>();
		}

		/**
		 * Performs one-time global initialization of the plugin.
		 *
		 * @see FB::FactoryBase::globalPluginInitialize
		 */
		void globalPluginInitialize()
		{
			SockIt::StaticInitialize();
		}

		/**
		 * Performs global deinitialization, or destruction, of the plugin.
		 *
		 * @see FB::FactoryBase::globalPluginDeinitialize
		 */
		void globalPluginDeinitialize()
		{
			SockIt::StaticDeinitialize();
		}
};

/**
 *	@fn getFactoryInstance()
 *	@brief  Returns the factory instance for this plugin module
 */
FB::FactoryBasePtr getFactoryInstance()
{
	static boost::shared_ptr<PluginFactory> factory = boost::make_shared<PluginFactory>();
	return factory;
}

