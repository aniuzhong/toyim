#ifndef TOYIM_GROUP_USER_H
#define TOYIM_GROUP_USER_H

#include "orm/user.h"

namespace toyim {
namespace orm {

class GroupUser : public User {
 public:
  GroupUser(int id = -1, const std::string& name = "",
            const std::string& password = "",
            const std::string& state = "offline",
            const std::string& role = "normal")
      : User(id, name, password, state), role_(role) {}

  void set_role(const std::string& role) { role_ = role; }
  std::string role() { return role_; }

 private:
  std::string role_;
};

}  // namespace orm
}  // namespace toyim

#endif