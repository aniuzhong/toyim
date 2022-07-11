#include "server.h"

#include <muduo/base/Logging.h>

#include "orm/friend_model.h"
#include "orm/group_model.h"
#include "orm/offline_message_model.h"
#include "orm/user.h"
#include "orm/user_model.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using muduo::MutexLockGuard;
using muduo::Timestamp;
using muduo::net::EventLoop;
using muduo::net::InetAddress;
using muduo::net::TcpConnectionPtr;

using toyim::AddFriend;
using toyim::AddGroup;
using toyim::Answer;
using toyim::CreateGroup;
using toyim::FriendChat;
using toyim::GroupChat;
using toyim::Login;
using toyim::Logout;
using toyim::Register;
using toyim::orm::FriendModel;
using toyim::orm::Group;
using toyim::orm::GroupModel;
using toyim::orm::GroupUser;
using toyim::orm::OfflineMessageModel;
using toyim::orm::User;
using toyim::orm::UserModel;

ImServer::ImServer(EventLoop* loop, const InetAddress& listenAddr)
    : server_(loop, listenAddr, "ImServer"),
      dispatcher_(std::bind(&ImServer::OnUnknownMessage, this, _1, _2, _3)),
      codec_(std::bind(&ProtobufDispatcher::onProtobufMessage, &dispatcher_, _1,
                       _2, _3)) {
  dispatcher_.registerMessageCallback<Register>(
      std::bind(&ImServer::OnRegister, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<Login>(
      std::bind(&ImServer::OnLogin, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<Logout>(
      std::bind(&ImServer::OnLogout, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<AddFriend>(
      std::bind(&ImServer::OnAddFriend, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<CreateGroup>(
      std::bind(&ImServer::OnCreateGroup, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<AddGroup>(
      std::bind(&ImServer::OnAddGroup, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<FriendChat>(
      std::bind(&ImServer::OnFriendChat, this, _1, _2, _3));
  dispatcher_.registerMessageCallback<GroupChat>(
      std::bind(&ImServer::OnGroupChat, this, _1, _2, _3));
  server_.setConnectionCallback(std::bind(&ImServer::OnConnection, this, _1));
  server_.setMessageCallback(
      std::bind(&ProtobufCodec::onMessage, &codec_, _1, _2, _3));
}

void ImServer::SetThreadNum(int n) { server_.setThreadNum(0); }

void ImServer::Start() { server_.start(); }

void ImServer::OnConnection(const TcpConnectionPtr& conn) {
  LOG_INFO << conn->localAddress().toIpPort() << " -> "
           << conn->peerAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void ImServer::OnUnknownMessage(const TcpConnectionPtr& conn,
                                const MessagePtr& message, Timestamp) {
  LOG_INFO << "onUnknownMessage: " << message->GetTypeName();
  conn->shutdown();
}

void ImServer::OnRegister(const muduo::net::TcpConnectionPtr& conn,
                          const RegisterPtr& message, muduo::Timestamp) {
  User user = UserModel::Query(message->name());
  Answer answer;
  if (user.id() == -1) {
    user.set_name(message->name());
    user.set_password(message->password());
    UserModel::Insert(user);
    answer.set_reply("OnRegister successfully.");
  } else {
    answer.set_reply("OnRegister failed.");
  }
  codec_.send(conn, answer);
}

void ImServer::OnLogin(const TcpConnectionPtr& conn, const LoginPtr& message,
                       Timestamp) {
  LOG_INFO << "OnLogin:\n" << message->GetTypeName();
  User user = UserModel::Query(message->id());
  Answer answer;
  // Check user's password.
  if (user.password() == message->password()) {
    // Repeat logins are not allowed.
    if (user.state() == "online") {
      answer.set_reply("Failed to login. Account has been logged in.");
      codec_.send(conn, answer);
    } else {
      // login successfully, record connection info.
      {
        MutexLockGuard lock(mutex_);
        connections_[user.id()] = conn;
      }
      // subscribe on redis
      // ...
      // Update user's state.
      user.set_state("online");
      UserModel::UpdateState(user);  // FIXME: error check
      // Query offline message
      // Query friend list
      // Query group info
      answer.set_reply("Login successfully.");
      codec_.send(conn, answer);
    }
  } else {
    // Wrong password or invalid user id.
    answer.set_reply("Failed to login.");
    codec_.send(conn, answer);
  }
}

void ImServer::OnLogout(const muduo::net::TcpConnectionPtr& conn,
                        const LogoutPtr& message, muduo::Timestamp) {
  UserModel::UpdateState(User(message->id(), "", "", "offline"));
  Answer answer;
  answer.set_reply("Logout successfulily.");
  codec_.send(conn, answer);
}

void ImServer::OnAddFriend(const muduo::net::TcpConnectionPtr& conn,
                           const AddFriendPtr& message, muduo::Timestamp) {
  User user = UserModel::Query(message->friend_id());
  Answer answer;
  if (user.id() == -1) {
    answer.set_reply("Friend not exist, OnAddFriend failed.");
  } else {
    FriendModel::Insert(message->user_id(), message->friend_id());
    answer.set_reply("OnAddFriend successfulily.");
    //  FIXME: error check
  }
  codec_.send(conn, answer);
}

void ImServer::OnFriendChat(const muduo::net::TcpConnectionPtr& conn,
                            const FriendChatPtr& message, muduo::Timestamp) {
  Answer answer;
  User user = UserModel::Query(message->to());
  if (user.id() == -1) {
    answer.set_reply(
        "Failed, friend does not exist, please change FriendId and try again.");
    codec_.send(conn, answer);
  } else {
    ConnectionMap::iterator it;
    {
      MutexLockGuard lock(mutex_);
      it = connections_.find(message->to());
    }
    answer.set_reply(message->content());
    codec_.send(it->second, answer);
  }
}

void ImServer::OnCreateGroup(const muduo::net::TcpConnectionPtr& conn,
                             const CreateGroupPtr& message, muduo::Timestamp) {
  Group group(message->group_id(), message->group_name(),
              message->descriptions());
  GroupModel::CreateGroup(group);
  GroupModel::AddGroup(message->user_id(), message->group_id(), "creator");
  Answer answer;
  answer.set_reply("OnCreateGroup successfulily.");
  codec_.send(conn, answer);
}

void ImServer::OnAddGroup(const muduo::net::TcpConnectionPtr& conn,
                          const AddGroupPtr& message, muduo::Timestamp) {
  Answer answer;
  // (groupid, userid) should be composite key of groupuser.
  GroupModel::AddGroup(message->user_id(), message->group_id(), "normal");
  answer.set_reply("AddGroup successfulily.");
  codec_.send(conn, answer);
}

void ImServer::OnGroupChat(const muduo::net::TcpConnectionPtr& conn,
                           const GroupChatPtr& message, muduo::Timestamp) {
  std::vector<int> ids =
      GroupModel::QueryGroupUsers(message->user_id(), message->group_id());
  Answer answer;
  MutexLockGuard lock(mutex_);
  for (const auto& id : ids) {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
      answer.set_reply(message->content());
      codec_.send(it->second, answer);
    } else {
      // TODO: Offline message.
    }
  }
}

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid();
  if (argc > 1) {
    UserModel::ResetState();
    EventLoop loop;
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    InetAddress serverAddr(port);
    ImServer server(&loop, serverAddr);
    server.Start();
    loop.loop();
  } else {
    printf("Usage: %s port\n", argv[0]);
  }
}