/**********************************************************\

  Auto-generated ZigJS.h

  This file contains the auto-generated main plugin object
  implementation for the awesome project

\**********************************************************/
#ifndef H_ZigJSPLUGIN
#define H_ZigJSPLUGIN

#include "PluginWindow.h"
#include "PluginEvents/MouseEvents.h"
#include "PluginEvents/AttachedEvent.h"

#include "PluginCore.h"

#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>


FB_FORWARD_PTR(ZigJS)
class ZigJS : public FB::PluginCore
{
protected:

	int m_lastFrame;
	
private:
	static unsigned long XN_CALLBACK_TYPE OpenNIThread(void * instance);
public:
	xn::Context m_context;
	xn::DepthGenerator m_depth;
	xn::GestureGenerator m_gestures;
	xn::HandsGenerator m_hands;
    static void StaticInitialize();
    static void StaticDeinitialize();
	
public:
    ZigJS();
    virtual ~ZigJS();

public:
    void onPluginReady();
    void shutdown();
    virtual FB::JSAPIPtr createJSAPI();
    // If you want your plugin to always be windowless, set this to true
    // If you want your plugin to be optionally windowless based on the
    // value of the "windowless" param tag, remove this method or return
    // FB::PluginCore::isWindowless()
    virtual bool isWindowless() { return false; }

    BEGIN_PLUGIN_EVENT_MAP()
        EVENTTYPE_CASE(FB::MouseDownEvent, onMouseDown, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseUpEvent, onMouseUp, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
        EVENTTYPE_CASE(FB::MouseMoveEvent, onMouseMove, FB::PluginWindow)
        EVENTTYPE_CASE(FB::AttachedEvent, onWindowAttached, FB::PluginWindow)
        EVENTTYPE_CASE(FB::DetachedEvent, onWindowDetached, FB::PluginWindow)
    END_PLUGIN_EVENT_MAP()

    /** BEGIN EVENTDEF -- DON'T CHANGE THIS LINE **/
    virtual bool onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *);
    virtual bool onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *);
    virtual bool onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *);
    virtual bool onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *);
    virtual bool onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *);
    /** END EVENTDEF -- DON'T CHANGE THIS LINE **/

	int getLastFrameID();
};


#endif

