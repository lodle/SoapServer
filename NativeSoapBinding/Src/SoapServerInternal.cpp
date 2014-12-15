#include "SoapServerInternal.h"
#include "Util.h"
#include "tinyxml2.h"

#include "ProtobufClassHelper.h"
#include "NativeClassHelper.h"

#include "SoapTcpFrame.h"

static const char* s_Namespaces[] = {
	"xmlns:wsdl", "http://schemas.xmlsoap.org/wsdl/" ,
	"xmlns:wsam", "http://www.w3.org/2007/05/addressing/metadata" ,
	"xmlns:wsx", "http://schemas.xmlsoap.org/ws/2004/09/mex" ,
	"xmlns:wsap", "http://schemas.xmlsoap.org/ws/2004/08/addressing/policy" ,
	"xmlns:msc", "http://schemas.microsoft.com/ws/2005/12/wsdl/contract" ,
	"xmlns:wsp", "http://schemas.xmlsoap.org/ws/2004/09/policy" ,
	"xmlns:xsd", "http://www.w3.org/2001/XMLSchema" ,
	"xmlns:soap", "http://schemas.xmlsoap.org/wsdl/soap/" ,
	"xmlns:wsu", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd" ,
	"xmlns:soap12", "http://schemas.xmlsoap.org/wsdl/soap12/" ,
	"xmlns:soapenc", "http://schemas.xmlsoap.org/soap/encoding/" ,
	"xmlns:tns", "http://Battle.net" ,
	"xmlns:wsa10", "http://www.w3.org/2005/08/addressing" ,
	"xmlns:wsaw", "http://www.w3.org/2006/05/addressing/wsdl" ,
	"xmlns:wsa", "http://schemas.xmlsoap.org/ws/2004/08/addressing",
	0, 0
};


#define POCO_STATIC

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerParams.h"
#include "Poco/Net/TCPServerConnectionFactory.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Net/Socket.h"

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"


namespace
{
	string GetTypeName(const type_info& type)
	{
		string name = type.name();

		if (name.find("class ") == 0)
		{
			name = string("C") + name.substr(6);
		}

		return name;
	}
}

class SoapTcpServer : 
	public Poco::Net::TCPServerConnection, 
	protected SoapTcpConnectionI
{
public:
	SoapTcpServer(const Poco::Net::StreamSocket& s, SoapServerInternal* internal)
		: Poco::Net::TCPServerConnection(s)
		, m_internal(internal)
		, m_soapProtocol(*this)
		, m_soapTcpFrame(m_soapProtocol, *this)
		, m_stop(false)
	{
	}

	~SoapTcpServer()
	{
		m_internal->SetProtocolBinding(m_bindingName, 0);
	}

	virtual void SetBindingName(const string& name)
	{
		m_bindingName = name;
		m_internal->SetProtocolBinding(m_bindingName, &m_soapProtocol);
	}

	virtual void End()
	{
		m_stop = true;
		socket().shutdown();
	}

	virtual void Write(const char* data, size_t size)
	{
		int sent = socket().sendBytes(data, size);
		cout << "Sent " << sent << " of " << size << " bytes" << endl << flush;
	}

	virtual map<string, ServiceBinding>& GetServiceBindings()
	{
		return m_internal->GetServiceBindings();
	}

	virtual void run()
	{
		cout << "New connection from: " << socket().peerAddress().host().toString() << endl << flush;

		bool isOpen = true;

		Poco::Timespan timeOut(10, 0);
		char buffer[16000];

		while (isOpen && !m_stop)
		{
			if (socket().poll(timeOut, Poco::Net::Socket::SELECT_READ) == true) 
			{
				int nBytes = -1;

				try 
				{
					nBytes = socket().receiveBytes(buffer, sizeof(buffer));
					cout << "Recv " << nBytes << " bytes" << endl << flush;
					m_soapTcpFrame.OnRead(buffer, nBytes);
				}
				catch (Poco::Exception& exc) 
				{
					//Handle your network errors.
					cerr << "Network error: " << exc.displayText() << endl;
					isOpen = false;
				}
			}
		}

		cout << "Connection finished!" << endl << flush;
	}

private:
	SoapProtocol m_soapProtocol;
	SoapTcpFrame m_soapTcpFrame;
	SoapServerInternal* m_internal;
	string m_bindingName;

	bool m_stop;
};


