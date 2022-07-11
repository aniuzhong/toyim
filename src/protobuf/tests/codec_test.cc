#include "protorpc/codec.h"

#include <muduo/net/Callbacks.h>

#include "test.pb.h"

using muduo::down_pointer_cast;
using muduo::net::Buffer;

void PrintBuffer(const Buffer& buf) {
  printf("encoded to %zd bytes\n", buf.readableBytes());
  for (size_t i = 0; i < buf.readableBytes(); ++i) {
    unsigned char ch = static_cast<unsigned char>(buf.peek()[i]);
    printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
  }
}

void TestLoginRequest() {
  toyim::LoginRequest request;
  request.set_id(23);
  request.set_password("123456");

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, request);
  PrintBuffer(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode error_code = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.peek(), len, &error_code);
  assert(error_code == ProtobufCodec::kNoError);
  assert(message != NULL);
  assert(message->DebugString() == request.DebugString());

  std::shared_ptr<toyim::LoginRequest> new_request =
      down_pointer_cast<toyim::LoginRequest>(message);
  assert(new_request != NULL);
}

void TestLoginResponse() {
  toyim::LoginResponse response;
  response.set_status_code(1);

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, response);
  PrintBuffer(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode error_code = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.peek(), len, &error_code);
  assert(error_code == ProtobufCodec::kNoError);
  assert(message != NULL);

  std::shared_ptr<toyim::LoginResponse> new_response =
      down_pointer_cast<toyim::LoginResponse>(message);
  assert(new_response != NULL);
}

int g_count = 0;

void OnMessage(const muduo::net::TcpConnectionPtr& conn,
               const MessagePtr& message, muduo::Timestamp receiveTime) {
  g_count++;
}

void TestOnMessage() {
  toyim::LoginRequest request;
  request.set_id(1);
  request.set_password("123456");

  Buffer buf1;
  ProtobufCodec::fillEmptyBuffer(&buf1, request);

  toyim::Empty empty;
  empty.set_id(1);

  Buffer buf2;
  ProtobufCodec::fillEmptyBuffer(&buf2, request);

  size_t total_len = buf1.readableBytes() + buf2.readableBytes();
  Buffer all;
  all.append(buf1.peek(), buf1.readableBytes());
  all.append(buf2.peek(), buf2.readableBytes());
  assert(all.readableBytes() == total_len);

  muduo::net::TcpConnectionPtr conn;
  muduo::Timestamp t;
  ProtobufCodec codec(OnMessage);
  for (size_t len = 0; len <= total_len; ++len) {
    Buffer input;
    input.append(all.peek(), len);

    g_count = 0;
    codec.onMessage(conn, &input, t);
    int expected = len < buf1.readableBytes() ? 0 : 1;
    if (len == total_len) expected = 2;
    assert(g_count == expected);
    (void)expected;

    input.append(all.peek() + len, total_len - len);
    codec.onMessage(conn, &input, t);
    assert(g_count == 2);
  }
}

int main() {
  TestLoginRequest();
  TestLoginResponse();
  TestOnMessage();
}
