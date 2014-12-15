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


typedef function<void(const google::protobuf::Message&, google::protobuf::Message&)> ProtobufCallback;
typedef function<void(void*, void*)> NativeCallback;
typedef function<void*()> CreateObjectCallback;

namespace
{
	template <typename T>
	void CallProtobuf(shared_ptr<T> t, const ::google::protobuf::MethodDescriptor* method, const google::protobuf::Message& req, google::protobuf::Message& resp)
	{
		::google::protobuf::RpcController* controller = 0;
		::google::protobuf::Closure* closure = 0;

		t->CallMethod(method, controller, &req, &resp, closure);
	}

	template <typename T, typename A, typename R>
	void CallNative(shared_ptr<T> t, void (T::*funct)(const A&, R&), void* aIn, void* rOut)
	{
		const A& a = *reinterpret_cast<const A*>(aIn);
		R& r = *reinterpret_cast<R*>(rOut);
		(*t.get().*funct)(a, r);
	}

	template <typename T>
	void* CreateObject()
	{
		return new T();
	}
}




template <typename T>
class SoapNativeFunctionBinder
{
public:
	SoapNativeFunctionBinder(SoapServer* server, const shared_ptr<T>& t, const string& serviceName, bool input)
		: m_server(server)
		, m_serviceName(serviceName)
		, m_t(t)
		, m_input(input)
	{
	}

	template <typename A, typename R>
	SoapNativeFunctionBinder<T>& Function(const string& name, void (T::*funct)(const A&, R&))
	{
		const ClassBinding& request = m_server->GetClassBinding<A>();
		const ClassBinding& response = m_server->GetClassBinding<R>();

		NativeCallback callback = bind(&CallNative<T, A, R>, m_t, funct, placeholders::_1, placeholders::_2);
		m_server->AddMethod(m_serviceName, name, request, response, callback, m_input);

		return *this;
	}

private:
	SoapServer* m_server;
	string m_serviceName;
	shared_ptr<T> m_t;
	bool m_input;
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

class VoidRequestResponse
{
};

inline SoapTypeVisitor& operator<<(SoapTypeVisitor& visitor, const VoidRequestResponse& r)
{
	visitor << SoapClassVistior(VoidRequestResponse);
	return visitor;
}


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
	SoapNativeFunctionBinder<T> BindNativeInbound(const string& serviceName, const shared_ptr<T>& t)
	{
		string name = serviceName;

		AddService(name);
		return SoapNativeFunctionBinder<T>(this, t, name, true);
	}

	template <typename I, typename T>
	SoapNativeFunctionBinder<T> BindNativeOutbound(const string& serviceName, shared_ptr<T>& t)
	{
		string name = serviceName;

		AddService(name);
		return SoapNativeFunctionBinder<T>(this, t, name, false);
	}


	::google::protobuf::RpcChannel* GetRpcChannel();

protected:
	 template <typename T>
	 friend class SoapNativeFunctionBinder;

	 friend class SoapTypeVisitor;

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

	void AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput);
	void AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput);

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

			RegisterClassBinding(typeid(T), visitor.GetFields(), bind(&CreateObject<T>));
		}

		return GetClassBinding(typeid(T));
	}

	template <typename T>
	const FieldBinding& GetFieldBinding(const string& name, size_t offset)
	{
		return GetFieldBinding(name, typeid(T).name(), offset, sizeof(T));
	}

private:
	vector<shared_ptr<::google::protobuf::Service> > m_protobufStubs;
	SoapServerInternal *m_internal;
};