#include "orm/group_model.h"

#include <stdio.h>

using toyim::orm::Group;
using toyim::orm::GroupModel;
using toyim::orm::GroupUser;

// test QueryGroups
void QueryGroups() {
  std::vector<Group> groups = GroupModel::QueryGroups(13);
  for (const auto& group : groups) {
    printf("group id: %d\nname: %s\ndesc: %s\n", group.id(),
           group.name().c_str(), group.desc().c_str());
    for (const auto& user : group.users()) {
      printf("id=%d, name=%s, password=%s, state=%s\n", user.id(),
             user.name().c_str(), user.password().c_str(),
             user.state().c_str());
    }
  }
}

// test QueryGroup
void QueryGroupUsers(int user_id, int group_id) {
  std::vector<int> ids = GroupModel::QueryGroupUsers(user_id, group_id);
  for (const auto& id : ids) {
    printf("%d ", id);
  }
  printf("\n");
}

void AddGroup(int user_id, int group_id) {
  GroupModel::AddGroup(user_id, group_id, "normal");
}

void CreateGroup(const std::string& group_name, const std::string& group_desc) {
  GroupModel::CreateGroup({-1, group_name, group_desc});
}

int main() {
  // CreateGroup("Bench", "Group for Benchmark.");
  // for (int i = 7022; i <= 12021; ++i) {
  //   AddGroup(i, 3);
  // }
  QueryGroupUsers(7022, 3);
}