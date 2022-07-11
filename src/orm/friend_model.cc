#include "orm/friend_model.h"

#include "mysqlclient/connection_pool.h"

namespace toyim {
namespace orm {

using mysql::ConnectionPool;
using mysql::ConnectionPtr;

// FIXME: error check
void FriendModel::Insert(int user_id, int friend_id) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO friend VALUES(%d, %d)", user_id, friend_id);

  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// Return friend list.
// FIXME: error check
std::vector<User> FriendModel::Query(int userid) {
  char sql[1024] = {0};
  std::map<std::string, std::string> row;
  std::vector<User> users;
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();

  sprintf(sql,
          "SELECT a.id, a.name, a.state \
          FROM user a \
          INNER JOIN friend b ON \
          b.friendid = a.id WHERE b.userid=%d",
          userid);

  conn->QueryAndStore(sql);

  for (row = conn->NextRow(); row != std::map<std::string, std::string>();
       row = conn->NextRow()) {
    users.emplace_back(std::stoi(row["id"]), row["name"], "", row["state"]);
  }

  return users;
}

}  // namespace orm
}  // namespace toyim