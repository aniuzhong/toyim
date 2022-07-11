#include "orm/group_model.h"

#include "mysqlclient/connection_pool.h"

namespace toyim {
namespace orm {

using mysql::ConnectionPool;
using mysql::ConnectionPtr;

// FIXME: error check
void GroupModel::CreateGroup(const Group &group) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO allgroup(groupname, groupdesc) VALUES('%s', '%s')",
          group.name().c_str(), group.desc().c_str());

  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// FIXME: error check
void GroupModel::AddGroup(int user_id, int group_id, const std::string &role) {
  char sql[1024] = {0};
  sprintf(sql, "INSERT INTO groupuser VALUES(%d, %d, '%s')", group_id, user_id,
          role.c_str());
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();
  conn->Update(sql);
}

// FIXME: error check
std::vector<Group> GroupModel::QueryGroups(int user_id) {
  char sql[1024] = {0};
  std::map<std::string, std::string> row;
  std::vector<Group> groups;
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();

  // Query groups to which the user belongs.
  sprintf(sql,
          "SELECT a.id, a.groupname, a.groupdesc \
          FROM allgroup a \
          INNER JOIN groupuser b ON a.id = b.groupid \
          WHERE b.userid=%d",
          user_id);

  conn->QueryAndStore(sql);
  // copy: MYSQL_ROW->std::map<std::string, std::string>
  for (row = conn->NextRow(); row != std::map<std::string, std::string>();
       row = conn->NextRow()) {
    groups.emplace_back(std::stoi(row["id"]), row["groupname"],
                        row["groupdesc"]);
  }

  // Query the user information of a group
  for (auto &group : groups) {
    sprintf(sql,
            "SELECT a.id, a.name, a.state, b.grouprole \
            FROM user a \
            INNER JOIN groupuser b ON b.userid = a.id \
            WHERE b.groupid=%d",
            group.id());

    conn->QueryAndStore(sql);

    for (row = conn->NextRow(); row != std::map<std::string, std::string>();
         row = conn->NextRow()) {
      group.users().emplace_back(std::stoi(row["id"]), row["name"], "",
                                 row["state"], row["grouprole"]);
    }
  }

  // RVO
  return groups;
}

// FIXME: error check
std::vector<int> GroupModel::QueryGroupUsers(int user_id, int group_id) {
  char sql[1024] = {0};
  std::map<std::string, std::string> row;
  std::vector<int> ids;
  ConnectionPtr conn =
      ConnectionPool::instance("root", "123456", "toyim", 4)->TakeConnection();

  sprintf(sql,
          "SELECT userid FROM groupuser WHERE groupid = %d and userid != %d",
          group_id, user_id);

  conn->QueryAndStore(sql);

  // handle error: group does not exist.
  for (row = conn->NextRow(); row != std::map<std::string, std::string>();
       row = conn->NextRow()) {
    ids.emplace_back(std::stoi(row["userid"]));
  }
  return ids;
}

}  // namespace orm
}  // namespace toyim