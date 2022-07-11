#ifndef TOYIM_ORM_GROUP_H
#define TOYIM_ORM_GROUP_H

#include <string>
#include <vector>

#include "orm/group_user.h"

namespace toyim {
namespace orm {

// group table's orm class.
class Group {
 public:
  Group(int id = -1, const std::string& name = "", const std::string& desc = "")
      : id_(id), name_(name), desc_(desc) {}

  int id() const { return id_; }
  std::string name() const { return name_; }
  std::string desc() const { return desc_; }

  void set_id(int id) { id_ = id; };
  void set_name(const std::string& name) { name_ = name; };
  void set_desc(const std::string& desc) { desc_ = desc; };

  // Return the users_.
  const std::vector<GroupUser>& users() const { return users_; }
  // Modify the users_.
  std::vector<GroupUser>& users() { return users_; }

 private:
  int id_;
  std::string name_;
  std::string desc_;
  std::vector<GroupUser> users_;
};

}  // namespace orm
}  // namespace toyim

#endif  // TOYIM_ORM_GROUP_H