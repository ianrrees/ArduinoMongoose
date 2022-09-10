#if defined(ENABLE_DEBUG) && !defined(ENABLE_DEBUG_MONGOOSE_MQTT_CLIENT)
#undef ENABLE_DEBUG
#endif

#ifdef ARDUINO
#include <Arduino.h>
#endif

#include <MicroDebug.h>

#include "MongooseCore.h"
#include "MongooseMqttClient.h"

MongooseMqttClient::MongooseMqttClient() :
  _client_id(NULL),
  _username(NULL),
  _password(NULL),
  _will_topic(NULL),
  _will_message(NULL),
  _will_retain(false),
  _nc(NULL),
  _connected(false),
  _reject_unauthorized(true),
  _onConnect(NULL),
  _onMessage(NULL),
  _onError(NULL)
{

}

MongooseMqttClient::~MongooseMqttClient()
{

}

void MongooseMqttClient::eventHandler(struct mg_connection *nc, int ev, void *p, void *u)
{
  MongooseMqttClient *self = (MongooseMqttClient *)u;
  self->eventHandler(nc, ev, p);
}

void MongooseMqttClient::eventHandler(struct mg_connection *nc, int ev, void *p)
{
  if (ev != MG_EV_POLL) { DBUGF("%s %p: %d", __PRETTY_FUNCTION__, nc, ev); }

  switch (ev) 
  {
    case MG_EV_CONNECT: {
      struct mg_mqtt_opts opts = {0};
      opts.client_id = mg_str_s(_client_id);
      opts.user = mg_str_s(_username);
      opts.pass = mg_str_s(_password);
      opts.will_topic = mg_str_s(_will_topic);
      opts.will_message = mg_str_s(_will_message);
      opts.will_retain = _will_retain;
      
      DBUGVAR(opts.client_id);
      DBUGVAR(opts.user);
      DBUGVAR(opts.pass);
      DBUGVAR(opts.will_topic);
      DBUGVAR(opts.will_message);
      DBUGVAR(opts.will_retain);

      mg_mqtt_login(nc, &opts);
      break;
    }

    case MG_EV_MQTT_OPEN: {
      int error_code = *(int *)p;
      if (error_code) {
        DBUGF("Got mqtt connection error: %d", error_code);
        if(_onError) {
          _onError(error_code);
          _nc = NULL;
        }
      } else {
        _connected = true;
        if(_onConnect) {
          _onConnect();
        }
      }
      break;
    }

    case MG_EV_MQTT_MSG: {
      struct mg_mqtt_message *msg = (struct mg_mqtt_message *)p;
      DBUGF("Got incoming message %.*s: %.*s", (int) msg->topic.len,
             msg->topic.ptr, (int) msg->data.len, msg->data.ptr);
      if(_onMessage) {
        _onMessage(MongooseString(msg->topic), MongooseString(msg->data));
      }
      break;
    }

    case MG_EV_CLOSE: {
      DBUGF("Connection %p closed", nc);
      _nc = NULL;
      _connected = false;
      break;
    }
  }
}

bool MongooseMqttClient::connect(MongooseMqttProtocol protocol, const char *server, const char *client_id, MongooseMqttConnectionHandler onConnect)
{
  if(NULL == _nc) 
  {
    struct mg_mqtt_opts opts = {0};
    opts.clean = true;
    // opts.will_qos = 
    opts.will_topic = mg_str_s(_will_topic);
    opts.will_message = mg_str_s(_will_message);
    opts.will_retain = _will_retain;

    DBUGF("Trying to connect to %s", server);
    _onConnect = onConnect;
    _client_id = client_id;
    _nc = mg_mqtt_connect(Mongoose.getMgr(), server, &opts, eventHandler, this);
    if(_nc) {
      return true;
    }

    // TODO
    // DBUGF("Failed to connect to %s: %s", server, err);
  }
  return false;
}

bool MongooseMqttClient::subscribe(const char *topic, int qos)
{
  if(connected())
  {
    mg_mqtt_sub(_nc, mg_str_s(topic), qos);

    return true;
  }
  return false;
}

bool MongooseMqttClient::publish(const char *topic, mg_str payload, bool retain, int qos)
{
  if(connected()) {
    mg_mqtt_pub(_nc, mg_str_s(topic), payload, qos, retain);
    return true;
  }

  return false;
}

bool MongooseMqttClient::disconnect()
{
  if(connected()) {
    mg_mqtt_disconnect(_nc);
    _nc->is_draining = 1;
    return true;
  }

  return false;
}
