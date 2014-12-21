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

#include "NativeSoapServer.h"
#include "ProtobufSoapServer.h"



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
	const FieldBinding& GetFieldBinding(const string& name, size_t offset)
	{
    //dont support pointers, use inline classes
    assert(!is_pointer<T>::value);

    string typeName = GetSoapTypeName<T>();

    if (typeName.find("class") == 0)
    {
      GetClassBinding<T>();
    }

		return GetFieldBinding(name, typeName, offset, sizeof(T));
	}

  void CallMethod(const string& serviceName, const string& methodName, const google::protobuf::Message& request, ::google::protobuf::Message* response, ::google::protobuf::Closure* done);
  void CallMethod(const string& serviceName, const string& methodName, const void* request, function<void(const void*)> callback, const type_info& reqType, const type_info& respType);


private:
	vector<shared_ptr<::google::protobuf::Service> > m_protobufStubs;
	SoapServerInternal *m_internal;
};