class MexHTTPRequestHandler : public Poco::Net::HTTPRequestHandler, protected SoapTcpConnectionI
{
public:
	MexHTTPRequestHandler(const string& wsdl)
		: m_wsdl(wsdl)
		, m_soapProtocol(*this)
	{
	}

	virtual void SetBindingName(const string& name)
	{
		assert(false);
	}

	virtual void End()
	{
		assert(false);
	}

	virtual void Write(const char* data, size_t size)
	{
		m_outbuf += string(data, size);
	}

	virtual map<string, ServiceBinding>& GetServiceBindings()
	{
		assert(false);
		return *(map<string, ServiceBinding>*)(0);
	}

	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
	{
		response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_OK);

		if (request.getContentType() == "application/soap+xml")
		{
			string req(std::istreambuf_iterator<char>(request.stream()), std::istreambuf_iterator<char>());

			tinyxml2::XMLDocument reqDoc;
			reqDoc.Parse(req.c_str());

			tinyxml2::XMLDocument respDoc;
			respDoc.LoadFile(m_wsdl.c_str());

			cout << "Sending wsdl via soap" << endl << flush;

			m_outbuf = "";
			m_soapProtocol.SendResponse(reqDoc, respDoc, respDoc.FirstChildElement(), "http://www.w3.org/2005/08/addressing/anonymous");
			response.sendBuffer(m_outbuf.c_str(), m_outbuf.size());

			m_outbuf = "";
		}
		else
		{
			cout << "Sending wsdl via http" << endl << flush;
			response.sendFile(m_wsdl, "text/xml");
		}		
	}

	string m_outbuf;

	SoapProtocol m_soapProtocol;
	string m_wsdl;
};


class TCPServerConnectionFactorySoap : public Poco::Net::TCPServerConnectionFactory
{
public:
	TCPServerConnectionFactorySoap(SoapServerInternal* internal)
		: m_internal(internal)
	{
	}

	Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket)
	{
		return new SoapTcpServer(socket, m_internal);
	}

	SoapServerInternal* m_internal;
};

class MexHTTPRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
	MexHTTPRequestHandlerFactory(const string& wsdl)
		: m_wsdl(wsdl)
	{
	}

	Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
	{
		cout << "Http request from: " << request.getURI() << endl << flush;
		return new MexHTTPRequestHandler(m_wsdl);
	}

	string m_wsdl;
};



SoapServerInternal::SoapServerInternal(int rpcPort, int mexPort)
	: m_rpcPort(rpcPort)
	, m_mexPort(mexPort)
	, m_wsdlPath("SoapServer.wsdl")
{
}

void SoapServerInternal::Start()
{
	if (m_running)
	{
		assert(false);
		return;
	}

	Poco::Net::ServerSocket mex(m_mexPort);
	Poco::Net::ServerSocket svs(m_rpcPort);

	//Configure some server params.
	Poco::Net::TCPServerParams* pTcpParams = new Poco::Net::TCPServerParams();
	pTcpParams->setMaxThreads(16);
	pTcpParams->setMaxQueued(1);

	Poco::Net::HTTPServerParams* pHttpParams = new Poco::Net::HTTPServerParams;
	pHttpParams->setMaxThreads(4);

	//Create your server
	
	m_soapTcpServer = shared_ptr<Poco::Net::TCPServer>(new Poco::Net::TCPServer(new TCPServerConnectionFactorySoap(this), svs, pTcpParams));
	m_mexHttpServer = shared_ptr<Poco::Net::HTTPServer>(new Poco::Net::HTTPServer(new MexHTTPRequestHandlerFactory(m_wsdlPath), mex, pHttpParams));

	m_soapTcpServer->start();
	m_mexHttpServer->start();

	m_running = true;
}

