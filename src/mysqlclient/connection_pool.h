#ifndef TOYIM_MYSQLCLIENT_CONNECTION_POOL_H
#define TOYIM_MYSQLCLIENT_CONNECTION_POOL_H

#include <muduo/base/BoundedBlockingQueue.h>

#include <memory>

#include "connection.h"

namespace toyim {
namespace mysql {

typedef std::shared_ptr<Connection> ConnectionPtr;

class ConnectionPool : public boost::noncopyable {
 public:
  static ConnectionPool* instance(const std::string& user_name,
                                  const std::string& db_name,
                                  const std::string& password,
                                  unsigned max_size = 3);

  ~ConnectionPool();
  ConnectionPtr TakeConnection();

 private:
  typedef muduo::BoundedBlockingQueue<Connection*> ConnectionList;

  ConnectionPool(const std::string& user_name, const std::string& db_name,
                 const std::string& password, unsigned max_size);

  void PutConnection(Connection* conn);

  // muduo::Thread recycler_;
  // muduo::Thread producer_;
  const std::string user_name_;
  const std::string password_;
  const std::string db_name_;
  const unsigned max_size_;
  ConnectionList connections_;
};

}  // namespace mysql
}  // namespace toyim

#endif  // TOYIM_MYSQLCLIENT_CONNECTION_H
