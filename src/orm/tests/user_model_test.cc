#include "orm/user_model.h"

#include <stdio.h>

using toyim::orm::User;
using toyim::orm::UserModel;

int main() {
  User user = UserModel::Query(23);
  printf("name=%s, password=%s, state=%s\n", user.name().c_str(),
         user.password().c_str(), user.state().c_str());
  user = UserModel::Query("zhang san");
  printf("name=%s, password=%s, state=%s\n", user.name().c_str(),
         user.password().c_str(), user.state().c_str());
}