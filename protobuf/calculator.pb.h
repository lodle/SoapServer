// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: calculator.proto

#ifndef PROTOBUF_calculator_2eproto__INCLUDED
#define PROTOBUF_calculator_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2006000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/service.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_calculator_2eproto();
void protobuf_AssignDesc_calculator_2eproto();
void protobuf_ShutdownFile_calculator_2eproto();

class CalculatorRequest;
class CalculatorResponse;
class PingRequest;
class PingResponse;

// ===================================================================

class CalculatorRequest : public ::google::protobuf::Message {
 public:
  CalculatorRequest();
  virtual ~CalculatorRequest();

  CalculatorRequest(const CalculatorRequest& from);

  inline CalculatorRequest& operator=(const CalculatorRequest& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CalculatorRequest& default_instance();

  void Swap(CalculatorRequest* other);

  // implements Message ----------------------------------------------

  CalculatorRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CalculatorRequest& from);
  void MergeFrom(const CalculatorRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required double param1 = 1;
  inline bool has_param1() const;
  inline void clear_param1();
  static const int kParam1FieldNumber = 1;
  inline double param1() const;
  inline void set_param1(double value);

  // required double param2 = 2;
  inline bool has_param2() const;
  inline void clear_param2();
  static const int kParam2FieldNumber = 2;
  inline double param2() const;
  inline void set_param2(double value);

  // @@protoc_insertion_point(class_scope:CalculatorRequest)
 private:
  inline void set_has_param1();
  inline void clear_has_param1();
  inline void set_has_param2();
  inline void clear_has_param2();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double param1_;
  double param2_;
  friend void  protobuf_AddDesc_calculator_2eproto();
  friend void protobuf_AssignDesc_calculator_2eproto();
  friend void protobuf_ShutdownFile_calculator_2eproto();

  void InitAsDefaultInstance();
  static CalculatorRequest* default_instance_;
};
// -------------------------------------------------------------------

class CalculatorResponse : public ::google::protobuf::Message {
 public:
  CalculatorResponse();
  virtual ~CalculatorResponse();

  CalculatorResponse(const CalculatorResponse& from);

  inline CalculatorResponse& operator=(const CalculatorResponse& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CalculatorResponse& default_instance();

  void Swap(CalculatorResponse* other);

  // implements Message ----------------------------------------------

  CalculatorResponse* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CalculatorResponse& from);
  void MergeFrom(const CalculatorResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required double result = 1;
  inline bool has_result() const;
  inline void clear_result();
  static const int kResultFieldNumber = 1;
  inline double result() const;
  inline void set_result(double value);

  // @@protoc_insertion_point(class_scope:CalculatorResponse)
 private:
  inline void set_has_result();
  inline void clear_has_result();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double result_;
  friend void  protobuf_AddDesc_calculator_2eproto();
  friend void protobuf_AssignDesc_calculator_2eproto();
  friend void protobuf_ShutdownFile_calculator_2eproto();

  void InitAsDefaultInstance();
  static CalculatorResponse* default_instance_;
};
// -------------------------------------------------------------------

class PingRequest : public ::google::protobuf::Message {
 public:
  PingRequest();
  virtual ~PingRequest();

  PingRequest(const PingRequest& from);

  inline PingRequest& operator=(const PingRequest& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const PingRequest& default_instance();

  void Swap(PingRequest* other);

  // implements Message ----------------------------------------------

  PingRequest* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const PingRequest& from);
  void MergeFrom(const PingRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:PingRequest)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_calculator_2eproto();
  friend void protobuf_AssignDesc_calculator_2eproto();
  friend void protobuf_ShutdownFile_calculator_2eproto();

  void InitAsDefaultInstance();
  static PingRequest* default_instance_;
};
// -------------------------------------------------------------------

class PingResponse : public ::google::protobuf::Message {
 public:
  PingResponse();
  virtual ~PingResponse();

  PingResponse(const PingResponse& from);

  inline PingResponse& operator=(const PingResponse& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const PingResponse& default_instance();

  void Swap(PingResponse* other);

  // implements Message ----------------------------------------------

  PingResponse* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const PingResponse& from);
  void MergeFrom(const PingResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // @@protoc_insertion_point(class_scope:PingResponse)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  friend void  protobuf_AddDesc_calculator_2eproto();
  friend void protobuf_AssignDesc_calculator_2eproto();
  friend void protobuf_ShutdownFile_calculator_2eproto();

  void InitAsDefaultInstance();
  static PingResponse* default_instance_;
};
// ===================================================================

class Calculator_Stub;

class Calculator : public ::google::protobuf::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline Calculator() {};
 public:
  virtual ~Calculator();

  typedef Calculator_Stub Stub;

  static const ::google::protobuf::ServiceDescriptor* descriptor();

