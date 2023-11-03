#ifndef WEB_WRAPPER_H
#define WEB_WRAPPER_H

#include <string>

#include <curl/curl.h>

class WebWrapper
{
public:
    enum ResultCode
    {
        success = 0,

        curlInitError = 2,
        curlGetResponseError
    };

    explicit WebWrapper();

    ~WebWrapper();

    ResultCode getResponse(std::string url, std::string *response);

    ResultCode getStatus();

private:
    ResultCode initialize();

    ResultCode m_status;

    CURL *m_curl;

protected:
};

WebWrapper *WebWrapperOpen();

void WebWrapperClose(WebWrapper *client);

std::string WebWrapper_GetError();

#endif // !WEB_WRAPPER_H

