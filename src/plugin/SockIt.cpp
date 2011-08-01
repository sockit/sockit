/**********************************************************\

  Auto-generated SockIt.cpp

  This file contains the auto-generated main plugin object
  implementation for the SockIt project

\**********************************************************/

#include "SockItAPI.h"

#include "SockIt.h"

void SockIt::StaticInitialize()
{
    // add any library initialization routines here
}

void SockIt::StaticDeinitialize()
{
    Logger::shutdown();
}

SockIt::SockIt()
{
}

SockIt::~SockIt()
{
    releaseRootJSAPI();
    m_host->freeRetainedObjects();
}

void SockIt::onPluginReady()
{
}

void SockIt::shutdown()
{
}


FB::JSAPIPtr SockIt::createJSAPI()
{
    return boost::make_shared<SockItAPI>(FB::ptr_cast<SockIt>(shared_from_this()), m_host);
}

bool SockIt::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool SockIt::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool SockIt::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}
bool SockIt::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *)
{
    // The window is attached; act appropriately
    return false;
}

bool SockIt::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    // The window is about to be detached; act appropriately
    return false;
}

