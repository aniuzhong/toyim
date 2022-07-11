#include "connection.h"

#include <muduo/base/Logging.h>

namespace toyim {
namespace mysql {

Connection::Connection() : mysql_(NULL), result_(NULL) {
  mysql_ = ::mysql_init(NULL);
  if (!mysql_) {
    LOG_FATAL << "mysql_init() failed.";
  }
}

Connection::~Connection() {
  if (result_) {
    ::mysql_free_result(result_);
    result_ = NULL;
  }

  if (mysql_) {
    ::mysql_close(mysql_);
  }
}

void Connection::Connect(const std::string& user_name,
                         const std::string& password,
                         const std::string& db_name,
                         const std::string& host_name, uint16_t port) {
  if (!::mysql_real_connect(mysql_, host_name.c_str(), user_name.c_str(),
                            password.c_str(), db_name.c_str(), port, NULL, 0)) {
    ::mysql_close(mysql_);
    LOG_FATAL << "mysql_real_connect() failed";
  }
  // LOG_INFO << "User " << mysql_->user << " connected to database " << mysql_->db
  //          << " successfully. " << mysql_->host_info
  //          << " (port:" << mysql_->port << ")";
}

void Connection::QueryAndStore(const std::string& stmt) {
  // Before a new query, free the result.
  if (result_) {
    ::mysql_free_result(result_);
    result_ = NULL;
  }

  if (::mysql_query(mysql_, stmt.c_str())) {
    ::mysql_close(mysql_);
    LOG_FATAL << "mysql_query() failed. " << ::mysql_error(mysql_);
  }

  result_ = ::mysql_store_result(mysql_);
  if (!result_) {
    ::mysql_close(mysql_);
    LOG_FATAL << "mysql_store_result() failed. " << ::mysql_error(mysql_);
  }
}

// FIXME: Error check
std::map<std::string, std::string> Connection::NextRow() {
  MYSQL_FIELD* fields = ::mysql_fetch_fields(result_);
  MYSQL_ROW current_row = ::mysql_fetch_row(result_);

  if (current_row == NULL) return std::map<std::string, std::string>();

  std::map<std::string, std::string> ret;
  for (unsigned int i = 0; i < ::mysql_num_fields(result_); ++i) {
    std::string field_name = fields[i].name;
    unsigned long field_length = ::mysql_fetch_lengths(result_)[i];
    std::string value(current_row[i], field_length);
    ret[field_name] = value;
  }
  return ret;
}

// FIXME: Error check
void Connection::Update(const std::string& stmt) {
  if (::mysql_query(mysql_, stmt.c_str())) {
    ::mysql_close(mysql_);
    LOG_FATAL << "mysql_query() failed. " << ::mysql_error(mysql_)
              << ", SQL statement is:" << stmt;
  }
}

}  // namespace mysql
}  // namespace toyim
