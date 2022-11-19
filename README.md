# ArduinoMongoose

Work-in-progress branch, updating Mongoose library.

Renames:
  struct mbuf -> struct mg_iobuf  (and in method names too: mbuf_init() -> mg_iobuf_init() )
  http_message -> mg_http_message

TODOs:
  * Websocket API seems quite different
	- See mg_ws_connect() and friend
  * What happened to struct mg_connect_opts

[![Build Status](https://travis-ci.org/jeremypoulter/ArduinoMongoose.svg?branch=master)](https://travis-ci.org/jeremypoulter/ArduinoMongoose)

A wrapper for Mongoose to help build into Arduino framework.

## Development notes

  * To enable debugging in one of the ArduinoMongoose source files, set
    preprocessor define for `ENABLE_DEBUG` (for example, via the platformio.ini
    if using PlatformIO), and check the top of the source file for additional
    defines that might be required, for example
    `ENABLE_DEBUG_MONGOOSE_HTTP_SERVER`.  This approach enables debugging users
    of this code, without ArduinoMongoose cluttering their output.