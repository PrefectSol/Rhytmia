#include "WebWrapper.h"

WebWrapper::ResultCode *errorCode = NULL;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output) 
{
    size_t totalSize = size * nmemb;
    output->append((char *)contents, totalSize);

    return totalSize;
}

WebWrapper::WebWrapper()
    : m_curl(NULL)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    errorCode = &m_status;
    m_status = initialize();
}

WebWrapper::~WebWrapper()
{
    curl_global_cleanup();
}

WebWrapper::ResultCode WebWrapper::initialize()
{
    m_curl = curl_easy_init();
    if (m_curl)
    {
        return ResultCode::success;
    }

    return ResultCode::curlInitError;
}

WebWrapper::ResultCode WebWrapper::getResponse(std::string url, std::string *response)
{
    CURLcode res;

    curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(m_curl);

    if (res != CURLE_OK) 
    {
        return ResultCode::curlGetResponseError;
    }

    curl_easy_cleanup(m_curl);

    return ResultCode::success;
}

WebWrapper::ResultCode WebWrapper::getStatus()
{
    return m_status;
}

WebWrapper *WebWrapperOpen()
{
    WebWrapper *client = new WebWrapper();

    if (client->getStatus() != WebWrapper::success)
    {
        return NULL;
    }

    return client;
}

void WebWrapperClose(WebWrapper *client)
{
    if (client != NULL)
    {
        delete client;
        client = nullptr;
    }
}

std::string WebWrapper_GetError()
{
    return std::string("Web Wrapper Error: ") + std::to_string(*errorCode);
}
