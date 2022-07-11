#include "orm/group_model.h"

#include <stdio.h>

using toyim::orm::Group;
using toyim::orm::GroupModel;
using toyim::orm::GroupUser;

int main() {
  // test QueryGroups
  std::vector<Group> groups = GroupModel::QueryGroups(13);
  for (const auto& group : groups) {
    printf("group id: %d\nname: %s\ndesc: %s\n", group.id(), group.name().c_str(),
           group.desc().c_str());
    for (const auto& user : group.users()) {
      printf("id=%d, name=%s, password=%s, state=%s\n", user.id(),
             user.name().c_str(), user.password().c_str(),
             user.state().c_str());
    }
  }

  // test QueryGroupUsers
  std::vector<int> ids = GroupModel::QueryGroupUsers(13, 1);
  for (const auto& id : ids) {
    printf("%d ", id);
  }
  printf("\n");
}