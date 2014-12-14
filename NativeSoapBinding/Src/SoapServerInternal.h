#pragma once

#include "std.h"

#include "SoapServer.h"

#include "ServiceBinding.h"
#include "FieldBinding.h"
#include "ClassBinding.h"


#include <google/protobuf/service.h>
#include "soapH.h"

#include "SoapProtocol.h"
#include "SoapInterfaces.h"

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

class SoapServerInternal 
	: public ::google::protobuf::RpcChannel
	, protected SoapWriteI
{
public:
	SoapServerInternal(int rpcPort, int mexPort);

	int Run();

	void GenerateWsdl();

	void AddMethod(const string& service, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput);
	const ClassBinding& GetClassBinding(const ::google::protobuf::Descriptor* descriptor);
	const FieldBinding& GetFieldBinding(const ::google::protobuf::FieldDescriptor* descriptor);

	virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
		::google::protobuf::RpcController* controller,
		const ::google::protobuf::Message* request,
		::google::protobuf::Message* response,
		::google::protobuf::Closure* done);

	function<void()> m_idleCallback;

protected:
	int soap_serve(struct soap *soap);

	bool ProcessHeader();

	int soap_serve_request();

	static int http_get(struct soap *soap);
	int http_get();

	virtual void write(const char* data, size_t size);

private:
	struct soap soap;

	map<string, ServiceBinding> m_soapMappings;
	map<string, ClassBinding> m_classBindings;
	map<string, FieldBinding> m_fieldBindings;

	int m_rpcPort;
	int m_mexPort;

	string m_wsdlPath;

	SoapProtocol m_soapProtocol;
};
