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

/// Debug helper
///
/// Only implemented if MongooseCore.cpp is compiled with ENABLE_DEBUG defined.
const char * mg_event_to_cstr(int ev);

#if defined(MG_ENABLE_MBEDTLS)
  #define ARDUINO_MONGOOSE_SSL_SUPPORTED() 1
#else
  #define ARDUINO_MONGOOSE_SSL_SUPPORTED() 0
#endif

class MongooseCore
{
  private:
#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
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

    /// Returns c-string of root CA for SSL, or NULL if SSL isn't enabled
    const char * getRootCa() const {
#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
      return _rootCa;
#else
      return NULL;
#endif // if/else ARDUINO_MONGOOSE_SSL_SUPPORTED()
    }

    void ipConfigChanged();

#if ARDUINO_MONGOOSE_SSL_SUPPORTED()
    /// Provide a root CA for SSL.  Does NOT take ownership of rootCa
    ///
    /// The memory pointed to by rootCa must continue to point at the
    /// certificate - it is /not/ copied in to RAM.  
    ///
    /// The certificate is a string, formatted like:
    /// ```
    /// const char *root_ca =
    /// "-----BEGIN CERTIFICATE-----\r\n"
    /// "MIIFNjCCBNygAwIBAgIQCH4DWYOebCLz/pcJdRheAzAKBggqhkjOPQQDAjBKMQsw\r\n"
    /// "CQYDVQQGEwJVUzEZMBcGA1UEChMQQ2xvdWRmbGFyZSwgSW5jLjEgMB4GA1UEAxMX\r\n"
    /// ...
    /// ```
    void setRootCa(const char *rootCa) {
      _rootCa = rootCa;
    }
#endif

};

extern MongooseCore Mongoose;

#endif // MongooseCore_h
