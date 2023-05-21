# ArduinoMongoose

Work-in-progress branch, updating Mongoose library to v7.

Notes on work, since I seem to be taking some rather long breaks and forgetting
how this hangs together:
  - Tracker item: https://github.com/jeremypoulter/ArduinoMongoose/issues/14
  - Updating from Mongoose 6.14 to 7.7
  - Using the ArduinoMongoose examples to build against WIP codebase
  - In the platformio.ini for each example, need to change the ArduinoMongoose
    dependency to point at local copy

## TODO
  * Finish upgrade to Mongoose 7.7
    * HTTP server
    * MQTT client
  * Upgrade to Mongoose 7.8
    * Document git-fu to track upstream
  - Websocket API seems quite different, but not clear if we need to explicitly
    handle it anymore (at least in server)
	  - See mg_ws_connect() and friend

## Development notes
  * To enable debugging in one of the ArduinoMongoose source files, set
    preprocessor define for `ENABLE_DEBUG` (for example, via the platformio.ini
    if using PlatformIO), and check the top of the source file for additional
    defines that might be required, for example
    `ENABLE_DEBUG_MONGOOSE_HTTP_SERVER`.  This approach enables debugging users
    of this code, without ArduinoMongoose cluttering their output.