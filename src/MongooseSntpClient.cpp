#if defined(ENABLE_DEBUG) && !defined(ENABLE_DEBUG_MONGOOSE_SNTP_CLIENT)
#undef ENABLE_DEBUG
#endif

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <MicroDebug.h>

#include "MongooseCore.h"
#include "MongooseSntpClient.h"

#if MG_ENABLE_SNTP

MongooseSntpClient::MongooseSntpClient() :
  _nc(NULL),
  _onTime(NULL),
  _onError(NULL)
{

}

MongooseSntpClient::~MongooseSntpClient()
{

}

/*static*/ void MongooseSntpClient::eventHandler(struct mg_connection *nc, int ev, void *p, void *u)
{
  MongooseSntpClient *self = (MongooseSntpClient *)u;
  self->eventHandler(nc, ev, p);
}

void MongooseSntpClient::eventHandler(struct mg_connection *nc, int ev, void *p)
{
  if (ev != MG_EV_POLL) { DBUGF("%s %p: %d", __PRETTY_FUNCTION__, nc, ev); }

  switch (ev) 
  {
    case MG_EV_SNTP_TIME:
      if(_onTime) {
        uint64_t milliseconds = *(uint64_t *)p;

        struct timeval time{};
        time.tv_sec = milliseconds / 1000;
        time.tv_usec = 1000 * (milliseconds - 1000 * time.tv_sec);
        
        _onTime(time);
      }
      break;

    case MG_EV_ERROR:
      if(_onError) {
        _onError(-1);
      }
      break;

    case MG_EV_CLOSE: {
      DBUGF("Connection %p closed", nc);
      _nc = NULL;
      break;
    }
  }
}

bool MongooseSntpClient::getTime(const char *server, MongooseSntpTimeHandler onTime)
{
  if(NULL == _nc) 
  {
    DBUGF("Trying to connect to %s", server);
    _onTime = onTime;

    _nc = mg_sntp_connect(Mongoose.getMgr(), server, eventHandler, this);
    if(_nc) {
      mg_sntp_request(_nc);
      return true;
    }

    DBUGF("Failed to connect to %s", server);
  }

  return false;
}

#endif // MG_ENABLE_SNTP
