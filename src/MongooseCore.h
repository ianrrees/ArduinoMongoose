#ifndef MongooseCore_h
#define MongooseCore_h

#ifdef ARDUINO
#include <Arduino.h>
#include <IPAddress.h>
#endif // ARDUINO

#include "mongoose/mongoose.h"

#ifndef ARDUINO_MONGOOSE_DEFAULT_ROOT_CA
#define ARDUINO_MONGOOSE_DEFAULT_ROOT_CA ""
#endif

class MongooseCore
{
  private:
#if MG_ENABLE_SSL
    const char *_rootCa;
#endif
#ifdef ARDUINO
    String _nameserver;
#endif // ARDUINO
    struct mg_mgr mgr;

  public:
    MongooseCore();
    void begin();
    void end();
    void poll(int timeout_ms);

    struct mg_mgr *getMgr();
    void getDefaultOpts(struct mg_connect_opts *opts, bool secure = false);

    void ipConfigChanged();

#if MG_ENABLE_SSL
    void setRootCa(const char *rootCa) {
      _rootCa = rootCa;
    }
#endif

};

extern MongooseCore Mongoose;

#endif // MongooseCore_h
