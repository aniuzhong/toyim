#include "redis.h"

#include <muduo/base/Logging.h>

Redis::Redis() : publish_ctx_(NULL), subscribe_ctx_(NULL) {}

Redis::~Redis() {
  if (publish_ctx_ != NULL) {
    redisFree(publish_ctx_);
  }

  if (subscribe_ctx_ != NULL) {
    redisFree(subscribe_ctx_);
  }
}

bool Redis::Connect() {
  publish_ctx_ = redisConnect("127.0.0.1", 6379);
  if (NULL == publish_ctx_) {
    LOG_FATAL << "Connect to Redis failed.";
    return false;
  }

  subscribe_ctx_ = redisConnect("127.0.0.1", 6379);
  if (NULL == subscribe_ctx_) {
    LOG_FATAL << "Connect to Redis failed.";
    return false;
  }

  // FIXME: Join.
  std::thread t([&]() { observer_channel_message(); });
  t.detach();

  LOG_INFO << "Connect to redis-server successfully.";

  return true;
}

bool Redis::Publish(int channel, const std::string &message) {
  redisReply *reply = static_cast<redisReply *>(
      redisCommand(publish_ctx_, "PUBLISH %d %s", channel, message.c_str()));
  if (NULL == reply) {
    LOG_ERROR << "Publish failed.";
    return false;
  }
  freeReplyObject(reply);
  return true;
}

bool Redis::Subscribe(int channel) {
  if (REDIS_ERR ==
      redisAppendCommand(subscribe_ctx_, "SUBSCRIBE %d", channel)) {
    LOG_ERROR << "Subscribe failed.";
    return false;
  }
  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(subscribe_ctx_, &done)) {
      LOG_ERROR << "redisBufferWrite() failed.";
      return false;
    }
  }
  return true;
}

bool Redis::Unsubscribe(int channel) {
  if (REDIS_ERR ==
      redisAppendCommand(subscribe_ctx_, "UNSUBSCRIBE %d", channel)) {
    LOG_ERROR << "Unsubscribe failed.";
    return false;
  }
  int done = 0;
  while (!done) {
    if (REDIS_ERR == redisBufferWrite(subscribe_ctx_, &done)) {
      LOG_ERROR << "redisBufferWrite() failed.";
      return false;
    }
  }
  return true;
}

void Redis::observer_channel_message() {
  redisReply *reply = NULL;
  while (REDIS_OK ==
         redisGetReply(subscribe_ctx_, reinterpret_cast<void **>(&reply))) {
    if (reply != NULL && reply->element[2] != NULL &&
        reply->element[2]->str != NULL) {
      _notify_message_handler(atoi(reply->element[1]->str),
                              reply->element[2]->str);
    }

    freeReplyObject(reply);
  }
  LOG_INFO << "observer_channel_message quit";
}

void Redis::init_notify_handler(std::function<void(int, std::string)> fn) {
  _notify_message_handler = fn;
}