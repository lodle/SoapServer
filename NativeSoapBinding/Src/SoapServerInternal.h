#pragma once

#include "std.h"

#include "SoapServer.h"

#include "ServiceBinding.h"
#include "FieldBinding.h"
#include "ClassBinding.h"


#include <google/protobuf/service.h>

#include "SoapProtocol.h"
#include "SoapInterfaces.h"

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

namespace Poco
{
	namespace Net
	{
		class TCPServer;
		class HTTPServer;
	}
}

class SoapServerInternal : public ::google::protobuf::RpcChannel
{
public:
	SoapServerInternal(int rpcPort, int mexPort);

	void Start();
	void Stop();

	void GenerateWsdl();

	void AddMethod(const string& service, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput);
	ClassBinding& GetClassBinding(const ::google::protobuf::Descriptor* descriptor);
	const FieldBinding& GetFieldBinding(const ::google::protobuf::FieldDescriptor* descriptor);

	virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
		::google::protobuf::RpcController* controller,
		const ::google::protobuf::Message* request,
		::google::protobuf::Message* response,
		::google::protobuf::Closure* done);


	void SetProtocolBinding(const string& url, SoapProtocol* binding);

	map<string, ServiceBinding>& GetServiceBindings();

protected:
	void OnProtobufResponse(::google::protobuf::Message* response, ::google::protobuf::Closure* done, tinyxml2::XMLElement* respNode);

	size_t ProcessHeader(const char* buff, size_t size);
	size_t ProcessEnd(const char* buff, size_t size);
	size_t ProcessRequest(const char* buff, size_t size);

private:
	map<string, ServiceBinding> m_soapMappings;
	map<string, ClassBinding> m_classBindings;
	map<string, FieldBinding> m_fieldBindings;

	map<string, SoapProtocol*> m_protocolBindings;

	int m_rpcPort;
	int m_mexPort;

	bool m_running;

	string m_wsdlPath;

	shared_ptr<Poco::Net::TCPServer> m_soapTcpServer;
	shared_ptr<Poco::Net::HTTPServer> m_mexHttpServer;
};
