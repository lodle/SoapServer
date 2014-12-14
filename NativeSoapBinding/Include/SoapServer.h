#pragma once

class SoapServerInternal;
class ClassBinding;

#include <string>
#include <functional>
#include <memory>

using namespace std;

#include <google/protobuf/message.h>

typedef function<void(const google::protobuf::Message&, google::protobuf::Message&)> ProtobufCallback;

namespace
{
	template <typename T>
	static void CallProtobuf(shared_ptr<T> t, const ::google::protobuf::MethodDescriptor* method, const google::protobuf::Message& req, google::protobuf::Message& resp)
	{
		::google::protobuf::RpcController* controller = 0;
		::google::protobuf::Closure* closure = 0;

		t->CallMethod(method, controller, &req, &resp, closure);
	}
}

namespace google
{
	namespace protobuf
	{
		class RpcChannel;
		class Service;
	}
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

	::google::protobuf::RpcChannel* GetRpcChannel();

protected:
	template <typename T>
	void BindProtobuf(const shared_ptr<T>& t, bool isInput)
	{
		const ::google::protobuf::ServiceDescriptor* descriptor = T::descriptor();

		string service = descriptor->name();

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


	void AddMethod(const string& service, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput);
	const ClassBinding& GetClassBinding(const ::google::protobuf::Descriptor* descriptor);

private:
	vector<shared_ptr<::google::protobuf::Service> > m_protobufStubs;
	SoapServerInternal *m_internal;
};