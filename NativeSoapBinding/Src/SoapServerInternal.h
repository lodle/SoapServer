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

	void AddService(string& serviceName);

	void AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput, bool isOneWay);
	void AddMethod(const string& service, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput, bool isOneWay);

	bool HasClassBinding(const type_info& type);
	const ClassBinding& GetClassBinding(const type_info& type);

  bool HasClassBinding(const string& type);
  const ClassBinding& GetClassBinding(const string& type);

	ClassBinding& GetClassBinding(const ::google::protobuf::Descriptor* descriptor);
	const FieldBinding& GetFieldBinding(const ::google::protobuf::FieldDescriptor* descriptor);
	const FieldBinding& GetFieldBinding(const string& name, const string& type, size_t offset, size_t size);

	void RegisterClassBinding(const type_info& type, vector<const FieldBinding*>& fields, CreateObjectCallback callback);


	virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
		::google::protobuf::RpcController* controller,
		const ::google::protobuf::Message* request,
		::google::protobuf::Message* response,
		::google::protobuf::Closure* done);

  void CallMethod(const string& serviceName, const string& methodName, const google::protobuf::Message& request, ::google::protobuf::Message* response, ::google::protobuf::Closure* done);
  void CallMethod(const string& serviceName, const string& methodName, const void* request, function<void(const void*)> callback, const type_info& reqType, const type_info& respType);

	void SetProtocolBinding(const string& url, SoapProtocol* binding);

	map<string, ServiceBinding>& GetServiceBindings();

protected:
  void CallMethod(const string& serviceName, const string& methodName, const google::protobuf::Message& request, SoapProtocol::ResponseCallback callback);

  void OnNativeResponse(TypeWrapper type, function<void(const void*)> callback, tinyxml2::XMLElement* respNode);
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

	mutex m_protocolLock;
};