void SoapServerInternal::Stop()
{
	if (!m_running)
	{
		assert(false);
		return;
	}

	m_soapTcpServer->stop();
	m_mexHttpServer->stop();
	m_running = false;
}

void SoapServerInternal::GenerateWsdl()
{
	tinyxml2::XMLDocument doc;

	auto root = doc.NewElement("wsdl:definitions");
	root->SetAttribute("name", "CalculatorService");
	root->SetAttribute("targetNamespace", "http://Battle.net");

	int x = 0;

	while (s_Namespaces[x])
	{
		root->SetAttribute(s_Namespaces[x], s_Namespaces[x + 1]);
		x += 2;
	};

	doc.LinkEndChild(root);

	auto types = doc.NewElement("wsdl:types");
	auto schema = doc.NewElement("xs:schema");

	schema->SetAttribute("elementFormDefault", "qualified");
	schema->SetAttribute("targetNamespace", "http://Battle.net");
	schema->SetAttribute("xmlns:xs", "http://www.w3.org/2001/XMLSchema");

	map<string, ClassBinding>::iterator it = m_classBindings.begin();

	for (; it != m_classBindings.end(); ++it)
	{
		schema->LinkEndChild(it->second.GenerateWsdl(&doc));
	}

	types->LinkEndChild(schema);
	root->LinkEndChild(types);

	map<string, ServiceBinding>::iterator sit = m_soapMappings.begin();

	for (; sit != m_soapMappings.end(); ++sit)
	{
		auto r = sit->second.GenerateWsdl(&doc);

		for (size_t x = 0; x < r.size(); ++x)
		{
			root->LinkEndChild(r[x]);
		}
	}

	doc.SaveFile(m_wsdlPath.c_str());
}

void SoapServerInternal::AddService(string& serviceName)
{
	string name = serviceName;
	char num = '1';

	map<string, ServiceBinding>::iterator it = m_soapMappings.find(serviceName);

	while (it != m_soapMappings.end())
	{
		serviceName = name + num;
		++num;
		it = m_soapMappings.find(name);
	}

	ServiceBinding sm(serviceName);
	m_soapMappings[serviceName] = sm;
}

void SoapServerInternal::AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput)
{
	map<string, ServiceBinding>::iterator it = m_soapMappings.find(serviceName);

	if (it != m_soapMappings.end())
	{
		it->second.AddBinding(name, request, response, callback, isInput);
	}
	else
	{
		assert(false);
	}
}

void SoapServerInternal::AddMethod(const string& serviceName, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput)
{
	map<string, ServiceBinding>::iterator it = m_soapMappings.find(serviceName);

	if (it != m_soapMappings.end())
	{
		it->second.AddBinding(name, request, response, callback, isInput);
	}
	else
	{
		assert(false);
	}
}


bool SoapServerInternal::HasClassBinding(const type_info& type)
{
	return m_classBindings.find(GetTypeName(type)) != m_classBindings.end();
}

const ClassBinding& SoapServerInternal::GetClassBinding(const type_info& type)
{
	assert(HasClassBinding(type));
	return m_classBindings[GetTypeName(type)];
}


void SoapServerInternal::RegisterClassBinding(const type_info& type, vector<const FieldBinding*>& fields, CreateObjectCallback callback)
{
	shared_ptr<NativeClassHelper> helper(new NativeClassHelper(callback));

	ClassBinding binding(GetTypeName(type), helper);

	for (size_t x = 0; x < fields.size(); ++x)
	{
		m_fieldBindings[fields[x]->GetName()] = *fields[x];
		binding.AddField(m_fieldBindings[fields[x]->GetName()]);
	}

	m_classBindings[GetTypeName(type)] = binding;
}


