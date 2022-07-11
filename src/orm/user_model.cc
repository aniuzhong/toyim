#include "orm/user_model.h"

#include "mysqlclient/connection_pool.h"

namespace toyim {
namespace orm {

using mysql::ConnectionPool;
using mysql::ConnectionPtr;

// FIXME: error check
void UserModel::Insert(const User& user) {
  char sql[1024] = {0};
  sprintf(sql,
          "INSERT INTO user(name, password, state) VALUES('%s', '%s', '%s')",
          user.name().c_str(), user.password().c_str(), user.state().c_str());
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// Query specified user by id.
// FIXME: error check
User UserModel::Query(int id) {
  char sql[1024] = {0};
  sprintf(sql, "SELECT * FROM user WHERE id = %d", id);
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->QueryAndStore(sql);
  auto row = conn->NextRow();
  if (row == std::map<std::string, std::string>()) return User();
  User user(id, row["name"], row["password"], row["state"]);
  return user;
}

// Query specified user by name.
// FIXME: error check
User UserModel::Query(const std::string& name) {
  char sql[1024] = {0};
  sprintf(sql, "SELECT * FROM user WHERE name = '%s'", name.c_str());
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->QueryAndStore(sql);
  auto row = conn->NextRow();
  if (row == std::map<std::string, std::string>()) return User();
  return {std::stoi(row["id"]), name, row["password"], row["state"]};
}

// FIXME: error check
void UserModel::UpdateState(const User& user) {
  char sql[1024] = {0};
  sprintf(sql, "UPDATE user SET state='%s' WHERE id = %d", user.state().c_str(),
          user.id());
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// FIXME: error check
void UserModel::ResetState() {
  char sql[1024] = {0};
  sprintf(sql, "UPDATE user SET state='offline' WHERE state='online'");
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

}  // namespace orm
}  // namespace toyim