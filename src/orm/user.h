#ifndef TOYIM_ORM_USER_H
#define TOYIM_ORM_USER_H

#include <muduo/base/copyable.h>

#include <string>

namespace toyim {
namespace orm {

// user table's ORM
class User : public muduo::copyable {
 public:
  User(int id = -1, const std::string& name = "",
       const std::string& password = "", const std::string& state = "offline")
      : id_(id), name_(name), password_(password), state_(state) {}

  void set_id(int id) { id_ = id; }
  void set_name(const std::string& name) { name_ = name; }
  void set_password(const std::string& password) { password_ = password; }
  void set_state(const std::string& state) { state_ = state; }

  int id() const { return id_; }
  const std::string& name() const { return name_; }
  const std::string& password() const { return password_; }
  const std::string& state() const { return state_; }

 protected:
  int id_;
  std::string name_;
  std::string password_;
  std::string state_;
};

}  // namespace orm
}  // namespace toyim

#endif  // TOYIM_MYSQLCLIENT_CONNECTION_H
