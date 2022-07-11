#include <muduo/base/CurrentThread.h>
#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>
#include <stdio.h>
#include <unistd.h>

#include <boost/noncopyable.hpp>
#include <iostream>

#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "toyim.pb.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using muduo::MutexLock;
using muduo::MutexLockGuard;
using muduo::Timestamp;
using muduo::CurrentThread::sleepUsec;
using muduo::net::EventLoop;
using muduo::net::EventLoopThread;
using muduo::net::InetAddress;
using muduo::net::TcpClient;
using muduo::net::TcpConnectionPtr;
using toyim::Answer;

typedef std::shared_ptr<toyim::Answer> AnswerPtr;

class ImClient : public boost::noncopyable {
 public:
  ImClient(EventLoop* loop, const InetAddress& server_addr)
      : client_(loop, server_addr, "ImClient"),
        dispatcher_(std::bind(&ImClient::onUnknownMessage, this, _1, _2, _3)),
        codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_,
                         _1, _2, _3)) {
    dispatcher_.registerMessageCallback<Answer>(
        std::bind(&ImClient::onAnswer, this, _1, _2, _3));
    client_.setConnectionCallback(std::bind(&ImClient::OnConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
    client_.enableRetry();
  }

  void Connect() { client_.connect(); }
  void Disconnect() { client_.disconnect(); }

  void write(const google::protobuf::Message& message) {
    MutexLockGuard lock(mutex_);
    if (connection_) {
      codec_.send(connection_, message);
    }
  }

 private:
  void OnConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected()) {
      connection_ = conn;
    } else {
      connection_.reset();
    }
  }

  void onUnknownMessage(const TcpConnectionPtr&, const MessagePtr& message,
                        Timestamp) {
    LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
  }

  void onAnswer(const TcpConnectionPtr&, const AnswerPtr& message, Timestamp) {
    LOG_INFO << "onAnswer:\n"
             << message->GetTypeName() << message->DebugString();
  }

  TcpClient client_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
  MutexLock mutex_;
  TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};

// TODO:
int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 2) {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress server_addr(argv[1], port);

    ImClient client(loopThread.startLoop(), server_addr);
    client.Connect();

    google::protobuf::Message* message_to_send;

    std::string line;
    while (std::getline(std::cin, line)) {
      if (line == "login") {
        std::getline(std::cin, line);
        toyim::Login query;
        query.set_id(std::stoi(line));
        query.set_password("123456");
        message_to_send = &query;
        client.write(*message_to_send);
        query.PrintDebugString();
      } else if (line == "groupchat") {
        std::getline(std::cin, line);
        toyim::GroupChat chat;
        chat.set_user_id(13);
        chat.set_group_id(1);
        chat.set_content(line);
        message_to_send = &chat;
        client.write(*message_to_send);
        chat.PrintDebugString();
      } else if (line == "friendchat") {
        std::getline(std::cin, line);
        toyim::FriendChat chat;
        chat.set_from(13);
        chat.set_to(15);
        chat.set_content(line);
        message_to_send = &chat;
        client.write(*message_to_send);
        chat.PrintDebugString();
      }
    }
    client.Disconnect();
    sleepUsec(1000 * 1000);
  } else {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}