ClassBinding& SoapServerInternal::GetClassBinding(const ::google::protobuf::Descriptor* descriptor)
{
	if (m_classBindings.find(descriptor->full_name()) == m_classBindings.end())
	{
		ClassBinding binding(descriptor->name(), shared_ptr<ProtobufClassHelper>(new ProtobufClassHelper(descriptor)));

		if (descriptor->containing_type())
		{
			binding.SetParent(GetClassBinding(descriptor->containing_type()));
		}

		for (int x = 0; x < descriptor->field_count(); ++x)
		{
			binding.AddField(GetFieldBinding(descriptor->field(x)));
		}

		m_classBindings[descriptor->full_name()] = binding;
	}

	return m_classBindings[descriptor->full_name()];
}

const FieldBinding& SoapServerInternal::GetFieldBinding(const ::google::protobuf::FieldDescriptor* descriptor)
{
	if (m_fieldBindings.find(descriptor->full_name()) == m_fieldBindings.end())
	{
		FieldBinding binding(descriptor->name(), descriptor->type_name());
		m_fieldBindings[descriptor->full_name()] = binding;
	}

	return m_fieldBindings[descriptor->full_name()];
}

const FieldBinding& SoapServerInternal::GetFieldBinding(const string& name, const string& type, size_t offset, size_t size)
{
	if (m_fieldBindings.find(name) == m_fieldBindings.end())
	{
		FieldBinding binding(name, type, offset, size);
		m_fieldBindings[name] = binding;
	}

	return m_fieldBindings[name];
}

void SoapServerInternal::OnProtobufResponse(::google::protobuf::Message* response, ::google::protobuf::Closure* done, tinyxml2::XMLElement* respNode)
{
	assert(response->GetDescriptor()->name() == respNode->Value());

	ClassBinding& binding = GetClassBinding(response->GetDescriptor());
	binding.GetProtobufHelper()->FillProtobuf(response, respNode->FirstChildElement());

	if (done)
	{
		done->Run();
	}
}


void SoapServerInternal::CallMethod(const ::google::protobuf::MethodDescriptor* method,
	::google::protobuf::RpcController* controller,
	const ::google::protobuf::Message* request,
	::google::protobuf::Message* response,
	::google::protobuf::Closure* done)
{
	string serviceName = method->service()->name();
	string methodName = method->name();

	try
	{
		map<string, ServiceBinding>::iterator it = m_soapMappings.find(serviceName);

		if (it == m_soapMappings.end())
		{
			throw exception("Cant find service binding");
		}

		map<string, SoapProtocol*>::iterator sp = m_protocolBindings.find(serviceName);

		if (sp == m_protocolBindings.end())
		{
			throw exception("Cant find protocol binding");
		}

		tinyxml2::XMLDocument doc;


		ClassBinding& binding = GetClassBinding(request->GetDescriptor());
		tinyxml2::XMLElement* resp = binding.GetProtobufHelper()->GenerateRequest(*request, doc);

		string actionUrl = it->second.GetActionUrl() + "/" + methodName;
		sp->second->SendRequest(actionUrl, doc, resp, bind(&SoapServerInternal::OnProtobufResponse, this, response, done, placeholders::_1));
	}
	catch (exception &e)
	{
		if (controller)
		{
			controller->SetFailed(e.what());
			controller->Failed();
		}
	}
	catch (...)
	{
		if (controller)
		{
			controller->SetFailed("Unknown");
			controller->Failed();
		}
	}
}

void SoapServerInternal::SetProtocolBinding(const string& url, SoapProtocol* binding)
{
	m_protocolBindings[url] = binding;
}

map<string, ServiceBinding>& SoapServerInternal::GetServiceBindings()
{
	return m_soapMappings;
}


