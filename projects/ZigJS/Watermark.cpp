
#include "Watermark.h"
#include "BrowserHost.h"
#include "DOM/Node.h"
#include "DOM/Element.h"
#include "DOM/Document.h"
#include "DOM/Window.h"
#include "variant_list.h"
//#include "JSObject.h"
#include <cstdlib> // TODO: for rand()
#include <boost/format.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// 15 seconds without verification is the max we allow
const boost::posix_time::time_duration Watermark::MaxDurationBetweenVerifications = boost::posix_time::time_duration(0, 0, 15, 0);

static const char watermarkHTML[] = "This is a watermark. <img src='data:image/png;base64,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"
"='>";
static const char watermarkStyle[] = "position:fixed; bottom:10px; z-index:65535;height:100px;width:100%;";
const int VERIFY_WATERMARK_TIME = 1000;
Watermark::Watermark(FB::BrowserHostPtr browserPtr)
{
	m_ok = true;
	m_gotValidate = true;
	m_browser = browserPtr;
	m_tryCount = 0;
	m_token = rand();
	m_lastValidationTime = boost::posix_time::second_clock::universal_time();

	//TODO: make a nicer watermark
	FB::DOM::ElementPtr body = browserPtr->getDOMDocument()->getBody();
	m_element = browserPtr->getDOMDocument()->callMethod<FB::JSObjectPtr>("createElement", FB::variant_list_of("div"));
	m_element->SetProperty("innerHTML", watermarkHTML);
	//TODO: sorry :(
	FB::DOM::Node::create(m_element)->getProperty<FB::JSObjectPtr>("style")->SetProperty("cssText", watermarkStyle);
	body->callMethod<void>("appendChild", FB::variant_list_of(m_element));
}
Watermark::~Watermark()
{

}

bool Watermark::IsOk()
{
	if (m_ok) {
		if (m_lastValidationTime + MaxDurationBetweenVerifications < boost::posix_time::second_clock::universal_time()) {
			Log("Timed out since last validation!");
			m_ok = false;
		}
	}
	return m_ok;
}

static const char * verificationFunction = "var plugins = document.getElementsByTagName('object');"
"for (var i = 0; i < plugins.length; i++) {"
"    var o = plugins[i];"
    //TODO: test mime type of plugin to reduce falses
"    var wm = o.watermark;"
"    if (wm === undefined) continue;"
"    o.watermark = 0;"
    // unchangeable property on our plugin alone
"    if (o.watermark != wm) { o.watermark = wm; continue; }"
"    var rect = wm.getBoundingClientRect();"
"    var wmX = Math.floor(rect.left + (rect.width/2));"
"    var wmY = Math.floor(rect.top + (rect.height/2));"
"    if (document.elementFromPoint(wmX, wmY) != wm) {"
//TODO: compile log only in debug mode
"        console.log('invalidating because invisible'); console.log(wm); console.log(document.elementFromPoint(wmX, wmY));"
"        o.invalidate();"
"    } else {"
"        o.validate(%1%);" // %1% will be set to m_token
"    }"
"    break;"
"}";

bool Watermark::Test()
{
	//TODO: test for "known URLs" here!
	Log("Got watermark test!");
	if (m_ok) {
		
		if (!m_gotValidate) {
			m_ok = false;
			//TODO: remove/ifdef after debugging
			Log("watermark validation failed - didn't get successful validate!");
		} else { // got a good validate - reset state for next cycle
			m_tryCount = 0;
			m_gotValidate = false;
			m_token = rand();
			FB::BrowserHostPtr browser = m_browser.lock();
			if (browser) {
				Log("watermark validation ok, starting next round");
				//Log((boost::format(verificationFunction) % m_token).str());
				browser->evaluateJavaScript((boost::format(verificationFunction) % m_token).str());
				return true;
			}
		}
	}
	return false;
}

void Watermark::Invalidate()
{
	Log("Got invalidate from javascript!");
	m_ok = false;
}
void Watermark::Validate(int key)
{
	Log((boost::format("Got validate(%1%) against token(%2%) (current try count is %3%)") % key % m_token % m_tryCount).str());

	if (m_tryCount >= MaxAttempts) {
		m_ok = false;
		return;
	}
	m_tryCount++;
	if (key == m_token) {
		m_gotValidate = true;
		m_lastValidationTime = boost::posix_time::second_clock::universal_time();
	} //TODO: immediately fail otherwise?
}

