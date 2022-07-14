#ifndef TOYIM_HIREDIS_REDIS_H
#define TOYIM_HIREDIS_REDIS_H

#include <hiredis/hiredis.h>

#include <functional>
#include <thread>

class Redis {
 public:
  Redis();
  ~Redis();
  bool Connect();
  bool Publish(int channel, const std::string& message);
  bool Subscribe(int channel);
  bool Unsubscribe(int channel);
  void observer_channel_message();
  void init_notify_handler(std::function<void(int, std::string)> fn);

 private:
  redisContext *publish_ctx_;
  redisContext *subscribe_ctx_;
  std::function<void(int, std::string)> _notify_message_handler;
};

#endif  // TOYIM_HIREDIS_REDIS_H
