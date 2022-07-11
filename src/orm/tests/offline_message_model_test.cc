#include "orm/offline_message_model.h"

#include <stdio.h>

using toyim::orm::OfflineMessageModel;

int main() {
  // test Query
  std::vector<std::string> messages = OfflineMessageModel::Query(19);
  // json
  for (const auto& message : messages) {
    printf("%s\n", message.c_str());
  }
}
