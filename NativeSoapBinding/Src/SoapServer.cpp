#include "SoapServer.h"
#include "SoapServerInternal.h"


SoapServer::SoapServer(int rpcPort, int mexPort)
	: m_internal(new SoapServerInternal(rpcPort, mexPort))
{

}

SoapServer::~SoapServer()
{
	delete m_internal;
}

void SoapServer::Start()
{
	m_internal->GenerateWsdl();
	m_internal->Start();
}

void SoapServer::Stop()
{
	m_internal->Stop();
}

void SoapServer::AddService(string& serviceName)
{
	m_internal->AddService(serviceName);
}

void SoapServer::AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput)
{
	m_internal->AddMethod(serviceName, name, request, response, callback, isInput);
}

void SoapServer::AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput)
{
	m_internal->AddMethod(serviceName, name, request, response, callback, isInput);
}

bool SoapServer::HasClassBinding(const type_info& type)
{
	return m_internal->HasClassBinding(type);
}

const ClassBinding& SoapServer::GetClassBinding(const type_info& type)
{
	return m_internal->GetClassBinding(type);
}


const ClassBinding& SoapServer::GetClassBinding(const ::google::protobuf::Descriptor* descriptor)
{
	return m_internal->GetClassBinding(descriptor);
}

void SoapServer::RegisterClassBinding(const type_info& type, vector<const FieldBinding*>& fields, CreateObjectCallback callback)
{
	m_internal->RegisterClassBinding(type, fields, callback);
}

::google::protobuf::RpcChannel* SoapServer::GetRpcChannel()
{
	return m_internal;
}

const FieldBinding& SoapServer::GetFieldBinding(const string& name, const string& type, size_t offset, size_t size)
{
	return m_internal->GetFieldBinding(name, type, offset, size);
}