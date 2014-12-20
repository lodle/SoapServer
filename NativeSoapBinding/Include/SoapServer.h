#pragma once

class SoapServer;
class SoapServerInternal;
class ClassBinding;
class FieldBinding;


#include <string>
#include <functional>
#include <memory>
#include <typeinfo>
#include <vector>

using namespace std;





#include <google/protobuf/message.h>

namespace google
{
	namespace protobuf
	{
		class RpcChannel;
		class Service;
	}
}

class ObjectWrapper
{
public:
  ObjectWrapper(void* obj, function<void()> deleter)
    : m_object(obj)
    , m_deleter(deleter)
  {
  }

  ~ObjectWrapper()
  {
    m_deleter();
  }

  template <typename T>
  static shared_ptr<ObjectWrapper> Create()
  {
    T* t = new T();
    function<void()> deleter = bind(&ObjectWrapper::DeleteObj<T>, t);
    ObjectWrapper* o = new ObjectWrapper(t, deleter);
    return shared_ptr<ObjectWrapper>(o);
  }

  void* get()
  {
    return m_object;
  }

protected:
  template <typename T>
  static void DeleteObj(T* obj)
  {
    delete obj;
  }

private:
  void* m_object;
  function<void()> m_deleter;
};

 
typedef function<void(google::protobuf::Message&)> ProtobufResponseCallback;
typedef function<void(const void*)> NativeResponseCallback;

typedef function<void(const google::protobuf::Message&, ProtobufResponseCallback)> ProtobufCallback;
typedef function<void(const void*, NativeResponseCallback)> NativeCallback;
typedef function<shared_ptr<ObjectWrapper>()> CreateObjectCallback;

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

  template <typename R>
  void CallNativeResponse(const R& r, NativeResponseCallback callback)
  {
    callback(&r);
  }

	template <typename T, typename A, typename R>
	void CallNative(shared_ptr<T> t, void (T::*funct)(const A&, function<void(const R&)>), const void* aIn, NativeResponseCallback callback)
	{
    function<void(const R&)> nativeCallback = bind(&CallNativeResponse<R>, placeholders::_1, callback);

		const A& a = *reinterpret_cast<const A*>(aIn);
		(*t.get().*funct)(a, nativeCallback);
	}

  template <typename T>
  void DeleteObject(void* obj)
  {
    delete reinterpret_cast<T*>(obj);
  }
}




template <typename T>
class SoapNativeFunctionBinderInbound
{
public:
  SoapNativeFunctionBinderInbound(SoapServer* server, const shared_ptr<T>& t, const string& serviceName)
		: m_server(server)
		, m_serviceName(serviceName)
		, m_t(t)
	{
	}

	template <typename A, typename R>
  SoapNativeFunctionBinderInbound<T>& Function(const string& name, void (T::*funct)(const A&, function<void(const R&)>))
	{
		const ClassBinding& request = m_server->GetClassBinding<A>();
		const ClassBinding& response = m_server->GetClassBinding<R>();

		NativeCallback callback = bind(&CallNative<T, A, R>, m_t, funct, placeholders::_1, placeholders::_2);
		m_server->AddMethod(m_serviceName, name, request, response, callback, true);

		return *this;
	}

private:
	SoapServer* m_server;
	string m_serviceName;
	shared_ptr<T> m_t;
};


class TypeWrapper
{
public:
  TypeWrapper(const type_info& info)
    : m_info(info)
  {

  }

  const type_info& get()
  {
    return m_info;
  }

private:
  const type_info& m_info;
};




template <typename REQ, typename RESP>
class NativeSoapCall
{
public:
  void operator()(function<void(const RESP&)> callback, const REQ& req)
  {
    assert(m_callsoap);
    m_callsoap(bind(&DoCallback, callback, placeholders::_1), &req);
  }

protected:
  static void DoCallback(function<void(const RESP&)> callback, const void* arg)
  {
    callback(*reinterpret_cast<const RESP*>(arg));
  }
  
