
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

const int VERIFY_WATERMARK_TIME = 1000;
Watermark::Watermark(FB::BrowserHostPtr browserPtr)
{
	m_ok = true;
	m_gotValidate = true;
	m_browser = browserPtr;
	m_tryCount = 0;
	m_token = rand();

	//TODO: make a nicer watermark
	FB::DOM::ElementPtr body = browserPtr->getDOMDocument()->getBody();
	m_element = browserPtr->getDOMDocument()->callMethod<FB::JSObjectPtr>("createElement", FB::variant_list_of("div"));
	m_element->SetProperty("innerHTML", "test test");
	body->callMethod<void>("appendChild", FB::variant_list_of(m_element));
}
Watermark::~Watermark()
{

}

bool Watermark::IsOk()
{
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
	FB::BrowserHostPtr browser = m_browser.lock();
	browser->htmlLog("Got watermark test!");
	if (m_ok) {
		
		if (!m_gotValidate) {
			m_ok = false;
			//TODO: remove/ifdef after debugging
			if (browser) {
				browser->htmlLog("watermark validation failed - didn't get successful validate!");
			}
		} else { // got a good validate - reset state for next cycle
			m_tryCount = 0;
			m_gotValidate = false;
			m_token = rand();
			if (browser) {
				browser->htmlLog("watermark validation ok, starting next round");
				//browser->htmlLog((boost::format(verificationFunction) % m_token).str());
				browser->evaluateJavaScript((boost::format(verificationFunction) % m_token).str());
				return true;
			}
		}
	}
	return false;
}

void Watermark::Invalidate()
{
	FB::BrowserHostPtr browser = m_browser.lock();
	if (browser) {
		browser->htmlLog("Got invalidate from javascript!");
	}
	m_ok = false;
}
void Watermark::Validate(int key)
{
	FB::BrowserHostPtr browser = m_browser.lock();
	if (browser) {
		browser->htmlLog((boost::format("Got validate(%1%) against token(%2%) (current try count is %3%)") % key % m_token % m_tryCount).str());
	}
	if (m_tryCount >= MaxAttempts) {
		m_ok = false;
		return;
	}
	m_tryCount++;
	if (key == m_token) {
		m_gotValidate = true;
	} //TODO: immediately fail otherwise?
}

