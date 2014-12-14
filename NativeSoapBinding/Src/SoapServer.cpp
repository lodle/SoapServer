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

void SoapServer::Start(function<void()> idleCallback)
{
	m_internal->m_idleCallback = idleCallback;
	m_internal->GenerateWsdl();
	m_internal->Run();
}


void SoapServer::AddMethod(const string& service, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback invoke, bool isInput)
{
	m_internal->AddMethod(service, name, request, response, invoke, isInput);
}

const ClassBinding& SoapServer::GetClassBinding(const ::google::protobuf::Descriptor* descriptor)
{
	return m_internal->GetClassBinding(descriptor);
}

::google::protobuf::RpcChannel* SoapServer::GetRpcChannel()
{
	return m_internal;
}