  template <typename T>
  friend class SoapNativeFunctionBinderOutbound;

  function<void(function<void(const void*)>, const REQ*)> m_callsoap;
};




template <typename T>
class SoapNativeFunctionBinderOutbound
{
public:
  SoapNativeFunctionBinderOutbound(SoapServer* server, const shared_ptr<T>& t, const string& serviceName)
    : m_server(server)
    , m_serviceName(serviceName)
    , m_t(t)
  {
  }

  template <typename A, typename R>
  SoapNativeFunctionBinderOutbound<T>& Function(const string& name, NativeSoapCall<A, R> &invoker)
  {
    const ClassBinding& request = m_server->GetClassBinding<A>();
    const ClassBinding& response = m_server->GetClassBinding<R>();

    CallInfo info(m_server, m_serviceName, name, typeid(A), typeid(R));

    invoker.m_callsoap = bind(&CallSoap, info,  placeholders::_1, placeholders::_2);
    
    NativeCallback nc;
    m_server->AddMethod(m_serviceName, name, request, response, nc, false);
    return *this;
  }

protected:
  class CallInfo
  {
  public:
    CallInfo(SoapServer* server, const string& serviceName, const string& name, const type_info& reqType, const type_info& respType)
      : m_server(server)
      , m_serviceName(serviceName)
      , m_name(name)
      , m_reqType(reqType)
      , m_respType(respType)
    {

    }
    SoapServer* m_server;
    string m_serviceName;
    string m_name;
    const type_info& m_reqType;
    const type_info& m_respType;
  };

  static void CallSoap(CallInfo info, function<void(const void*)> callback, const void* resp)
  {
    info.m_server->CallMethod(info.m_serviceName, info.m_name, resp, callback, info.m_reqType, info.m_respType);
  }

private:
  SoapServer* m_server;
  string m_serviceName;
  shared_ptr<T> m_t;
};



template <typename T>
class SoapFieldVisitorT
{
public:
	SoapFieldVisitorT(const string& name, size_t offset)
		: m_name(name)
		, m_offset(offset)
		, m_size(sizeof(T))
	{
	}

	const string m_name;
	const size_t m_offset;
	const size_t m_size;
};

template <typename T>
class SoapClassVisitorT
{
public:
	SoapClassVisitorT(const string &name)
		: m_name(name)
	{
	}

	const string m_name;
};

template <typename T>
SoapFieldVisitorT<T> SoapFieldVisitorCreate(const string& name, size_t offset, const T&)
{
	return SoapFieldVisitorT<T>(name, offset);
}

#define SoapFieldVisitor( Obj, Field )		\
	SoapFieldVisitorCreate(					\
		#Field ,							\
		((size_t)(&reinterpret_cast<char const volatile&>(Obj.Field) - &reinterpret_cast<char const volatile&>(Obj))),	\
		Obj.Field							\
	)

#define SoapClassVistior( Klass ) SoapClassVisitorT<Klass>( #Klass )

class SoapTypeVisitor
{
public:
	SoapTypeVisitor(SoapServer* server)
		: m_server(server)
	{
	}

	template <typename T>
	SoapTypeVisitor& operator<<(SoapFieldVisitorT<T> info)
	{
		m_fields.push_back(&m_server->GetFieldBinding<T>(info.m_name, info.m_offset));
		return *this;
	}

	template <typename T>
	SoapTypeVisitor& operator<<(SoapClassVisitorT<T> info)
	{
		return *this;
	}

	vector<const FieldBinding*>& GetFields()
	{
		return m_fields;
	}

private:
	vector<const FieldBinding*> m_fields;
	SoapServer* m_server;
};



class SoapServer
{
public:
	SoapServer(int rpcPort, int mexPort);
	~SoapServer();

	void Start();
	void Stop();


	template <typename T>
	void BindProtobufInbound(const shared_ptr<T>& t)
	{
		BindProtobuf<T>(t, true);
	}

