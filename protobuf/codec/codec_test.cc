#include "codec.h"

#include <protobuf/codec/query.pb.h>

#include <memory>
#include <stdio.h>
#include <zlib.h>  // adler32


void print(const Buffer& buf)
{
  printf("encoded to %zd bytes\n", buf.readableBytes());
  for (size_t i = 0; i < buf.readableBytes(); ++i)
  {
    unsigned char ch = static_cast<unsigned char>(buf.readablePtr()[i]);

    printf("%2zd:  0x%02x  %c\n", i, ch, isgraph(ch) ? ch : ' ');
  }
}

void testQuery()
{
  NetLib::Query query;
  query.set_id(1);
  query.set_questioner("Chen Shuo");
  query.add_question("Running?");

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, query);
  print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.readablePtr(), len, &errorCode);
  assert(errorCode == ProtobufCodec::kNoError);
  assert(message != NULL);
  message->PrintDebugString();
  assert(message->DebugString() == query.DebugString());

  std::shared_ptr<NetLib::Query> newQuery = std::dynamic_pointer_cast<NetLib::Query>(message);
  assert(newQuery != NULL);
}

void testAnswer()
{
  NetLib::Answer answer;
  answer.set_id(1);
  answer.set_questioner("Chen Shuo");
  answer.set_answerer("blog.csdn.net/Solstice");
  answer.add_solution("Jump!");
  answer.add_solution("Win!");

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, answer);
  print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.readablePtr(), len, &errorCode);
  assert(errorCode == ProtobufCodec::kNoError);
  assert(message != NULL);
  message->PrintDebugString();
  assert(message->DebugString() == answer.DebugString());

  std::shared_ptr<NetLib::Answer> newAnswer = std::dynamic_pointer_cast<NetLib::Answer>(message);
  assert(newAnswer != NULL);
}

void testEmpty()
{
  NetLib::Empty empty;

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, empty);
  print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
  MessagePtr message = ProtobufCodec::parse(buf.readablePtr(), len, &errorCode);
  assert(message != NULL);
  message->PrintDebugString();
  assert(message->DebugString() == empty.DebugString());
}

void redoCheckSum(string& data, int len)
{
  int32_t checkSum = htobe32(static_cast<int32_t>(
      ::adler32(1,
                reinterpret_cast<const Bytef*>(data.c_str()),
                static_cast<int>(len - 4))));
  data[len-4] = reinterpret_cast<const char*>(&checkSum)[0];
  data[len-3] = reinterpret_cast<const char*>(&checkSum)[1];
  data[len-2] = reinterpret_cast<const char*>(&checkSum)[2];
  data[len-1] = reinterpret_cast<const char*>(&checkSum)[3];
}

void testBadBuffer()
{
  NetLib::Empty empty;
  empty.set_id(43);

  Buffer buf;
  ProtobufCodec::fillEmptyBuffer(&buf, empty);
  // print(buf);

  const int32_t len = buf.readInt32();
  assert(len == static_cast<int32_t>(buf.readableBytes()));

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len-1, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kCheckSumError);
  }

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[len-1]++;
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kCheckSumError);
  }

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[0]++;
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kCheckSumError);
  }

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[3] = 0;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kInvalidNameLen);
  }

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[3] = 100;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kInvalidNameLen);
  }

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[3]--;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kUnknownMessageType);
  }

  {
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    data[4] = 'M';
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    assert(message == NULL);
    assert(errorCode == ProtobufCodec::kUnknownMessageType);
  }

  {
    // FIXME: reproduce parse error
    string data(buf.readablePtr(), len);
    ProtobufCodec::ErrorCode errorCode = ProtobufCodec::kNoError;
    redoCheckSum(data, len);
    MessagePtr message = ProtobufCodec::parse(data.c_str(), len, &errorCode);
    // assert(message == NULL);
    // assert(errorCode == ProtobufCodec::kParseError);
  }
}

int g_count = 0;

void onMessage(const TcpConnectionPtr& conn,
               const MessagePtr& message)
{
  g_count++;
}

void testOnMessage()
{
  NetLib::Query query;
  query.set_id(1);
  query.set_questioner("Chen Shuo");
  query.add_question("Running?");

  Buffer buf1;
  ProtobufCodec::fillEmptyBuffer(&buf1, query);

  NetLib::Empty empty;
  empty.set_id(43);
  empty.set_id(1982);

  Buffer buf2;
  ProtobufCodec::fillEmptyBuffer(&buf2, empty);

  size_t totalLen = buf1.readableBytes() + buf2.readableBytes();
  Buffer all;
  all.append(buf1.readablePtr(), buf1.readableBytes());
  all.append(buf2.readablePtr(), buf2.readableBytes());
  assert(all.readableBytes() == totalLen);
  TcpConnectionPtr conn;

  ProtobufCodec codec(onMessage);
  for (size_t len = 0; len <= totalLen; ++len)
  {
    Buffer input;
    input.append(all.readablePtr(), len);

    g_count = 0;
    codec.onMessage(conn, &input);
    int expected = len < buf1.readableBytes() ? 0 : 1;
    if (len == totalLen) expected = 2;
    assert(g_count == expected); (void) expected;
    // printf("%2zd %d\n", len, g_count);

    input.append(all.readablePtr() + len, totalLen - len);
    codec.onMessage(conn, &input);
    assert(g_count == 2);
  }
}

int main()
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testQuery();
  puts("");
  testAnswer();
  puts("");
  testEmpty();
  puts("");
  testBadBuffer();
  puts("");
  testOnMessage();
  puts("");

  puts("All pass!!!");

  google::protobuf::ShutdownProtobufLibrary();
}

