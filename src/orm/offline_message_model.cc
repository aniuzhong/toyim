#include "orm/offline_message_model.h"

#include "mysqlclient/connection_pool.h"

namespace toyim {
namespace orm {

using mysql::ConnectionPool;
using mysql::ConnectionPtr;

// FIXME: error check
void OfflineMessageModel::Insert(int user_id, const std::string& message) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO offlinemessage VALUES(%d, '%s')", user_id,
          message.c_str());

  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// FIXME: error check
void OfflineMessageModel::Remove(int user_id) {
  char sql[1024] = {0};
  sprintf(sql, "DELETE FROM offlinemessage WHERE userid=%d", user_id);

  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// FIXME: error check
std::vector<std::string> OfflineMessageModel::Query(int user_id) {
  char sql[1024] = {0};
  std::map<std::string, std::string> row;
  std::vector<std::string> messages;
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();

  sprintf(sql, "SELECT message FROM offlinemessage WHERE userid = %d", user_id);

  conn->QueryAndStore(sql);

  for (row = conn->NextRow(); row != std::map<std::string, std::string>();
       row = conn->NextRow()) {
    messages.emplace_back(row["message"]);
  }

  return messages;
}

}  // namespace orm
}  // namespace toyim