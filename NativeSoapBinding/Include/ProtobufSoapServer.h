#pragma once


#include <google/protobuf/message.h>

namespace google
{
  namespace protobuf
  {
    class RpcChannel;
    class Service;
  }
}

typedef function<void(google::protobuf::Message&)> ProtobufResponseCallback;
typedef function<void(const google::protobuf::Message&, ProtobufResponseCallback)> ProtobufCallback;


namespace
{
  template <typename T>
  void CallProtobuf(shared_ptr<T> t, const ::google::protobuf::MethodDescriptor* method, const google::protobuf::Message& req, function<void(const google::protobuf::Message&)> callback)
  {
    ::google::protobuf::RpcController* controller = 0;
    ::google::protobuf::Closure* closure = 0;

    google::protobuf::Message resp;

    t->CallMethod(method, controller, &req, &resp, closure);

    callback(resp);
  }
}


