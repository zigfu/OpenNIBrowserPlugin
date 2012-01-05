#pragma once
#include "HTTPService/HTTPHandler.h"
#include "APITypes.h"
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <map>

class ProxyHandler : public HTTP::HTTPHandler {
public:
    virtual ~ProxyHandler() { }
    virtual bool requiresVerifiedURI() const;
    virtual HTTP::HTTPResponseData* handleRequest(const HTTP::HTTPRequestData& req);

    typedef boost::function<void(const FB::URI&, const std::string&)> error_callback_t;

    void registerErrorCallback(int32_t instanceID, error_callback_t callback);
    void unregisterErrorCallback(int32_t instanceID);

protected:
    std::map<int32_t, error_callback_t> error_callbacks;
    boost::mutex callback_lock;
};