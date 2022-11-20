# ArduinoMongoose

Work-in-progress branch, updating Mongoose library to v7.

## TODO
  * Finish upgrade to Mongoose 7.7
    * HTTP server
    * MQTT client
  * Upgrade to Mongoose 7.8
    * Document git-fu to track upstream
  - Websocket API seems quite different
	  - See mg_ws_connect() and friend
## Development notes

  * To enable debugging in one of the ArduinoMongoose source files, set
    preprocessor define for `ENABLE_DEBUG` (for example, via the platformio.ini
    if using PlatformIO), and check the top of the source file for additional
    defines that might be required, for example
    `ENABLE_DEBUG_MONGOOSE_HTTP_SERVER`.  This approach enables debugging users
    of this code, without ArduinoMongoose cluttering their output.