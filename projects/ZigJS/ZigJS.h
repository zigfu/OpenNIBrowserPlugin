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

class HandPoint
{
public:
	int handid;
	int userid;
	XnPoint3D position;
	HandPoint(int handid, int userid, XnPoint3D position)
		: handid(handid), userid(userid), position(position) 
	{}

};


FB_FORWARD_PTR(ZigJS)

FB_FORWARD_PTR(ZigJSAPI)

class ZigJS : public FB::PluginCore
{
private:
	static XN_THREAD_HANDLE s_threadHandle;

	static std::list<ZigJSAPIWeakPtr > s_listeners;
	static boost::recursive_mutex s_listenersMutex;

	// UI callbacks
	static void XN_CALLBACK_TYPE GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);
	static void XN_CALLBACK_TYPE HandCreateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandUpdateHandler(xn::HandsGenerator& generator, XnUserID user, const XnPoint3D* pPosition, XnFloat fTime, void* pCookie);
	static void XN_CALLBACK_TYPE HandDestroyHandler(xn::HandsGenerator& generator, XnUserID user, XnFloat fTime, void* pCookie);

	// User callbacks
	static void XN_CALLBACK_TYPE OnNewUser(xn::UserGenerator& generator, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnLostUser(xn::UserGenerator& generator, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnPoseDetected(xn::PoseDetectionCapability& poseDetection, const XnChar* strPose, XnUserID nId, void* pCookie);
	static void XN_CALLBACK_TYPE OnCalibrationStart(xn::SkeletonCapability& skeleton, const XnUserID nUserId, void* pCookie);
	static void XN_CALLBACK_TYPE OnCalibrationEnd(xn::SkeletonCapability& skeleton, const XnUserID nUserId, XnBool bSuccess, void* pCookie);

	static FB::VariantList GetJointsList(XnUserID userid);
	static FB::VariantList PositionToVariant(XnPoint3D pos);
	static FB::VariantList OrientationToVariant(XnMatrix3X3 ori);
	static FB::VariantList MakeUsersList();
	static FB::VariantList MakeHandsList();

	static XnUserID WhichUserDoesThisPointBelongTo(XnPoint3D point);

	// we keep a list of hand points
	static std::list<HandPoint> s_handpoints;

public:
	static xn::Context s_context;
	static xn::DepthGenerator s_depth;
	static xn::GestureGenerator s_gestures;
	static xn::HandsGenerator s_hands;
	static xn::UserGenerator s_users;
	static volatile bool s_quit;

	static int s_lastFrame;

    static void StaticInitialize();
    static void StaticDeinitialize();

	static void AddListener(ZigJSAPIWeakPtr listener);
	static void ReadFrame();
	
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

