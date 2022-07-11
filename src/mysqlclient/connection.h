#ifndef TOYIM_MYSQLCLIENT_CONNECTION_H
#define TOYIM_MYSQLCLIENT_CONNECTION_H

#include <mysql/mysql.h>

#include <boost/noncopyable.hpp>
#include <map>
#include <string>

namespace toyim {
namespace mysql {

class Connection : public boost::noncopyable {
 public:
  Connection();
  ~Connection();

  void Connect(const std::string& user_name, const std::string& password,
               const std::string& db_name,
               const std::string& host_name = "127.0.0.1",
               uint16_t port = 3306);

  void QueryAndStore(const std::string& stmt);
  std::map<std::string, std::string> NextRow();

  void Update(const std::string& stmt);

 private:
  MYSQL* mysql_;
  MYSQL_RES* result_;
};

}  // namespace mysql
}  // namespace toyim

#endif  // TOYIM_MYSQLCLIENT_CONNECTION_H
