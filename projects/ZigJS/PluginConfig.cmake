#/**********************************************************\ 
#
# Auto-Generated Plugin Configuration file
# for ZigJS
#
#\**********************************************************/

set(PLUGIN_NAME "ZigJS")
set(PLUGIN_PREFIX "ZIGJS")
set(COMPANY_NAME "ZigFu")

# ActiveX constants:
set(FBTYPELIB_NAME ZigJSLib)
set(FBTYPELIB_DESC "ZigJS 0.97 Type Library")
set(IFBControl_DESC "ZigJS Control Interface")
set(FBControl_DESC "ZigJS Control Class")
set(IFBComJavascriptObject_DESC "ZigJS IComJavascriptObject Interface")
set(FBComJavascriptObject_DESC "ZigJS ComJavascriptObject Class")
set(IFBComEventSource_DESC "ZigJS IFBComEventSource Interface")
set(AXVERSION_NUM "1")

# NOTE: THESE GUIDS *MUST* BE UNIQUE TO YOUR PLUGIN/ACTIVEX CONTROL!  YES, ALL OF THEM!
set(FBTYPELIB_GUID 0651752b-142c-5830-b9a7-ebc91de7b74b)
set(IFBControl_GUID c1030034-110c-5cce-9001-13157fcdeff2)
set(FBControl_GUID 1b7f2c65-a72d-5b96-b7ab-2eed12992bda)
set(IFBComJavascriptObject_GUID c5d47965-7f55-5e8a-8d6c-2f2057ba36b1)
set(FBComJavascriptObject_GUID 0acf2f0a-08a5-57c0-9582-016427d1119a)
set(IFBComEventSource_GUID 656cce92-f0e9-5f44-9afa-57879d949486)

# these are the pieces that are relevant to using it from Javascript
set(ACTIVEX_PROGID "ZigFu.ZigJS")
set(MOZILLA_PLUGINID "zigfu.com/ZigJS")

# strings
set(FBSTRING_CompanyName "ZigFu")
set(FBSTRING_PluginDescription "Motion control in the browser")
set(FBSTRING_PLUGIN_VERSION "0.9.7")
set(FBSTRING_LegalCopyright "Copyright 2012 Motion Arcade Inc.")
set(FBSTRING_PluginFileName "np${PLUGIN_NAME}v${FBSTRING_PLUGIN_VERSION}.dll")
set(FBSTRING_ProductName "ZigJS")
set(FBSTRING_FileExtents "")
set(FBSTRING_PluginName "ZigJS")
set(FBSTRING_MIMEType "application/x-zig")

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

add_firebreath_library(log4cplus)
add_firebreath_library(jsoncpp)
