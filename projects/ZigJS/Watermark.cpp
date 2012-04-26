
#include "Watermark.h"

#ifndef NO_WATERMARK
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
#include "global/config.h" // for FBSTRING_PLUGIN_VERSION
// 15 seconds without verification is the max we allow
const boost::posix_time::time_duration Watermark::MaxDurationBetweenVerifications = boost::posix_time::time_duration(0, 0, 15, 0);

// note that the <img> has style='border : none;' because IE otherwise adds a blue border around the image because it's a link
static const char watermarkHTML[] = "<a href='http://zigfu.com/watermark'><img style='border : none;' alt='Powered by Zigfu' src='data:image/png;base64,"
"iVBORw0KGgoAAAANSUhEUgAAAKEAAABhCAMAAACJZzEwAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJ"
"bWFnZVJlYWR5ccllPAAAA2RpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdp"
"bj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6"
"eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMC1jMDYwIDYxLjEz"
"NDc3NywgMjAxMC8wMi8xMi0xNzozMjowMCAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJo"
"dHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlw"
"dGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wTU09Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEu"
"MC9tbS8iIHhtbG5zOnN0UmVmPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvc1R5cGUvUmVz"
"b3VyY2VSZWYjIiB4bWxuczp4bXA9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC8iIHhtcE1N"
"Ok9yaWdpbmFsRG9jdW1lbnRJRD0ieG1wLmRpZDoxRjM3Qzk4NDNGNkVFMTExOTc0M0E0QTU3OUFD"
"MTBGMyIgeG1wTU06RG9jdW1lbnRJRD0ieG1wLmRpZDpDMTUyOUVEQjdBQzExMUUxQUY1NkU2RTNC"
"M0M0OTA1QyIgeG1wTU06SW5zdGFuY2VJRD0ieG1wLmlpZDpDMTUyOUVEQTdBQzExMUUxQUY1NkU2"
"RTNCM0M0OTA1QyIgeG1wOkNyZWF0b3JUb29sPSJBZG9iZSBQaG90b3Nob3AgQ1M1IFdpbmRvd3Mi"
"PiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDoxRjM3Qzk4NDNG"
"NkVFMTExOTc0M0E0QTU3OUFDMTBGMyIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDoxRjM3Qzk4"
"NDNGNkVFMTExOTc0M0E0QTU3OUFDMTBGMyIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRG"
"PiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PlCv0kcAAAG/UExURT97jP///z97jD97"
"jD97jD97jD97jD97jD97jD97jD97jD97jD97jBSEOBWGQxaJTReLWBhuTxiOYhpCVBqSdxuVghxa"
"cRyIOByMhh2XjR+coiAbeiFGkyGWsCGhtyIAfCILgiMgjyMrlSM1nCNAoiNKqCOMOSRgtSRquyR1"
"wSSAyCSd0CWV1CWf2yWq4Sagoien3Cik1iqh0SuQOSyezC8AfS+YwTKTtzKVOjWNrDiHoTsAfjxz"
"hT1+kT97jEGdO0SAh0gAf0mEgkmhO0pzjEuDk0yElE53oU6JflGMPVGlPFM5WVOOeVUAgFVsjFeM"
"mlipPFmNm1yXb2EAgWGcamOUomWVo2ahZWpcjGulYW0AfG4Agm+cqW+2PXCqXHKeq3VUjHWuV3a6"
"PnmzUnoAg3uksH27U364TX6+Pn+nsoBNjIO9SYXCP4cAhIett4jBRItFjIywuo3GP5O1v5QAhZY9"
"jJm5wp+9xqAAhqA2jKXByasujKvFzK0Ah7LK0bYnjLfN1LoAiL/T2cEfjMPW28YAicsXjMzc4M/e"
"4tMAitYPjNnl6Nvm6d8Ai+EIjOXt8Ofu8ewAjPL29/P3+P///5qYjVsAAAANdFJOUwAABhtsb8DD"
"xs/S8/kPyAGRAAAHXUlEQVRo3u2b+X/TNhTA6cW1ht1jd9nG7pINGNAdaIXhjDG2LGwOZRiyAllp"
"YWFJ1piFLJi2EI8Eclh/8KzLp2Q7adOP+azvh9qRZeurp/eenuR627ZYy8jICD6Ojm+fBPGSye3j"
"ozbh2C4QR9k1xghHd4N4yu5RSjgO4irjlHBHbAl3UMLJ2BJOUkIQX9ki/F8S1iGRxiL5na2aP5pF"
"dFoxT3ARhFV0XISwB0AbMmkAUKSnvYrrYaYAq2avkrKbazubW4FwhZVWQwkhrFAK8oQUJoMpB2kV"
"P5BLaNZIiQhNxqyPECNlzEPGLMyZVVIRCGEO17WbBz0IaVcxaRP3QkAI62JC2E75CWEW10e3Nah6"
"hIToaraNFWTWbedISzmMViWgqE6KPLWNL1Epkq4g9fbsh9k0OYpd8ZRle7iMKNGlQiEhGt42AJA2"
"T3pFhhcP/AozQz4hIOPFJUQPq3vLaM060oH7rmDCDB1PpJM6M8QqNiVWxiesk3HlE1ZYNxw6bBMD"
"yiAjdalQPMpNpKcMaQg/tE4sbxH9YcdFpyXVgdgO207CopuQygprvufqVoineAlNtVVN82tWyBHr"
"V0DYAP0RUrVlHOdRoo2XcJHSmeXtRRpz+ISeeBhBh82UdUslUsReyQE/IdJbA3lwEx+rgG+HDceY"
"ceywyLdDUpMF3XBPsXQObE/Bhoi9pIJ7sSggLNLIG91TUNmKu8lohFa0adLSKrXpLCZMCQhRWV1M"
"yIs2FnW/hChiZwkYflCOqa4HHXOoj7BClcglRA8rcka5OhBhzhMMMFmKNlPx2HrDIkz1Bpj1cgMR"
"ujIHQObkBivPCgmRElHI6DNzGIiQZl9WUYVd7uEpT0SYISqOnn01iyAq4VaOvUW4RbhF+KQQpgua"
"3oFQ12sFKUL12XPzC6WFy7ObRqisQYdo6ZDqZ+dLRG7Nbg6h5OJDUgiqfpLxmXJhUwilFvRJPkCB"
"t2zA0sK6CRUk/QPCjrD6uZJL1ktYw821pPA6XhGZ4ulbG0tIm6sFOTEXEAoUP3vDDXhjgwj1gCpl"
"a2BV0yDytQ75JVD7BTdg6eomEGq0zprkRG4J3NgDWDq/CYQ69CpNFw/yZS/h6fUStiITOpRWgFAV"
"WKHHTUq/f370UDKZPPjZsS8GJMyT5tVwQmcn0ulokebmN58kLfn02PGB4mFaNSVwhtDD1WzJvAvw"
"p71PJ51y6HgIYVrxiKkJSVXLsqOOrK7ppqgFVJguazKPUCqT645eFohWnYN8cyaReNNFmDwaSChz"
"ZoYCaV5nzRU6jostw/xjWIQdlUqaeLduM+K7NNOTTjsArxxOJF5MeiSIUDI4UVenvmxIwtnN9mUm"
"Bi0x0u6A0HKZ4ZW9icQLyX4IuVOXxqJNWQTIIYSyzu52h6wCOG8BXjcBX/UBHgwi5DZfdsZDDUYk"
"9FkmZAHBcpSbhy3A6Y9MmSbnx4IIBXOrTSiYfv2ELTHhVUY4QwCn3397isi+dz44kDwY6MsGv3mb"
"UI1KWBATLlDAMxhwev+US/Z/GxgP87otjHbNSchAUIABckEXEWpATMi8JJF4JXnAwzc1dbt751S0"
"iC078mQfIctMVS8hjTYKCCd8z/TiD/d5+I7c7na7j3+LQih1HGmyj1AXEeriWcZDeCbxTPJdrwJP"
"3O9iWQontIOKMhzC64k9H3tH+MgfXSZzYYQ2YA0Mh3Am8cZbbr4f/npkAXbvhhDagGRtIiTUBiW8"
"lHjZBjxx7fa9rkeCCW1AQwZcQpaodgYlnHmODvGX1/551PXLw0BC2c4MZG+OzZTWyktAsrYYWn0R"
"mhH70p7XCd79Ll+Wgwhlw7dnYBMWuAG71i/hzEsm38/3uiJ5EOTLHEAHITf5QaviPgjnS5ee3Td1"
"8V8hX/fuqQBCiQPoIFQEKuyH8Hzp69dOiPXXfbwUNKc4nEThrfV4yQ/29z4If7n+/MVH3Yery8tL"
"c3NzS8vLd1YfOgZ46VRgjl3zeLGHUOYAkhVyH4Rnv/v1wdKPHuuf+/Pu6urq355yP6HMBbQJ/amN"
"rohWUt71qUV48vu5wfdgNS6gj7BTUHH6UyukxWs9nR8QzCXtV4PvEkswcBXg0CHN0NbY8o1DaGXj"
"ayjb0VowfNEdTpgX5KcBo4yCN5+QHzrXSaiGEcoCJXPXy8amEXbY4kDnLJjEc4q9YTd0QpWZlO6a"
"cdyLrTJnV0TbeEJur82AnDas5uUOf5xpqHfv26gbTij5B9FA+xhA0gy6sy+VW2sGb0ddUg3/RnJa"
"s+vqNRX9Uob2TkpRZOqzmr39VDPst/CmfhWZsyLLo2CTJ1ckJQ2GRmjHI50zS8bnvZ7u2T6PHaHk"
"ebHCYl4nNoQKcx9sV0q5Y+9LxISQP6eEvGLcXDs0BLNOfAj5b+/yMSLkbsEOa4wHe7/sf81tDE2D"
"g74Bd/+rgKFKIG6EaO5SNZxia6oChiqx/8bnKUq4M7aEOynhRGwJJ56Y7/W2jcUTcfeY47vRie2x"
"49sx4fhuNObf3o7EXeJP+B+E/6d7/EfaCAAAAABJRU5ErkJggg==" // end of base64 encoded image
"' onload='(function(a){var b=new XMLHttpRequest;b.open(\"GET\",\"http://zigfu.com/watermark?v=" FBSTRING_PLUGIN_VERSION "\",true);b.onreadystatechange=function(){if(b.readyState==4&&b.status==200){var c=document.createElement(\"div\");c.innerHTML=b.responseText;a.appendChild(c);}};b.send()}(this.parentElement.parentElement))'></a>";
static const char watermarkStyle[] = "position:fixed;display:block;left:10px;bottom:10px;z-index:65535;";
const int VERIFY_WATERMARK_TIME = 1000;
Watermark::Watermark(FB::BrowserHostPtr browserPtr)
{
	m_ok = true;
	m_browser = browserPtr;
	m_tryCount = 0;
	m_token = rand();
	m_lastValidationTime = boost::posix_time::second_clock::universal_time();

	//TODO: make a nicer watermark
	FB::DOM::ElementPtr body = browserPtr->getDOMDocument()->getBody();
	FB::DOM::DocumentPtr doc = browserPtr->getDOMDocument();
	if (!doc) {
		m_ok = false;
		//browserPtr->htmlLog("doc invalid on add watermark :(");
		return;
	}
	try {
		//browserPtr->htmlLog("wm: creating element");
		FB::DOM::ElementPtr elem;
		elem = doc->createElement("div");//doc->callMethod<FB::JSObjectPtr>("createElement", FB::variant_list_of("div"));
		m_element = elem->getJSObject();
		if (!m_element) {
			m_ok = false;
			//browserPtr->htmlLog("element is bad :(");
			return;
		}

		//browserPtr->htmlLog("wm: appending child");
		body->appendChild(elem);

		//TODO: sorry :(
		//browserPtr->htmlLog("wm: setting style");
		elem->getProperty<FB::JSObjectPtr>("style")->SetProperty("cssText", watermarkStyle);
		//browserPtr->htmlLog("wm: setting innerHTML");
		m_element->SetProperty("innerHTML", watermarkHTML);
	} catch(const FB::script_error& ex) {
		//browserPtr->htmlLog(ex.what());
		m_ok = false;
	}
}
Watermark::~Watermark()
{
}

