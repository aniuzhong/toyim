#ifndef TOYIM_ORM_USER_MODEL_H
#define TOYIM_ORM_USER_MODEL_H

#include "orm/user.h"

namespace toyim {
namespace orm {

struct UserModel {
  static void Insert(const User& user);
  static User Query(int id);
  static User Query(const std::string& name);
  static void UpdateState(const User& user);
  static void ResetState();
};

}  // namespace orm
}  // namespace toyim

#endif  // TOYIM_ORM_USER_MODEL_H