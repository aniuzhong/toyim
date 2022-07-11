#ifndef TOYIM_IMSERVER_H
#define TOYIM_IMSERVER_H

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include <boost/noncopyable.hpp>
#include <map>

#include "protobuf/codec.h"
#include "protobuf/dispatcher.h"
#include "toyim.pb.h"

typedef std::shared_ptr<toyim::Register> RegisterPtr;
typedef std::shared_ptr<toyim::Login> LoginPtr;
typedef std::shared_ptr<toyim::Logout> LogoutPtr;
typedef std::shared_ptr<toyim::AddFriend> AddFriendPtr;
typedef std::shared_ptr<toyim::FriendChat> FriendChatPtr;
typedef std::shared_ptr<toyim::CreateGroup> CreateGroupPtr;
typedef std::shared_ptr<toyim::AddGroup> AddGroupPtr;
typedef std::shared_ptr<toyim::GroupChat> GroupChatPtr;

class ImServer : public boost::noncopyable {
 public:
  ImServer(muduo::net::EventLoop* loop,
           const muduo::net::InetAddress& listen_addr);
  void SetThreadNum(int n);
  void Start();
  void OnConnection(const muduo::net::TcpConnectionPtr& conn);
  void OnUnknownMessage(const muduo::net::TcpConnectionPtr& conn,
                        const MessagePtr& message, muduo::Timestamp);
  void OnRegister(const muduo::net::TcpConnectionPtr& conn,
                  const RegisterPtr& message, muduo::Timestamp);
  void OnLogin(const muduo::net::TcpConnectionPtr& conn,
               const LoginPtr& message, muduo::Timestamp);
  void OnLogout(const muduo::net::TcpConnectionPtr& conn,
                const LogoutPtr& message, muduo::Timestamp);
  void OnAddFriend(const muduo::net::TcpConnectionPtr& conn,
                   const AddFriendPtr& message, muduo::Timestamp);
  void OnFriendChat(const muduo::net::TcpConnectionPtr& conn,
                    const FriendChatPtr& message, muduo::Timestamp);
  void OnCreateGroup(const muduo::net::TcpConnectionPtr& conn,
                     const CreateGroupPtr& message, muduo::Timestamp);
  void OnAddGroup(const muduo::net::TcpConnectionPtr& conn,
                     const AddGroupPtr& message, muduo::Timestamp);
  void OnGroupChat(const muduo::net::TcpConnectionPtr& conn,
                   const GroupChatPtr& message, muduo::Timestamp);

 private:
  typedef std::map<int, muduo::net::TcpConnectionPtr> ConnectionMap;
  muduo::MutexLock mutex_;
  muduo::net::TcpServer server_;
  ProtobufDispatcher dispatcher_;
  ProtobufCodec codec_;
  ConnectionMap connections_ GUARDED_BY(mutex_);
};

#endif  // TOYIM_IMSERVER_H
