#ifndef TOYIM_ORM_FRIENDMODEL_H
#define TOYIM_ORM_FRIENDMODEL_H

#include <vector>

#include "orm/user.h"

namespace toyim {
namespace orm {

struct FriendModel {
  static void Insert(int user_id, int friend_id);
  static std::vector<User> Query(int userid);
};

}  // namespace orm
}  // namespace toyim

#endif  // TOYIM_ORM_FRIENDMODEL_H