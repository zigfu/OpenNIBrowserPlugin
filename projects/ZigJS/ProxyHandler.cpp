/*
*  ProxyHandler.cpp
*  fbplugin
*
* Copyright Facebook, inc
*/

#include "HTTPClient/HTTPRequest.h"
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include "logging.h"
#include <set>

#include "ProxyHandler.h"

using namespace boost::algorithm;
using std::string;

using namespace HTTP;

static const char* target_prefix = "http://motionos.com";

static std::set<std::string> proxy_whitelist = boost::assign::list_of
    ("/test/")
    ("/securearea/")
    ;

bool ProxyHandler::requiresVerifiedURI() const {
    return false;
}

HTTPResponseData* ProxyHandler::handleRequest(const HTTPRequestData& req) {
    bool valid = false;
    for (std::set<std::string>::iterator it = proxy_whitelist.begin();
        it != proxy_whitelist.end(); it++) {
        if (starts_with(req.uri.path, *it)) {
            valid = true;
            break;
        }
    }
    if (!valid) return NULL;
    //if (proxy_whitelist.find(req.uri.path) == proxy_whitelist.end()) return NULL;

#ifndef NDEBUG
    FBLOG_INFO("ProxyHandler: Starting request for \"%s\"\n", req.uri.path.c_str());
#endif

    FB::URI target = string(target_prefix) + req.uri.path;
    target.query_data = req.uri.query_data;

    boost::scoped_ptr<HTTPRequest> http_req(HTTPRequest::create());
    boost::shared_ptr<HTTPRequestData> http_reqdata(new HTTPRequestData(target));
    http_req->startRequest(http_reqdata);
    http_req->awaitCompletion();

    HTTPResponseData* our_resp = new HTTPResponseData;
    HTTP::Status final_status = http_req->getStatus();

    boost::shared_ptr<HTTPResponseData> resp;
    if (final_status.state == HTTP::Status::COMPLETE && (resp = http_req->getResponse()) && resp->code == 200) { 
        our_resp->code = 200;
        our_resp->data.swap(resp->data); // take ownership of remote response data

        FBLOG_DEBUG("ProxyHandler", "ProxyHandler: response received. " << our_resp->coalesceBlocks()->size() << " bytes.\n");
        FBLOG_DEBUG("ProxyHandler", "Received headers (" << resp->headers.size() << "):\n");
#ifndef NDEBUG
        for (std::multimap<std::string, std::string>::const_iterator mt = resp->headers.begin(); mt != resp->headers.end(); ++mt) {
            FBLOG_DEBUG("ProxyHandler", "\"" << mt->first.c_str() << "\": \"" << mt->second.c_str() << "\"\n");
        }
#endif
        // Maintain Content-Type header
        std::multimap<std::string, std::string>::const_iterator mt = resp->headers.find("Content-Type");
        if (mt != resp->headers.end()) {
            our_resp->headers.insert(std::make_pair("Content-Type", mt->second));
        }
		//TODO: potential security risk - review this!!!
		our_resp->headers.insert(std::make_pair("Access-Control-Allow-Origin", "*"));
    } else {
        if (resp) {
            our_resp->code = resp->code;
            our_resp->data.swap(resp->data);
        } else {
            our_resp->code = 500;
            our_resp->addDatablock(new HTTPStringDatablock(final_status.last_error));
        }

        std::map<std::string, std::string>::const_iterator inst = req.uri.query_data.find("__instanceid");
        if (inst != req.uri.query_data.end()) {
            try {
                int32_t instanceID = boost::lexical_cast<int32_t>(inst->second);
                boost::mutex::scoped_lock _l(callback_lock);
                std::map<int32_t, error_callback_t>::iterator it = error_callbacks.find(instanceID);
                if (it != error_callbacks.end()) (it->second)(target, final_status.last_error);
            } catch (...) {}
        }
    }
    return our_resp;
}

void ProxyHandler::registerErrorCallback(int32_t instanceID, error_callback_t callback) {
    boost::mutex::scoped_lock _l(callback_lock);
    error_callbacks[instanceID] = callback;
}

void ProxyHandler::unregisterErrorCallback(int32_t instanceID) {
    boost::mutex::scoped_lock _l(callback_lock);
    std::map<int32_t, error_callback_t>::iterator it = error_callbacks.find(instanceID);
    if (it != error_callbacks.end()) error_callbacks.erase(it);
}
