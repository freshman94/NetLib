#pragma once

#include <net/Buffer.h>
#include <net/TcpConnection.h>

#include <google/protobuf/message.h>

// struct ProtobufTransportFormat __attribute__ ((__packed__))
// {
//   int32_t  len;
//   int32_t  nameLen;
//   char     typeName[nameLen];
//   char     protobufData[len-nameLen-8];
//   int32_t  checkSum; // adler32 of nameLen, typeName and protobufData
// }

typedef std::shared_ptr<google::protobuf::Message> MessagePtr;

class ProtobufCodec : noncopyable
{
 public:

  enum ErrorCode
  {
    kNoError = 0,
    kInvalidLength,
    kCheckSumError,
    kInvalidNameLen,
    kUnknownMessageType,
    kParseError,
  };

  typedef std::function<void (const TcpConnectionPtr&,
                                const MessagePtr&)> ProtobufMessageCallback;

  typedef std::function<void (const TcpConnectionPtr&,
                              Buffer*, ErrorCode)> ErrorCallback;

  explicit ProtobufCodec(const ProtobufMessageCallback& messageCb)
    : messageCallback_(messageCb),
      errorCallback_(defaultErrorCallback)
  {
  }

  ProtobufCodec(const ProtobufMessageCallback& messageCb, const ErrorCallback& errorCb)
    : messageCallback_(messageCb),
      errorCallback_(errorCb)
  {
  }

  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf);

  void send(const TcpConnectionPtr& conn,
            const google::protobuf::Message& message)
  {
    // FIXME: serialize to TcpConnection::outputBuffer()
    Buffer buf;
    fillEmptyBuffer(&buf, message);
    conn->send(&buf);
  }

  static const string& errorCodeToString(ErrorCode errorCode);
  static void fillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message);
  static google::protobuf::Message* createMessage(const std::string& type_name);
  static MessagePtr parse(const char* buf, int len, ErrorCode* errorCode);

 private:
  static void defaultErrorCallback(const TcpConnectionPtr&,
                                   Buffer*, ErrorCode);

  ProtobufMessageCallback messageCallback_;
  ErrorCallback errorCallback_;

  const static int kHeaderLen = sizeof(int32_t);
  const static int kMinMessageLen = 2*kHeaderLen + 2; // nameLen + typeName + checkSum
  const static int kMaxMessageLen = 64*1024*1024; // same as codec_stream.h kDefaultTotalBytesLimit
};
