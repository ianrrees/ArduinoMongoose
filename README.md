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
