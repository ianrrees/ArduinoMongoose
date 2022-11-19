#if defined(ENABLE_DEBUG) && !defined(ENABLE_DEBUG_MONGOOSE_HTTP_CLIENT)
#undef ENABLE_DEBUG
#endif

#ifdef ARDUINO
#include <Arduino.h>
#else
#define utoa(i, buf, base) sprintf(buf, "%u", i)
#endif

#include <MicroDebug.h>

#include "MongooseCore.h"
#include "MongooseHttpClient.h"

MongooseHttpClient::MongooseHttpClient()
{

}

MongooseHttpClient::~MongooseHttpClient()
{

}

/*static*/ void MongooseHttpClient::eventHandler(struct mg_connection *nc, int ev, void *p, void *u)
{
  MongooseHttpClientRequest *request = (MongooseHttpClientRequest *)u;
  request->_client ->eventHandler(nc, request, ev, p);
}

void MongooseHttpClient::eventHandler(struct mg_connection *nc, MongooseHttpClientRequest *request, int ev, void *p)
{
  if (ev != MG_EV_POLL) {
    DBUGF("%s %p: %s(%d)", __PRETTY_FUNCTION__, nc, mg_event_to_cstr(ev), ev);
  }

  switch (ev)
  {
    case MG_EV_ERROR:
      DBUGF("Mongoose error %s", (const char *)p);
      break;

    case MG_EV_CONNECT: {
      const char *extra_headers = request->_extraHeaders;
      if (!extra_headers) {
        extra_headers = "";
      }

      mg_str host = mg_url_host(request->_uri);

      if (mg_url_is_ssl(request->_uri)) {
        mg_tls_opts opts = {.ca = Mongoose.getRootCa(), .srvname = host};
        mg_tls_init(nc, &opts);
      }

      int content_length = request->_body ? strlen((const char *)request->_body) : 0;
      mg_printf(nc,
                "%s %s HTTP/1.0\r\n"
                "Host: %.*s\r\n"
                "Content-Type: octet-stream\r\n"
                "Content-Length: %d\r\n"
                "%s\r\n",
                request->_body ? "POST" : "GET", mg_url_uri(request->_uri),
                (int) host.len, host.ptr,
                content_length,
                extra_headers);
      mg_send(nc, request->_body, content_length);

      break;
    }
    case MG_EV_READ:
    {
      const struct mg_str *rx_data = (struct mg_str *)p;
      DBUGF("MG_EV_READ, bytes = %d", rx_data->len);
      DBUGF("Buffer %p, len %d: \n%.*s",
            rx_data->ptr, rx_data->len, rx_data->len, rx_data->ptr);
      break;
    }
    case MG_EV_WRITE:
    {
      long num_bytes = *(long *)p;
      DBUGF("MG_EV_WRITE, bytes = %d", num_bytes);
      break;
    }
    case MG_EV_HTTP_MSG:
    case MG_EV_HTTP_CHUNK:
    {
      struct mg_http_message *hm = (struct mg_http_message *) p;

#if defined(ENABLE_DEBUG)
      char addr[
        39 // 8x16b hex, :-separted IPv6 address (note mg_straddr() in Mongoose
           // v7.7 has a hardcoded 30-char limit, which is a bug)
        +3 // formatting
        +5 // port
        +1 // null terminator
      ] = {'\0'};

      mg_straddr(&nc->rem, addr, sizeof(addr));

      DBUGF("HTTP %s from %s, body %zu @ %p",
        MG_EV_HTTP_MSG == ev ? "reply" : "chunk",
        addr, hm->body.len, hm->body.ptr);
#endif // #if defined(ENABLE_DEBUG)

      MongooseHttpClientResponse response(hm);
      if(MG_EV_HTTP_CHUNK == ev)
      {
        if(request->_onBody) {
          request->_onBody(&response);
          mg_http_delete_chunk(nc, hm);
        }
      } else {
        if(request->_onResponse) {
          request->_onResponse(&response);
        }
        nc->is_closing = 1; // Tell mongoose to close this connection
      }

      break;
    }

    case MG_EV_CLOSE: {
      DBUGF("Connection %p closed", nc);
      if(request->_onClose) {
        request->_onClose();
      }
      delete request;
      break;
    }
  }
}

