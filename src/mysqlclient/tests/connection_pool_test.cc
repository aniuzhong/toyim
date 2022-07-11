#include "mysqlclient/connection_pool.h"

#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>
#include <stdio.h>

#include <memory>
#include <vector>

using muduo::Thread;
using muduo::Timestamp;
using toyim::mysql::ConnectionPool;
using toyim::mysql::ConnectionPtr;

ConnectionPool* g_conn =
    ConnectionPool::instance("root", "123456", "toyim", 10);

void ConnectAndInsert(int num) {
  ConnectionPtr conn = g_conn->TakeConnection();
  char sql[1024] = {0};
  sprintf(sql,
          "INSERT INTO user(name, password, state) "
          "VALUES('User%d','888888','offline')",
          num);
  conn->Update(sql);
}

void ConnectAndInsertNTimesWithSingleThread(int n) {
  Timestamp start(Timestamp::now());
  for (int i = 0; i < n; ++i) ConnectAndInsert(i + n);
  printf("Connect and insert %d times with single thread %f\n", n,
         timeDifference(Timestamp::now(), start));
}

void ConnectAndDelete(int num) {
  ConnectionPtr conn = g_conn->TakeConnection();
  char sql[1024] = {0};
  sprintf(sql, "DELETE FROM user WHERE name='User%d'", num);
  conn->Update(sql);
}

void ConnectAndDeleteNTimesWithSingleThread(int n) {
  Timestamp start(Timestamp::now());
  for (int i = 0; i < n; ++i) ConnectAndDelete(i + n);
  printf("Connect and delete %d times with single thread %f\n", n,
         timeDifference(Timestamp::now(), start));
}

void ConnectAndQuery(int num) {
  ConnectionPtr conn = g_conn->TakeConnection();
  char sql[1024] = {0};
  sprintf(sql, "SELECT id FROM user WHERE name='User%d'", num);
  conn->QueryAndStore(sql);
}

void ConnectAndQueryNTimesWithSingleThread(int n) {
  Timestamp start(Timestamp::now());
  for (int i = 0; i < n; ++i) ConnectAndQuery(i);
  printf("Connect and query %d times with single thread %f\n", n,
         timeDifference(Timestamp::now(), start));
}

void ConnectAndQueryNTimesWithMutiThread(int n, int nthreads) {
  Timestamp start(Timestamp::now());
  std::vector<std::unique_ptr<Thread>> threads;

  for (int i = 0; i < nthreads; ++i) {
    threads.emplace_back(new Thread([&](){
      for (int i = 0; i < n / nthreads; ++i) ConnectAndQuery(i);
    }));
    threads.back()->start();
  }

  for (int i = 0; i < nthreads; ++i) {
    threads[i]->join();
  }

  printf("Connect and query %d times with muti-thread %f\n",
         n, timeDifference(Timestamp::now(), start));
}

int main() {
  ConnectAndInsertNTimesWithSingleThread(1000);
  ConnectAndDeleteNTimesWithSingleThread(1000);
  ConnectAndQueryNTimesWithSingleThread(1000);
  ConnectAndQueryNTimesWithMutiThread(1000, 10);
  return 0;
}