  virtual void Add(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Subtract(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Multiply(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
  virtual void Divide(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::google::protobuf::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  ::google::protobuf::RpcController* controller,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done);
  const ::google::protobuf::Message& GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const;
  const ::google::protobuf::Message& GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Calculator);
};

class Calculator_Stub : public Calculator {
 public:
  Calculator_Stub(::google::protobuf::RpcChannel* channel);
  Calculator_Stub(::google::protobuf::RpcChannel* channel,
                   ::google::protobuf::Service::ChannelOwnership ownership);
  ~Calculator_Stub();

  inline ::google::protobuf::RpcChannel* channel() { return channel_; }

  // implements Calculator ------------------------------------------

  void Add(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
  void Subtract(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
  void Multiply(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
  void Divide(::google::protobuf::RpcController* controller,
                       const ::CalculatorRequest* request,
                       ::CalculatorResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::google::protobuf::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Calculator_Stub);
};


// -------------------------------------------------------------------

class Echo_Stub;

class Echo : public ::google::protobuf::Service {
 protected:
  // This class should be treated as an abstract interface.
  inline Echo() {};
 public:
  virtual ~Echo();

  typedef Echo_Stub Stub;

  static const ::google::protobuf::ServiceDescriptor* descriptor();

  virtual void Ping(::google::protobuf::RpcController* controller,
                       const ::PingRequest* request,
                       ::PingResponse* response,
                       ::google::protobuf::Closure* done);

  // implements Service ----------------------------------------------

  const ::google::protobuf::ServiceDescriptor* GetDescriptor();
  void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                  ::google::protobuf::RpcController* controller,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done);
  const ::google::protobuf::Message& GetRequestPrototype(
    const ::google::protobuf::MethodDescriptor* method) const;
  const ::google::protobuf::Message& GetResponsePrototype(
    const ::google::protobuf::MethodDescriptor* method) const;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Echo);
};

class Echo_Stub : public Echo {
 public:
  Echo_Stub(::google::protobuf::RpcChannel* channel);
  Echo_Stub(::google::protobuf::RpcChannel* channel,
                   ::google::protobuf::Service::ChannelOwnership ownership);
  ~Echo_Stub();

  inline ::google::protobuf::RpcChannel* channel() { return channel_; }

  // implements Echo ------------------------------------------

  void Ping(::google::protobuf::RpcController* controller,
                       const ::PingRequest* request,
                       ::PingResponse* response,
                       ::google::protobuf::Closure* done);
 private:
  ::google::protobuf::RpcChannel* channel_;
  bool owns_channel_;
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Echo_Stub);
};


// ===================================================================


// ===================================================================

// CalculatorRequest

// required double param1 = 1;
inline bool CalculatorRequest::has_param1() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CalculatorRequest::set_has_param1() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CalculatorRequest::clear_has_param1() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CalculatorRequest::clear_param1() {
  param1_ = 0;
  clear_has_param1();
}
inline double CalculatorRequest::param1() const {
  // @@protoc_insertion_point(field_get:CalculatorRequest.param1)
  return param1_;
}
inline void CalculatorRequest::set_param1(double value) {
  set_has_param1();
  param1_ = value;
  // @@protoc_insertion_point(field_set:CalculatorRequest.param1)
}

// required double param2 = 2;
inline bool CalculatorRequest::has_param2() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void CalculatorRequest::set_has_param2() {
  _has_bits_[0] |= 0x00000002u;
}
inline void CalculatorRequest::clear_has_param2() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void CalculatorRequest::clear_param2() {
  param2_ = 0;
  clear_has_param2();
}
inline double CalculatorRequest::param2() const {
  // @@protoc_insertion_point(field_get:CalculatorRequest.param2)
  return param2_;
}
inline void CalculatorRequest::set_param2(double value) {
  set_has_param2();
  param2_ = value;
  // @@protoc_insertion_point(field_set:CalculatorRequest.param2)
}

// -------------------------------------------------------------------

// CalculatorResponse

// required double result = 1;
inline bool CalculatorResponse::has_result() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void CalculatorResponse::set_has_result() {
  _has_bits_[0] |= 0x00000001u;
}
inline void CalculatorResponse::clear_has_result() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void CalculatorResponse::clear_result() {
  result_ = 0;
  clear_has_result();
}
inline double CalculatorResponse::result() const {
  // @@protoc_insertion_point(field_get:CalculatorResponse.result)
  return result_;
}
inline void CalculatorResponse::set_result(double value) {
  set_has_result();
  result_ = value;
  // @@protoc_insertion_point(field_set:CalculatorResponse.result)
}

// -------------------------------------------------------------------

// PingRequest

// -------------------------------------------------------------------

// PingResponse


// @@protoc_insertion_point(namespace_scope)

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_calculator_2eproto__INCLUDED
