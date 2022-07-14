#include <muduo/base/Atomic.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/TcpClient.h>
#include <stdio.h>
#include <unistd.h>

#include <boost/noncopyable.hpp>

#include "orm/group_model.h"
#include "orm/user_model.h"
#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "toyim.pb.h"

using namespace muduo;
using namespace muduo::net;
using toyim::orm::Group;
using toyim::orm::GroupModel;
using toyim::orm::GroupUser;
using toyim::orm::User;
using toyim::orm::UserModel;

typedef std::shared_ptr<toyim::Answer> AnswerPtr;

int g_connections = 0;
AtomicInt32 g_aliveConnections;
AtomicInt32 g_messagesReceived;
Timestamp g_startTime;
std::vector<Timestamp> g_receiveTime;
EventLoop* g_loop;
std::function<void()> g_statistic;

class ImClient : boost::noncopyable {
 public:
  ImClient(EventLoop* loop, const InetAddress& server_addr, int num)
      : num_(num),
        loop_(loop),
        client_(loop, server_addr, "User" + std::to_string(num)),
        dispatcher_(std::bind(&ImClient::onUnknownMessage, this, _1, _2, _3)),
        codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_,
                         _1, _2, _3)) {
    dispatcher_.registerMessageCallback<toyim::Answer>(
        std::bind(&ImClient::onAnswer, this, _1, _2, _3));
    client_.setConnectionCallback(std::bind(&ImClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
  }

  void connect() { client_.connect(); }
  void disconnect() { client_.disconnect(); }

  Timestamp receiveTime() const { return receiveTime_; }

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
      connection_ = conn;
      User user = UserModel::Query(client_.name());
      LOG_INFO << "id:" << user.id();
      toyim::Login message;
      message.set_id(user.id());
      message.set_password("888888");
      codec_.send(connection_, message);

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
    // printf("<<< %s\n", message->DebugString().c_str());
    if (message->reply() == "Login successfully.") {
      if (g_aliveConnections.incrementAndGet() == g_connections) {
        LOG_INFO << "all logged in.";
        loop_->runAfter(10.0, std::bind(&ImClient::send, this));
      }
    }

    if (message->reply() == "hello") {
      receiveTime_ = loop_->pollReturnTime();
      int received = g_messagesReceived.incrementAndGet();
      if (received == g_connections-1) {
        Timestamp endTime = Timestamp::now();
        LOG_INFO << "all received " << g_connections << " in "
                 << timeDifference(endTime, g_startTime);
        g_loop->queueInLoop(g_statistic);
      } else if (received % 1000 == 0) {
        LOG_INFO << received;
      }
    }
  }

  void send() {
    User user = UserModel::Query(client_.name());
    toyim::GroupChat message;
    message.set_user_id(user.id());
    message.set_group_id(3);
    message.set_content("hello");

    g_startTime = Timestamp::now();
    codec_.send(connection_, message);
    LOG_DEBUG << "sent";
  }

  const int num_;
  EventLoop* loop_;
  TcpClient client_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
  Timestamp receiveTime_;
  TcpConnectionPtr connection_;
};

void statistic(const std::vector<std::unique_ptr<ImClient>>& clients) {
  LOG_INFO << "statistic " << clients.size();
  std::vector<double> seconds(clients.size());
  for (size_t i = 0; i < clients.size(); ++i) {
    seconds[i] = timeDifference(clients[i]->receiveTime(), g_startTime);
  }

  std::sort(seconds.begin(), seconds.end());
  for (size_t i = 0; i < clients.size();
       i += std::max(static_cast<size_t>(1), clients.size() / 20)) {
    printf("%6zd%% %.6f\n", i * 100 / clients.size(), seconds[i]);
  }
  if (clients.size() >= 100) {
    printf("%6d%% %.6f\n", 99, seconds[clients.size() - clients.size() / 100]);
  }
  printf("%6d%% %.6f\n", 100, seconds.back());
}

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 2) {
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);
    g_connections = 5000;
    int threads = 4;

    EventLoop loop;
    g_loop = &loop;
    EventLoopThreadPool loopPool(&loop, "groupchat-loadtest");
    loopPool.setThreadNum(threads);
    loopPool.start();

    g_receiveTime.reserve(g_connections);
    std::vector<std::unique_ptr<ImClient>> clients(g_connections);
    g_statistic = std::bind(statistic, std::ref(clients));

    for (int i = 0; i < g_connections; ++i) {
      clients[i].reset(new ImClient(loopPool.getNextLoop(), serverAddr, i));
      clients[i]->connect();
      usleep(200);
    }

    loop.loop();
    // client.disconnect();
  } else {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}
