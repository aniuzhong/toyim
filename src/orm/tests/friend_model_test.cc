#include "orm/friend_model.h"

#include <stdio.h>

using toyim::orm::FriendModel;
using toyim::orm::User;

int main() {
  // test Query
  std::vector<User> users = FriendModel::Query(13);

  for (const auto& user : users) {
    printf("id=%d, name=%s, password=%s, state=%s\n", user.id(),
           user.name().c_str(), user.password().c_str(), user.state().c_str());
  }
}