	template <typename T>
	shared_ptr<T> BindProtobufOutput()
	{
		BindProtobuf<T>(0, false);

		shared_ptr<T> stub(new T(GetRpcChannel()));
		m_protobufStubs.push_back(stub);
		return stub;
	}

	template <typename T>
	SoapNativeFunctionBinderInbound<T> BindNativeInbound(const string& serviceName, const shared_ptr<T>& t)
	{
		string name = serviceName;

		AddService(name);
		return SoapNativeFunctionBinderInbound<T>(this, t, name);
	}

  template <typename T>
  SoapNativeFunctionBinderOutbound<T> BindNativeOutbound(const string& serviceName, const shared_ptr<T>& t)
  {
    string name = serviceName;

    AddService(name);
    return SoapNativeFunctionBinderOutbound<T>(this, t, name);
  }

	template <typename T>
	void BindProtobuf(const shared_ptr<T>& t, bool isInput)
	{
		const ::google::protobuf::ServiceDescriptor* descriptor = T::descriptor();

		string service = descriptor->name();
		AddService(service);

		for (int x = 0; x < descriptor->method_count(); ++x)
		{
			const ::google::protobuf::MethodDescriptor* method = descriptor->method(x);

			string methodname = method->name();

			const ::google::protobuf::Descriptor* input = method->input_type();
			const ::google::protobuf::Descriptor* output = method->output_type();

			if (t)
			{
				ProtobufCallback callback = bind(&CallProtobuf<T>, t, method, placeholders::_1, placeholders::_2);
				AddMethod(service, methodname, GetClassBinding(input), GetClassBinding(output), callback, isInput);
			}
			else
			{
				AddMethod(service, methodname, GetClassBinding(input), GetClassBinding(output), ProtobufCallback(), isInput);
			}
		}
	}

	void AddService(string& serviceName);

	void AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput, bool isOneWay = false);
	void AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput, bool isOneWay = false);

	bool HasClassBinding(const type_info& type);
	const ClassBinding& GetClassBinding(const type_info& type);
	const ClassBinding& GetClassBinding(const ::google::protobuf::Descriptor* descriptor);
	
	void RegisterClassBinding(const type_info& type, vector<const FieldBinding*>& fields, CreateObjectCallback callback);

	const FieldBinding& GetFieldBinding(const string& name, const string& type, size_t offset, size_t size);

	template <typename T>
	const ClassBinding& GetClassBinding()
	{
		if (!HasClassBinding(typeid(T)))
		{
			T t;
			SoapTypeVisitor visitor(this);
			visitor << t;

			RegisterClassBinding(typeid(T), visitor.GetFields(), bind(&ObjectWrapper::Create<T>));
		}

		return GetClassBinding(typeid(T));
	}

  template <typename T>
  string GetSoapTypeName()
  {
    return typeid(T).name();
  }

  template <>
  string GetSoapTypeName<string>()
  {
    return "string";
  }

  template <>
  string GetSoapTypeName<bool>()
  {
    return "boolean";
  }

  template <>
  string GetSoapTypeName<uint64_t>()
  {
    return "unsignedLong";
  }

  template <>
  string GetSoapTypeName<uint32_t>()
  {
    return "unsignedInt";
  }

  template <>
  string GetSoapTypeName<int64_t>()
  {
    return "long";
  }

  template <>
  string GetSoapTypeName<int32_t>()
  {
    return "int";
  }

	template <typename T>
	const FieldBinding& GetFieldBinding(const string& name, size_t offset)
	{
		return GetFieldBinding(name, GetSoapTypeName<T>(), offset, sizeof(T));
	}

  void CallMethod(const string& serviceName, const string& methodName, const google::protobuf::Message& request, ::google::protobuf::Message* response, ::google::protobuf::Closure* done);
  void CallMethod(const string& serviceName, const string& methodName, const void* request, function<void(const void*)> callback, const type_info& reqType, const type_info& respType);

  ::google::protobuf::RpcChannel* GetRpcChannel();

private:
	vector<shared_ptr<::google::protobuf::Service> > m_protobufStubs;
	SoapServerInternal *m_internal;
};