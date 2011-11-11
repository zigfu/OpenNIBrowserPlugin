/**********************************************************\

  Auto-generated shitshit.cpp

  This file contains the auto-generated main plugin object
  implementation for the awesome project

\**********************************************************/

#include "shitshitAPI.h"

#include "shitshit.h"
struct shit_t {
	int * frameId;
	xn::Context * context;
	xn::DepthGenerator * depth;
};


extern "C" unsigned long __stdcall threadproc(void * ptr)
{
	shit_t * data = (shit_t *)ptr;
	int * frameId = data->frameId;
	xn::Context *shit = data->context;
	xn::DepthGenerator * depth = data->depth;
	XnStatus nRetVal = shit->StartGeneratingAll();
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail start generating");
		*frameId = -1;
		return -1;
	} else {
		FBLOG_INFO("xnInit", "ok start generating");
	}
	xn::DepthMetaData md;
	
	while(true) {
		shit->WaitAndUpdateAll();
		if (nRetVal != XN_STATUS_OK) {
			FBLOG_INFO("xnInit", "fail wait & update");
			*frameId = -111;
			break;
		} else {
			depth->GetMetaData(md);
			*frameId = (int)md.FrameID();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @fn shitshit::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginInitialize()
///
/// @see FB::FactoryBase::globalPluginInitialize
///////////////////////////////////////////////////////////////////////////////
void shitshit::StaticInitialize()
{
    // Place one-time initialization stuff here; As of FireBreath 1.4 this should only
    // be called once per process
}

///////////////////////////////////////////////////////////////////////////////
/// @fn shitshit::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginDeinitialize()
///
/// @see FB::FactoryBase::globalPluginDeinitialize
///////////////////////////////////////////////////////////////////////////////
void shitshit::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here. As of FireBreath 1.4 this should
    // always be called just before the plugin library is unloaded
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  shitshit constructor.  Note that your API is not available
///         at this point, nor the window.  For best results wait to use
///         the JSAPI object until the onPluginReady method is called
///////////////////////////////////////////////////////////////////////////////
shitshit::shitshit()
{
	m_lastFrame = -1337;
	XnStatus nRetVal = XN_STATUS_OK;
	nRetVal = m_context.Init();
	char temp[100];
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail context init");
		m_lastFrame = -5;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok context init");
	}
	//TODO: leak some memory
	XnLicense * license = new XnLicense();
	xnOSStrCopy(license->strKey, "0KOIk2JeIBYClPWVnMoRKn5cdY4=", sizeof(license->strKey));
	xnOSStrCopy(license->strVendor, "PrimeSense", sizeof(license->strVendor));
	m_context.AddLicense(*license);
	m_context.CreateAnyProductionTree(XN_NODE_TYPE_DEPTH, NULL, m_depth);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail get depth");
		m_lastFrame = -6;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok get depth");
	}
	XN_THREAD_HANDLE handle;
	shit_t *data = new shit_t;
	data->frameId = &m_lastFrame;
	data->context = &m_context;
	data->depth = &m_depth;
	nRetVal = xnOSCreateThread(threadproc, data, &handle);
	if (nRetVal != XN_STATUS_OK) {
		FBLOG_INFO("xnInit", "fail start thread");
		m_lastFrame = -7;
		return;
	} else {
		FBLOG_INFO("xnInit", "ok start thread");
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  shitshit destructor.
///////////////////////////////////////////////////////////////////////////////
shitshit::~shitshit()
{
    // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
    // root object) and tell the host to free the retained JSAPI objects then
    // unless you are holding another shared_ptr reference to your JSAPI object
    // they will be released here.
    releaseRootJSAPI();
    m_host->freeRetainedObjects();
}

void shitshit::onPluginReady()
{
    // When this is called, the BrowserHost is attached, the JSAPI object is
    // created, and we are ready to interact with the page and such.  The
    // PluginWindow may or may not have already fire the AttachedEvent at
    // this point.
}

void shitshit::shutdown()
{
    // This will be called when it is time for the plugin to shut down;
    // any threads or anything else that may hold a shared_ptr to this
    // object should be released here so that this object can be safely
    // destroyed. This is the last point that shared_from_this and weak_ptr
    // references to this object will be valid
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an instance of the JSAPI object that provides your main
///         Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<shitshit> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
///////////////////////////////////////////////////////////////////////////////
FB::JSAPIPtr shitshit::createJSAPI()
{
    // m_host is the BrowserHost
    return boost::make_shared<shitshitAPI>(FB::ptr_cast<shitshit>(shared_from_this()), m_host);
}

bool shitshit::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool shitshit::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}

bool shitshit::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
    return false;
}
bool shitshit::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *)
{
    // The window is attached; act appropriately
    return false;
}

bool shitshit::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
    // The window is about to be detached; act appropriately
    return false;
}

int shitshit::getLastFrameID() {
	return m_lastFrame;
}
