#include "connection_pool.h"

#include <functional>

namespace toyim {
namespace mysql {

using std::placeholders::_1;

ConnectionPool::ConnectionPool(const std::string& user_name,
                               const std::string& password,
                               const std::string& db_name, unsigned max_size)
    : user_name_(user_name),
      password_(password),
      db_name_(db_name),
      max_size_(max_size),
      connections_(max_size_) {
  for (unsigned i = 0; i < max_size_; ++i) {
    Connection* conn = new Connection;
    conn->Connect(user_name_, password_, db_name_);
    PutConnection(conn);
  }
}

ConnectionPool::~ConnectionPool() {
  while (!connections_.empty()) {
    Connection* conn = connections_.take();
    delete conn;
  }
}

ConnectionPool* ConnectionPool::instance(const std::string& user_name,
                                         const std::string& password,
                                         const std::string& db_name,
                                         unsigned max_size) {
  // FIXME: Load configuration.
  static ConnectionPool pool(user_name, password, db_name, max_size);
  return &pool;
}

ConnectionPtr ConnectionPool::TakeConnection() {
  return {connections_.take(),
          std::bind(&ConnectionPool::PutConnection, this, _1)};
}

void ConnectionPool::PutConnection(Connection* conn) { connections_.put(conn); }

}  // namespace mysql
}  // namespace toyim