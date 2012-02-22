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

#include "json/json.h"
#include "Timer.h"


FB_FORWARD_PTR(ZigJS);

FB_FORWARD_PTR(ZigJSAPI);

FB_FORWARD_PTR(Sensor);

class ZigJS : public FB::PluginCore
{
private:

	static std::list<ZigJSAPIWeakPtr > s_listeners;
	static SensorPtr s_sensor;
	//static boost::recursive_mutex s_listenersMutex; //unneeded since we're running everything on the same thread now
	static SensorPtr InitSensor(); // TODO: point to some factory function?

	static FB::TimerPtr s_timer;

	static bool s_getDepth;
	static bool s_getImage;
	static bool s_isWebplayer;
public:
	static void SetStreams(bool getDepth, bool getImage, bool isWebplayer);

    static void StaticInitialize();
    static void StaticDeinitialize();

	static void AddListener(ZigJSAPIWeakPtr listener);
	static void ReadFrame(void *);
	static void TimerCallback();
	static bool IsSensorConnected();

	static FB::VariantList ConvertImageToWorldSpace(const std::vector<double>& points);
	static FB::VariantList ConvertWorldToImageSpace(const std::vector<double>& points);
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
    virtual bool isWindowless() { return true; }

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

};


#endif