void MongooseHttpClient::get(const char *uri, MongooseHttpResponseHandler onResponse, MongooseHttpCloseHandler onClose)
{
  MongooseHttpClientRequest *request = beginRequest(uri);
  request->setMethod(HTTP_GET);
  if(NULL != onResponse) {
    request->onResponse(onResponse);
  }
  if(NULL != onClose) {
    request->onClose(onClose);
  }
  send(request);
}

void MongooseHttpClient::post(const char* uri, const char *contentType, const char *body, MongooseHttpResponseHandler onResponse, MongooseHttpCloseHandler onClose)
{
  MongooseHttpClientRequest *request = beginRequest(uri);
  request->setMethod(HTTP_POST);
  request->setContentType(contentType);
  request->setContent(body);
  if(NULL != onResponse) {
    request->onResponse(onResponse);
  }
  if(NULL != onClose) {
    request->onClose(onClose);
  }
  send(request);
}

MongooseHttpClientRequest *MongooseHttpClient::beginRequest(const char *uri)
{
  return new MongooseHttpClientRequest(this, uri);
}

void MongooseHttpClient::send(MongooseHttpClientRequest *request)
{
  // Pending https://github.com/jeremypoulter/ArduinoMongoose/issues/22
  if (request->_client != this) {
    DBUGF("HTTP request's associated client doesn't match sending client\n");
    return;
  }

  if (mg_url_is_ssl(request->_uri)) {
    auto root_ca = Mongoose.getRootCa();
    if (!root_ca) {
      DBUGF("Request is to an SSL-enabled URL, but SSL support isn't enabled\n");
      return;
    } else if (strlen(root_ca) == 0 || root_ca[0] != '-') {
      DBUGF("Request is to an SSL-enabled URL, but root CA isn't set/valid\n");
      return;
    }
  }

  mg_connection *nc = mg_http_connect(Mongoose.getMgr(), request->_uri, eventHandler, request);

  if(nc) {
    request->_nc = nc;
  } else {
    DBUGF("Failed to connect to %s", request->_uri);
  }
}

MongooseHttpClientRequest::MongooseHttpClientRequest(MongooseHttpClient *client, const char *uri) :
  _client(client),
  _onResponse(NULL),
  _onBody(NULL),
  _onClose(NULL),
  _uri(uri),
  _method(HTTP_GET),
  _contentType("application/x-www-form-urlencoded"),
  _contentLength(-1),
  _body(NULL),
  _extraHeaders(NULL)
{

}


MongooseHttpClientRequest::~MongooseHttpClientRequest()
{
  if (_extraHeaders) {
    free(_extraHeaders);
    _extraHeaders = NULL;
  }
}

MongooseHttpClientRequest *MongooseHttpClientRequest::setContent(const uint8_t *content, size_t len)
{
  setContentLength(len);
  _body = content;
  return this;
}

bool MongooseHttpClientRequest::addHeader(const char *name, size_t nameLength, const char *value, size_t valueLength)
{
  size_t startLen = _extraHeaders ? strlen(_extraHeaders) : 0;
  size_t newLen = sizeof(": \r\n");
  newLen += nameLength;
  newLen += valueLength;
  size_t len = startLen + newLen;

  char * newBuffer = (char *)realloc(_extraHeaders, len);
  if(newBuffer)
  {
    snprintf(newBuffer + startLen, newLen, "%.*s: %.*s\r\n", (int)nameLength, name, (int)valueLength, value);
    _extraHeaders = newBuffer;
    return true;
  }

  return false;
}

void MongooseHttpClientRequest::abort()
{
  if (_nc) {
    _nc->is_closing = 1; // Tell mongoose to close this connection
  }
}

size_t MongooseHttpClientResponse::contentLength() {
  MongooseString content_length = headers("Content-Length");
  if(content_length != NULL) {
    return atoll(content_length.c_str());
  }
  return _msg->body.len;
}
