/**********************************************************\

  Auto-generated ZigJSAPI.h

\**********************************************************/

#ifndef H_ZigJSAPI
#define H_ZigJSAPI

#include <string>
#include <sstream>
#include <boost/weak_ptr.hpp>
#include "JSAPIAuto.h"
#include "BrowserHost.h"
#include "ZigJS.h"

#ifdef _WIN32
// windows threads expect DWORD return value
typedef unsigned long thread_ret_t;
#else
// pthreads expects void* return value
typedef void *thread_ret_t;
#endif

const std::string VERSION = "0.9";

class ZigJSAPI : public FB::JSAPIAuto
{
public:
	
//	void XN_CALLBACK_TYPE GestureRecognizedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pIDPosition, const XnPoint3D* pEndPosition, void* pCookie);

    ZigJSAPI(const ZigJSPtr& plugin, const FB::BrowserHostPtr& host);
    virtual ~ZigJSAPI();

    ZigJSPtr getPlugin();

    // Event helpers
	FB_JSAPI_EVENT(WaveGesture, 3, (float,float,float));
	FB_JSAPI_EVENT(HandCreate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandUpdate, 5, (int,float,float,float,float));
	FB_JSAPI_EVENT(HandDestroy, 2, (int,float));

	FB_JSAPI_EVENT(UserEntered, 1, (int));
	FB_JSAPI_EVENT(UserLeft, 1, (int));
	FB_JSAPI_EVENT(UserTrackingStarted, 1, (int));
	FB_JSAPI_EVENT(UserTrackingStopped, 1, (int));

	//FB_JSAPI_EVENT(NewFrame, 2,(const FB::variant&, const FB::variant&));
	FB_JSAPI_EVENT(NewFrame, 1,(const std::string&));
	//FB_JSAPI_EVENT(NewFrame, 0, ());

	void onHandCreate(int handId, float x, float y, float z, float time);
	void onHandUpdate(int handId, float x, float y, float z, float time);
	void onHandDestroy(int handId, float time);

	void setHands(const FB::VariantList& hands);
	void setUsers(const FB::VariantList& users);

	void onUserEntered(int userId);
	void onUserLeft(int userId);
	void onUserTrackingStarted(int userId);
	void onUserTrackingStopped(int userId);

	//void onNewFrame(const FB::variant& users, const FB::variant& hands);
	void onNewFrame(const std::string& out);

private:
	ZigJSWeakPtr m_plugin;
    FB::BrowserHostPtr m_host;

	bool get_firingEvents();
	void set_firingEvents(const bool firingEvents);
	bool firingEvents;

	void update();

	static thread_ret_t XN_CALLBACK_TYPE timerThread(void * param);

	//TODO: unhack
public:
	void setImage(FB::JSAPIPtr img);
	FB::JSAPIPtr getImage() const;
private:
	FB::JSAPIPtr m_image;
	mutable boost::recursive_mutex m_imageMutex;

};

#endif // H_ZigJSAPI

