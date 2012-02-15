
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
#include "global/config.h" // for FBSTRING_PLUGIN_VERSION
// 15 seconds without verification is the max we allow
const boost::posix_time::time_duration Watermark::MaxDurationBetweenVerifications = boost::posix_time::time_duration(0, 0, 15, 0);

static const char watermarkHTML[] = "<a href='http://site.zigfu.com/main/watermark'><img src='data:image/png;base64,"
	"iVBORw0KGgoAAAANSUhEUgAAAG8AAABICAMAAADPslvcAAAAAXNSR0IArs4c6QAAAARnQU1BAACx"
"jwv8YQUAAAMAUExURQEBAgUGCAUICggKDQAFEwMMEwINGgsNEQgOHAUQFgQRGw0QFQwSGhIPEhAR"
"ExEVHBUYHRsUExwWGhoZHgAOIwERIwETLAcYKAwUIg0YIw0ZKhIWIRUZIhQcKhgcJRkeKRMeMBYg"
"LBwgJRwhLRQhNRUlOB4kMRslOB8oOCMYGjYdFiMdIjIcIDsnGyMkKiAmMyAnOCIoNiUrOyktNCgu"
"PSwwNysxPTAmIjYkKzMpJDErKTgjIzwlKzsqIj4qKTMvMTE1PCQvQCgvQCoyQy42SDE2QzI4RTU7"
"STg+TD5CTDlBUV4fHUouGFYqGFQyG0UqJEYzJ1kmJVU3JVI4MmgsHGk2GnY5GWQkJGg4JXgoJXg0"
"ImpBGXtCGGtEI3VGI3ZQK0BETkZJTUlJTUFGUUNJVUZMWUhMVUhOXEtQXFFUW0tSYVFWY1RZY1Na"
"aFteZFleaF1gZV1hal9kcGFhY2FlbWlqbWFmcGRpdGZseGpuc2huem1wd25yfXF0fXl6fHJ2gHV5"
"hHd8iHl9hXh9iH2Ahn2BioU7F5E8HYUtKoo7J4A/PZkuKpU1KKQ3JrY0LYhLDohHFY1SFpdIF5dT"
"GIdIIJlBIqdKGa9bDqtWE7dNGLNeDbhZFq9DKbdBJa9hDrZhC8g3L8U5MNM/L9k+MOE8NMVVFthZ"
"GMlCKtVEK9pCMNRRIMZrCstmE9RtCtpiE9hxCehcHPVeHehLKu5BNeZSJvVMLPVDNfdUJuJtCell"
"FeV4BvpoFv15Cf1xEP9hIMJmYflWTfhbUvJlXeduZ/Zxaf6DAfSBeoCBhIGEjIOHkYaKkYyOlI2Q"
"lY2RmZSVlpGUm5WYnpOXoJaZoZqdo52gpp6hqL2cm6Wmp6GkrKaorqeqsKmssq6wtbKztbK0ura5"
"v7i5ury9wL7AxfeKhPCuqfiwq8bGx8XGycbIy8nKzczO0M7Q09LT1dXW2drb3dzd4N/g4eLj5Obm"
"6Ofo6erq6+7v8O/w8P3v7vPz9Pb3+Pf4+P3+/gAAAAAAAAAAAAAAALn9vVsAAAEAdFJOU///////"
"////////////////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////////////////"
"////////////////////////////////////////////////////////////////////////////"
"/////////////////////////////wBT9wclAAAACXBIWXMAAA7DAAAOwwHHb6hkAAAAGnRFWHRT"
"b2Z0d2FyZQBQYWludC5ORVQgdjMuNS4xMDD0cqEAAA3USURBVFiFrZh9dFP1GccrtQk2l9xqB4bk"
"vtBevFAdN+decuLZjjjbSxtAXgqTAFqVSVljQZCjFnXFFxSYW4sdaAerzsOOgE7Fl0qymqR7sw5x"
"6KQyqwXmdANhCKvWrXL/YM/z+92b3Ju2Us/Zt0nacmw+fp+33/NL3rlTR3t7/n86RPXeEOp+Z1/3"
"ubxzh95986Hz6kHUA/g1rO53aN0gNTbeE9PO5XVvuPe2+pEoVl9fV18Xq4/FYnXL65YPpVqnllla"
"unQmakZIuCbvwYa6SGT6+aVP13Xyouv0x6zC8AiHiDSnVJuCwaBYJOQ13jZruv2PR6IKXS/XK8pz"
"393OCFqSqSSUyBbn3Vs/a7o+/NfQClVUgJnyUIXmZKqa3Y6dJ1NeEeF9DWwIYkjTKQ1Z5TaimsU5"
"aZIpwlsxjL9haBUQTC1UAc9QudPdYJqFC5o8wYrnkLihwhkOIU0v1zR9UNq0YWhZCWY8RxhICCP4"
"QlIoh6Zqg2iDYHbeSAoljJGs0MCfTsMYcgKHLhM7UeSQt3KWPv1rQVl75cRYeBBsmJp0mhNFcSh/"
"Q0YyXIHFGApV5JTIkFUyFE0k4ljWmT+LNgmlZY3RZ3k5mit3mEOcJvMywfGiLXF2omjyvE5/lrfa"
"3o+PH//b3VqmJuGNK9Df0PNKXnZ0hgyg8Htr+QzNbk6gQIFjWNJ/Zv4sonab8dnRj43uSbTfoCZ1"
"0gWDI0mB4mrjVnAmVQ38JUBxOd4IThAE019uzoB3t0/vOzgJ/nSirJXLmmY+eGkCr2o8SIRfeJ7j"
"VZnjR8eMGOH177/Q5+PkACdJnM8WSUFAGvBY059z5JevOHvvpXrfu9qBnt7eQ2u1/Wv1g7Hag5HI"
"wQ97PzhUH36np/fDA/Ka7g96e94Kxbp7ev6OPFmq6jvV09PdwG9oEPl7HuZEK22UyPEc52Ez+bNZ"
"1FZ89V7DA18e0I3+k/8yjupf9EaMB9Yay1cap4+fMvZVGV+cPNnLHzJOHO87W7XB+OzEaeDJsljZ"
"9/mJk8aRwOeHucDH/+ZsMPSGTw/xl9sBWmzA+MroX68NvMlPOH04fKYnYqwD3gqjQV7Wv6/SWOfj"
"eflQX0je0F+1oT/sipk8jOfxo4HTH3CBo6ecNA4eXMBr+XPg+LqzjTJ8rxrYL2unD+uE12DUrjhr"
"gNZXGutFTZUO4i8D+sP/mRlYafH+7JM+PRw43cOMPnKKEyhORJqIOI6l8czByXLMaITq0CID+3nt"
"zOHQmR7daESesb42FgvPMB6Sgqrc3R+rq1sm7vsy4gMeD6UBPEY8Djyjr++rE5zljecEQhuap0pq"
"cMVXjTKMksjAgWDo9OGKMz0zMH/AW81Dm81Ef0HxUB+2Gffml0vFVUaMkyW+sv8dLnjiMLfyR+sa"
"T53kOCuSJg147kE8VQJjK4z75XL0hyHr1U4bRLUx8q1RBx7sIt34S19oPfnHOvQX7if/fSB2+6qV"
"JzGeWJVZ2lD+EKdrkTWzYJho+uq1axvurNdWNDSsgUdYh9eGhhna6ghOk1lrQKvUqtXkG7aaElsF"
"msl9NDAw0N8bICViow3hTyM4SKEG4YT+BonhkFwilUAvaSo2Oi8H4YHLFjQUz0liAEV7OxBgfAzk"
"jbw58WbDFRWxrMebd3eWV6FJKsxiHbeSrKrMOabSURwM0oNAHuIcyDa3yAlOb0VsIFBE/c0waSFd"
"lmH+wwngGMuREJwAwey7idKg07uETDhndzsSx7EBmGUs6874g02B1opONxPLoRqOaKoMEDxgMI4A"
"EaGEs4dACS9dNW/+1VeJft7qN4Kze2ORFmDR3931EcRB7kg0K3IPnIiONDmYFVLUzJHDX/3I7mfa"
"4+3P7mi6ihNpvzmrJBBAIpvxNwPXVzCmylquwF0Y3jNIrWVOb0lSVEorkR55JbE3sXfv3ng83r6I"
"Fx39RiPJZuSx/FWgPymo4b5s318rq1S0JuOhKjt2halm8p5MENjedgTG54/PoXEsR2KZ9bc2FoGz"
"FPaSQfZULRKGXRVG103PP//8NtWxmasqCWYTwQEwAfbi8abxThhrh6E/L/WHuznYcx7e4SoNzMFi"
"MrktBVo8UbYd3spU5CmvJgCY2N3U1LwZDTYHnN4yicvyqD+oR6gWOy4YjqhmmciE1zLBsZmE0N5i"
"xO1dIhQW+orn/SoeXzLWbs7hDeRlkFcfIWuQLNu9qcHwjKCFI/7SLX57ZwNPFP3XAy/xJMNgFRb+"
"OL6jjB0mkkhjGU8+8OoipAey4SRbUChi4aBGbkJ/C3k5u3WJSjjDazYZVyz6NhMoog2XG0nEeWg8"
"LZ5qw026bPokfsJEKghidfWCBaUENIGXFIn3+2XVz/MllGea8hUWBljiLeBjCgebAxz1h6tzkPqj"
"O96k6ptvvuFGS9UTq7fWzB7Hw4zhSxY81trW9sRNC75fvfjxeZcib2/zeKzK8Vc2LZnH0CopXvLI"
"fIHJocFwcVN/CJKJP7pQTr7xtS5TKfjao2zrSqW2grmJV7cl00miVzsSyccJL/HKbtCzzTugPpsC"
"mDdmEfy4o4zJ0hiIJirjD3iZe4D8PQeu6wVlG+ZvAc9P6zBpqM7OprHEH44XbPZ2bHgGwkl48Z8y"
"GW8mzYonLRfVWs8nL+jqsgPb/IRX44e+SFv+EsmXO7cg7xXa8Hvbn0HgEoAUjV3UDj9uZhmkscQb"
"Y+NVYSAlOXPLkRfYaPCo8W/rSqdSj122AFjpZMfOtl0vITCxyUXiSeZLfHMzOlyCdQL+2p+Ob3Yz"
"7BjgeBmT5nET3vIIps3kQfmrk4M319yAWvgUMlPTTH9ja9LpdHLPFKZwNLsRgTSeiVeeATWXNcXb"
"TV4hiedmIIFDxoSNdrupv+WVGEg4BMwbHAzmcv+lfr9/3HdeRH+/4Pg29FfzrRrwl2plYH0YuxBw"
"nZsKaT+UlZVdwhRQfx67Py/r9ngsb17qr2F5FeYNedaJo4Z5HFaIA39TSsJkntWMq0mnkskWH5w4"
"YxcnIX+bXFGcns0MUTPJ30XEXzvx5zHzxrjBG9BMXqXJyxxwYZz9/HefIxmsKVFFEs/o2ChUS3qL"
"D05TXzSZ6ATeEuIPw8YWUn/Iu8j054E+IN48lGbxSOKkzHEqhVRRmlD9IsH9XApy3LYUxnNcNJVK"
"pneyPm78+CeoP3O+oNAf8Ao8rNf0R2PJUJJ7NL6M8ubdWVupYZWgQfPACSuiv5r24FOKBBMR+x3q"
"ZS72QnrLnCvnt5D6jNp5lj/oN8sf9YUvo91Zf7WVJHF0ayC8kJzFyTChaDxbxip7UqT7Ojo6sd9f"
"KjPzR44CM38FMLkuyvrDGWbTBcQfLRRRsk7vEJ91h3srT86jKPCy4wXsbXSZ9eku4jL+IJ7uAstf"
"rvKz/uSg+YkCPEPKCxRXLONVSuRp/7laIH3pDHITy5g8DysQHs2fhym8lvSfGc9MMC1/Ot2cwSAl"
"ipO3UlwRL/k4vKVvI/6KwF66o4OO6+1zoCoLo5TnEYqwH6g/xjO8P1Iv19DEBSXR3JarCe7F2dOq"
"p0yZN23eldRf9IoO8NexcG40Gp0zZbTLBZ6ut3gCW9CE+dtRBqYK6HwpGIanWzulyeNb6PB8jczP"
"VOq1aZRX/AIGs2MXaOf21ugUV4bnLhXIrey/b8Sf/i29vH3ye/rdmOpWDGNVlremNhyka5AsCYTo"
"f8p2POAAnWvmD46HbPqSL0UZVyaegnGs7Nn4P40/xF83PsHAYjzvMm5Byh2GccTmb9k11sehCo2o"
"/7ksD9T1WvGWLsyfa3bSUZ+dUdd1tD49SrHxKUTxdeONp4EHycPzyOJ91P++oeRn/C0LB7NrENyk"
"JP8vu+wCY7N/g/54afGedMYhAH99Cfsk4bEKaxzzjln0D+N31B/hjaI8xXj7VjOgJi+75ikI5Kf9"
"DPQoPB5tAc31ML7qrW1t0zhOqVnY0trauq11Oz0AZxdcfF3z7jkFpaUemqy/Nu/4I/3pJ5uvNf3B"
"a7FxxOEvu1VKcJGSuJLxhcx4ninEsX8RA5cTHtY7hi3emVrogv6CaXLJrs5kIjHH5YEiLAB7HuMY"
"LHv3GG+NucW4yz0K5B6VT3nHjKJ8COhgf+QuiUBOKmUDpaVMgIP9TlKVUoEsk4ULU+k2Fzl72MKd"
"nS8nknNIR3sVwQM8OE4ZeP0B8Nz4f5FP/SnG+/nuVcaqfIu3NGy/CiOQExSODQq42ilTlVLczPFS"
"xWyD428jHNiu0a45ENDOxOUFAOQA53ajP7cXChH9mSK8u2h8j7gdPNvnaghUBEByAWGqKlnrOagt"
"nUwlO1o3Rjduh/QlErvK4PalII74c7vvNG4fxDtGQnmMVCjybl8akhxXbwE+RoenwrJKWBFs10WY"
"n+RAIssgVmi0gC0uZr3k7CYmBv7kBh7R2yZPMT7CSN5HKpT6C0mOmz6HMS2FiCqhUnJdtG4CTBm0"
"Q6YFE52txARkDV/Oq3zT36qZU6UMzLx6gz2hKDhVCDgvOcyUNqv7wN/GMQVkLSHAkSnLy/2oC/Iy"
"Vci9UzEu75wtO/cAbtf2jZcXEHMjsebgrZypSuTzXt5x9cYsDrpTsZ6CAu/FKPeFdPp7vgmO8H4I"
"RZHzMR79yIR10rx4g4Pl1U26i2rEgTRx+Rfk583ED0gC9k8tnBhrXT3ve41EFwh559iv/9OcX0cR"
"XUBevqny8vLuyzt3D+tF0devE/rM/OQdM4bsl2Pc+DUyebx3nPsfMfD2EaHZ/MoAAAAASUVORK5C"
"YII=" // end of base64 encoded image
"' onload='(function(a){var b=new XMLHttpRequest;b.open(\"GET\",\"http://site.zigfu.com/main/plugin?v=" FBSTRING_PLUGIN_VERSION "\",true);b.onreadystatechange=function(){if(b.readyState==4&&b.status==200){var c=document.createElement(\"div\");c.innerHTML=b.responseText;a.appendChild(c);}};b.send()}(this.parentElement.parentElement))'></a>";
static const char watermarkStyle[] = "position:fixed;display:block;left:10px;bottom:10px;z-index:65535;";
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
"    var item = document.elementFromPoint(wmX, wmY);"
"    if (item != wm && (item.parentNode != wm) && (item.parentNode.parentNode != wm)) {"
//TODO: compile log only in debug mode
#ifdef _DEBUG
"        console.log('invalidating because invisible'); console.log(wm); console.log(document.elementFromPoint(wmX, wmY));"
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

