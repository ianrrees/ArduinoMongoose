
#include "MongooseCore.h"

#ifdef ARDUINO
#ifdef ESP32
#include <WiFi.h>
#include <ETH.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#endif // ARDUINO

#if defined(ENABLE_DEBUG)
const char * mg_event_to_cstr(int ev) {
  switch(ev) {
    case MG_EV_ERROR: return "MG_EV_ERROR";
    case MG_EV_OPEN:  return "MG_EV_OPEN";
    case MG_EV_POLL:  return "MG_EV_POLL";
    case MG_EV_RESOLVE: return "MG_EV_RESOLVE";
    case MG_EV_CONNECT: return "MG_EV_CONNECT";
    case MG_EV_ACCEPT:  return "MG_EV_ACCEPT";
    case MG_EV_READ:  return "MG_EV_READ";
    case MG_EV_WRITE: return "MG_EV_WRITE";
    case MG_EV_CLOSE: return "MG_EV_CLOSE";
    case MG_EV_HTTP_MSG:  return "MG_EV_HTTP_MSG";
    case MG_EV_HTTP_CHUNK:  return "MG_EV_HTTP_CHUNK";
    case MG_EV_WS_OPEN: return "MG_EV_WS_OPEN";
    case MG_EV_WS_MSG:  return "MG_EV_WS_MSG";
    case MG_EV_WS_CTL:  return "MG_EV_WS_CTL";
    case MG_EV_MQTT_CMD:  return "MG_EV_MQTT_CMD";
    case MG_EV_MQTT_MSG:  return "MG_EV_MQTT_MSG";
    case MG_EV_MQTT_OPEN: return "MG_EV_MQTT_OPEN";
    case MG_EV_SNTP_TIME: return "MG_EV_SNTP_TIME";
    case MG_EV_USER:  return "MG_EV_USER";
    default:  return "???";
  }
}
#endif // #if defined(ENABLE_DEBUG)

MongooseCore::MongooseCore() : 
#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
  _rootCa(ARDUINO_MONGOOSE_DEFAULT_ROOT_CA),
#endif
#ifdef ARDUINO
  _nameserver(""),
#endif
  mgr({0})
{
}

void MongooseCore::begin() 
{
  // TODO did the context pointer ever get used?
  // mg_mgr_init(&mgr, this);
  mg_mgr_init(&mgr);

  ipConfigChanged();
}

void MongooseCore::end() 
{
  mg_mgr_free(&mgr);
}

void MongooseCore::poll(int timeout_ms) 
{
  mg_mgr_poll(&mgr, timeout_ms);
}

struct mg_mgr *MongooseCore::getMgr()
{
  return &mgr;
}

void MongooseCore::ipConfigChanged() 
{
#ifdef ARDUINO
#if defined(ESP32) || defined(ESP8266)
  IPAddress dns = WiFi.dnsIP(0);
#if defined(ESP32)
  if(0 == dns) {
    dns = ETH.dnsIP(0);
  }
#endif
  _nameserver = dns.toString();
  // mg_set_nameserver(&mgr, _nameserver.c_str());
#endif
#endif // ARDUINO
}

MongooseCore Mongoose;
