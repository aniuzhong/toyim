#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/TcpClient.h>
#include <stdio.h>
#include <unistd.h>

#include <boost/noncopyable.hpp>

#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "toyim.pb.h"

using muduo::AtomicInt32;
using muduo::timeDifference;
using muduo::Timestamp;
using muduo::net::EventLoop;
using muduo::net::EventLoopThreadPool;
using muduo::net::InetAddress;
using muduo::net::TcpClient;
using muduo::net::TcpConnectionPtr;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

typedef std::shared_ptr<toyim::Answer> AnswerPtr;

int g_connections = 5000;
AtomicInt32 g_alive_connections;
AtomicInt32 g_messages_received;
Timestamp g_start_time;
std::vector<Timestamp> g_receiveTime;
EventLoop* g_loop;
google::protobuf::Message* g_message_to_send;

class ImClient : public boost::noncopyable {
 public:
  ImClient(EventLoop* loop, const InetAddress& server_addr)
      : loop_(loop),
        client_(loop, server_addr, "LoginLoadTestCient"),
        dispatcher_(std::bind(&ImClient::onUnknownMessage, this, _1, _2, _3)),
        codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_,
                         _1, _2, _3)) {
    dispatcher_.registerMessageCallback<toyim::Answer>(
        std::bind(&ImClient::onAnswer, this, _1, _2, _3));
    client_.setConnectionCallback(std::bind(&ImClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
  }

  void Connect() { client_.connect(); }
  void Disconnect() { client_.disconnect(); }
  Timestamp receiveTime() const { return receive_time_; }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    // LOG_INFO << conn->localAddress().toIpPort() << " -> "
    //          << conn->peerAddress().toIpPort() << " is "
    //          << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
      connection_ = conn;
      send();
    } else {
      connection_.reset();
    }
  }

  void onUnknownMessage(const TcpConnectionPtr& conn, const MessagePtr& message,
                        Timestamp) {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
  }

  void onAnswer(const TcpConnectionPtr& conn, const AnswerPtr& message,
                Timestamp) {
    // LOG_INFO << "OnAnswer";
    receive_time_ = loop_->pollReturnTime();
    int received = g_messages_received.incrementAndGet();
    if (received == g_connections) {
      Timestamp end_time = Timestamp::now();
      LOG_INFO << "all received " << g_connections << " in "
               << timeDifference(end_time, g_start_time);
    } else if (received % 1000 == 0) {
      LOG_DEBUG << received;
    }
  }

  void send() {
    codec_.send(connection_, *g_message_to_send);
    LOG_DEBUG << "sent";
  }

  EventLoop* loop_;
  TcpClient client_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
  TcpConnectionPtr connection_;
  Timestamp receive_time_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 2) {
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress server_addr(argv[1], port);
    int threads = 16;

    EventLoop loop;
    g_loop = &loop;
    EventLoopThreadPool loop_pool(&loop, "login-loadtest");
    loop_pool.setThreadNum(threads);
    loop_pool.start();

    toyim::Login query;
    query.set_id(13);
    query.set_password("123456");
    g_message_to_send = &query;

    std::vector<std::unique_ptr<ImClient>> clients(g_connections);
    g_start_time = Timestamp::now();
    for (int i = 0; i < g_connections; ++i) {
      clients[i].reset(new ImClient(loop_pool.getNextLoop(), server_addr));
      clients[i]->Connect();
    }
    loop.loop();
  } else {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}