bool Watermark::IsOk()
{
	//if (m_ok) {
		if (m_lastValidationTime + MaxDurationBetweenVerifications < boost::posix_time::second_clock::universal_time()) {
			Log("Timed out since last validation!");
			m_ok = false;
		} else {
			m_ok = true;
		}
	//}
	return m_ok;
}

static const char * verificationFunction = "function alpha(elem) {"
"  var opacity = elem.style.opacity;"
"  if (!opacity) { "
"    opacity = 1;"
"  } else {"
"    opacity = +opacity;"
"  }"
"  var filter = elem.style.filter;"
"  if (filter) {"
	     // use RE to parse 'filter' css property
"    var result = /alpha\\s*\\(\\s*opacity=\\s*(\\d+)\\s*\\)/.exec(filter);"
"    if (result) {"
"      return Math.min((+result[1])/100, opacity);"
"    }"
"  }"
"  return opacity;"
"};"
"function isOpaque(element, threshold) {"
"  if (alpha(element) < threshold) {"
"    return false;"
"  }"
"  for(var i = element.children.length - 1; i >= 0; i--) {"
"    if (!isOpaque(element.children[i], threshold)) {"
"      return false;"
"    }"
"  }"
"  return true;"
"};"
"var plugins = document.getElementsByTagName('object');"
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
"    var item = document.elementFromPoint(wmX, wmY);"
"    if ((item != wm && (item.parentNode != wm) && (item.parentNode.parentNode != wm)) || (!isOpaque(wm, 0.8))) {"
// innerHTML-based comparison 
//"    var item = document.elementFromPoint(wmX, wmY);var wmh = wm.innerHTML"
//"    if (item.innerHTML != wmh && (item.parentNode.innerHTML != wmh) && (item.parentNode.parentNode.innerHTML != wmh)) {"
#ifdef _DEBUG
"        console.log('invalidating because invisible'); console.log(wm); console.log(item); console.log(item.parentNode); console.log(item.parentNode.parentNode);"
"		 console.log(this);"
#endif
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
	//if (m_ok) {
		

		m_tryCount = 0;
		m_token = rand();
		FB::BrowserHostPtr browser = m_browser.lock();
		if (browser) {
			Log("watermark validation ok, starting next round");
			//Log((boost::format(verificationFunction) % m_token).str());
			browser->evaluateJavaScript((boost::format(verificationFunction) % m_token).str());
			return true;
		}
	//}
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
		m_lastValidationTime = boost::posix_time::second_clock::universal_time();
	} //TODO: immediately fail otherwise?
}
#endif


