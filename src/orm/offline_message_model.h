#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>
#include <vector>

namespace toyim {
namespace orm {

struct OfflineMessageModel {
  static void Insert(int user_id, const std::string& message);
  static void Remove(int user_id);
  static std::vector<std::string> Query(int user_id);
};

}  // namespace orm
}  // namespace toyim

#endif