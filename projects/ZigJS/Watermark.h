#ifndef __Watermark_h__ 
#define __Watermark_h__ 

#include "BrowserHost.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

class Watermark {
public:
	Watermark(FB::BrowserHostPtr host);
	~Watermark();
	//bool IsOk(FB::BrowserHostPtr browser);
	bool IsOk();
	bool Test();
	FB::JSObjectPtr GetElement() const { return m_element; };
	void Invalidate();
	void Validate(int key);
private:
	int m_token;
	int m_tryCount;
	static const int MaxAttempts = 1000;

	bool m_ok; // are we okay?

	FB::BrowserHostWeakPtr m_browser;
	FB::JSObjectPtr m_element;
	boost::posix_time::ptime m_lastValidationTime;
	static const boost::posix_time::time_duration MaxDurationBetweenVerifications;

	//TODO: don't include in release builds, but include in release with debug info
	inline void Log(const std::string& str) {
#ifdef _DEBUG
		FB::BrowserHostPtr browser = m_browser.lock();
		if (browser) { browser->htmlLog(str); }
#endif
	}
};

#endif
