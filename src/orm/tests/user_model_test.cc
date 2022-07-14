#include "orm/user_model.h"

#include <stdio.h>

using toyim::orm::User;
using toyim::orm::UserModel;

void InsertOneUser(int user_num) {
  std::string user_name("User" + std::to_string(user_num));
  UserModel::Insert({-1, user_name, "888888", "offline"});
}

void InsertNUsers(int n) {
  for (int i = 0; i < n; ++i) InsertOneUser(i);
}

int main() {
  // 
  InsertNUsers(5000);
}