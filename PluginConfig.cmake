#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for Low Level Network Plugin
#
#\**********************************************************/

set(PLUGIN_NAME "SockIt")
set(PLUGIN_PREFIX "SockIt")
set(COMPANY_NAME "SockIt")

# ActiveX constants:
set(FBTYPELIB_NAME SockItLib)
set(FBTYPELIB_DESC "SockIt 1.0 Type Library")
set(IFBControl_DESC "SockIt Control Interface")
set(FBControl_DESC "SockIt Control Class")
set(IFBComJavascriptObject_DESC "SockIt IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "SockIt ComJavascriptObject Class")
set(IFBComEventSource_DESC "SockIt IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 2B8B62D3-E60F-48CC-BD2C-EA5EB8A35AC7)
set(IFBControl_GUID A4AFAFFF-CFD0-4AA9-B374-8AFE7CDAEFE8)
set(FBControl_GUID 8DF8483E-2CC1-4FAD-9677-A2ECBFB97499)
set(IFBComJavascriptObject_GUID AA1BF5D5-FE79-472B-AEED-BA88F90538B2)
set(FBComJavascriptObject_GUID D51A75ED-4F10-4082-8B77-B230DEDAB901)
set(IFBComEventSource_GUID 59543CCA-F732-4A8C-BB40-7C0277B26F35)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "sockit.SockIt")
set(MOZILLA_PLUGINID "sockit.com/SockIt")

# strings
set(FBSTRING_CompanyName "sockit")
set(FBSTRING_FileDescription "sockit.SockIt")
set(FBSTRING_PLUGIN_VERSION "0.0.1.0")
set(FBSTRING_LegalCopyright "Copyright 2011 SockIt")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}.dll")
set(FBSTRING_ProductName "sockit")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "sockit")
set(FBSTRING_MIMEType "application/x-sockit")

# Uncomment this next line if you're not planning on your plugin doing
# any drawing:

set (FB_GUI_DISABLED 1)

# Mac plugin settings. If your plugin does not draw, set these all to 0
set(FBMAC_USE_QUICKDRAW 0)
set(FBMAC_USE_CARBON 0)
set(FBMAC_USE_COCOA 0)
set(FBMAC_USE_COREGRAPHICS 0)
set(FBMAC_USE_COREANIMATION 0)
set(FBMAC_USE_INVALIDATINGCOREANIMATION 0)

# If you want to register per-machine on Windows, uncomment this line
#set (FB_ATLREG_MACHINEWIDE 1)

add_boost_library(filesystem)
add_boost_library(date_time)
