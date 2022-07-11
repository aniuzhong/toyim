#ifndef TOYIM_GROUP_MODEL_H
#define TOYIM_GROUP_MODEL_H

#include "orm/group.h"

namespace toyim {
namespace orm {

struct GroupModel {
  static void CreateGroup(const Group& group);

  static void AddGroup(int user_id, int group_id, const std::string& role);

  // Query user's group info, use to init ImClient.
  static std::vector<Group> QueryGroups(int userid);

  // Query group user ids based on the specified groupid, except userid.
  // Use to send group messages to other members of the group.
  static std::vector<int> QueryGroupUsers(int user_id, int group_id);
};

}  // namespace orm
}  // namespace toyim

#endif