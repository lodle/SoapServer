#include "SoapServerInternal.h"
#include "Util.h"
#include "tinyxml2.h"

#include "ProtobufClassHelper.h"

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

class SoapTcpServer : 
	public Poco::Net::TCPServerConnection, 
	protected SoapWriteI
{
public:
	SoapTcpServer(const Poco::Net::StreamSocket& s, SoapServerInternal* internal)
		: Poco::Net::TCPServerConnection(s)
		, m_internal(internal)
	{
		m_internal->SetWriter(this);
	}

	~SoapTcpServer()
	{
		m_internal->SetWriter(0);
	}

	virtual void write(const char* data, size_t size)
	{
		
		int sent = socket().sendBytes(data, size);
		cout << "Sent " << sent << " of " << size << " bytes" << endl << flush;
	}

	void run()
	{
		cout << "New connection from: " << socket().peerAddress().host().toString() << endl << flush;

		bool isOpen = true;

		Poco::Timespan timeOut(10, 0);
		char buffer[16000];

		while (isOpen) 
		{
			if (socket().poll(timeOut, Poco::Net::Socket::SELECT_READ) == true) 
			{
				int nBytes = -1;

				try 
				{
					nBytes = socket().receiveBytes(buffer, sizeof(buffer));
					cout << "Recv " << nBytes << " bytes" << endl << flush;
					m_internal->OnRead(buffer, nBytes);
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

	SoapServerInternal* m_internal;
};


class MexHTTPRequestHandler : public Poco::Net::HTTPRequestHandler
{
public:
	MexHTTPRequestHandler(const string& wsdl)
		: m_wsdl(wsdl)
	{
	}

	void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
	{
		cout << "Sending wsdl"  << endl << flush;
		response.sendFile(m_wsdl, "text/xml");
	}

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
		return new MexHTTPRequestHandler(m_wsdl);
	}

	string m_wsdl;
};



SoapServerInternal::SoapServerInternal(int rpcPort, int mexPort)
	: m_rpcPort(rpcPort)
	, m_mexPort(mexPort)
	, m_wsdlPath("SoapServer.wsdl")
	, m_soapProtocol(*this)
	, m_writer(0)
{
	m_rawMessageHandlers[0x0] = bind(&SoapServerInternal::ProcessHeader, this, placeholders::_1, placeholders::_2);
	m_rawMessageHandlers[0x6] = bind(&SoapServerInternal::ProcessRequest, this, placeholders::_1, placeholders::_2);
	m_rawMessageHandlers[0x7] = bind(&SoapServerInternal::ProcessEnd, this, placeholders::_1, placeholders::_2);
}

int SoapServerInternal::Run()
{
	Poco::Net::ServerSocket mex(m_mexPort);
	Poco::Net::ServerSocket svs(m_rpcPort);

	//Configure some server params.
	Poco::Net::TCPServerParams* pTcpParams = new Poco::Net::TCPServerParams();
	pTcpParams->setMaxThreads(1);
	pTcpParams->setMaxQueued(1);

	Poco::Net::HTTPServerParams* pHttpParams = new Poco::Net::HTTPServerParams;
	pHttpParams->setMaxQueued(1);
	pHttpParams->setMaxThreads(1);

	//Create your server
	Poco::Net::TCPServer soapTcpServer(new TCPServerConnectionFactorySoap(this), svs, pTcpParams);
	Poco::Net::HTTPServer mexHttpServer(new MexHTTPRequestHandlerFactory(m_wsdlPath), mex, pHttpParams);

	soapTcpServer.start();
	mexHttpServer.start();

	m_running = true;

	while (m_running)
	{
		Sleep(1000);
	}

	soapTcpServer.stop();
	mexHttpServer.stop();
	return 0;
}


bool SoapServerInternal::OnRead(const char* buff, size_t size)
{
	size_t read = 0;

	while (read < size)
	{
		const char* b = buff + read;
		size_t s = size - read;

		map<char, function<size_t(const char* buff, size_t size)> >::iterator it = m_rawMessageHandlers.find(b[0]);

		if (it == m_rawMessageHandlers.end())
		{
			return false;
		}

		read += it->second(b, s);
	}

	if (m_idleCallback)
	{
		m_idleCallback();
	}

	return true;
}



size_t SoapServerInternal::ProcessHeader(const char* buff, size_t size)
{
	size_t buffidx = 0;

	_declspec(align(1)) struct VersionRecord
	{
		char RecordType;
		char MajorVersion;
		char MinorVersion;

		bool IsValid() const
		{
			return RecordType == 0x00 && MajorVersion == 0x01 && MinorVersion == 0x00;
		}
	};

	const struct VersionRecord* v = reinterpret_cast<const VersionRecord*>(buff);
	buffidx += 3;

	if (!v->IsValid())
	{
		assert(false);
		return buffidx;
	}

	_declspec(align(1)) struct ModeRecord
	{
		char RecordType;
		char Mode;

		bool IsValid() const
		{
			return RecordType == 0x01 && Mode == 0x02; //Duplex
		}
	};

	const struct ModeRecord* m = reinterpret_cast<const ModeRecord*>(buff + buffidx);
	buffidx += 2;

	if (!m->IsValid())
	{
		assert(false);
		return buffidx;
	}


	_declspec(align(1)) struct ViaRecord
	{
		char RecordType;
		mutable char ViaLength;

		bool IsValid() const
		{
			return RecordType == 0x02 && ViaLength != 0x00;
		}

		string GetUrl() const
		{
			size_t count = 0;
			size_t len = DecodePackedInt(&ViaLength, count) + count;
			return string(&ViaLength + count, len - count);
		}

		size_t GetSize() const
		{
			size_t count = 0;
			return 1 + DecodePackedInt(&ViaLength, count) + count;
		}
	};

	const struct ViaRecord* via = reinterpret_cast<const ViaRecord*>(buff + buffidx);
	buffidx += via->GetSize();

	if (!via->IsValid())
	{
		assert(false);
		return buffidx;
	}

	string url = via->GetUrl();


	_declspec(align(1)) struct EnvelopeEncodingRecord
	{
		char RecordType;
		char Encoding;
		char PreambleEnd;

		bool IsValid() const
		{
			return RecordType == 0x03 && Encoding == 0x0 && PreambleEnd == 0x0C; //UTF8
		}
	};

	const struct EnvelopeEncodingRecord* e = reinterpret_cast<const EnvelopeEncodingRecord*>(buff + buffidx);
	buffidx += 3;

	if (!e->IsValid())
	{
		assert(false);
		return buffidx;
	}

	_declspec(align(1)) struct PreambleAck
	{
		char RecordType;

		PreambleAck()
			: RecordType(0x0B)
		{
		}
	};

	struct PreambleAck p;
	write((char*)&p, 1);
	return buffidx;
}

size_t SoapServerInternal::ProcessEnd(const char* buff, size_t size)
{
	assert(buff[0] == 0x07);

	static char end[1] = { 0x07 };
	write(end, 1);
	return 1;
}

size_t SoapServerInternal::ProcessRequest(const char* buff, size_t size)
{
	assert(buff[0] == 0x06);

	_declspec(align(1)) struct SizedEnvelopRecord
	{
		char RecordType;
		char Size;

		bool IsValid() const
		{
			return RecordType == 0x06 && Size != 0x00;
		}

		string GetData() const
		{
			size_t count = 0;
			size_t len = DecodePackedInt(&Size, count) + count;
			return string(&Size + count, len - count);
		}

		size_t GetSize() const
		{
			size_t count = 0;
			return 1 + DecodePackedInt(&Size, count) + count;
		}
	};

	const struct SizedEnvelopRecord* e = reinterpret_cast<const SizedEnvelopRecord*>(buff);

	assert(e->IsValid());
	m_soapProtocol.HandleRequest(e->GetData(), m_soapMappings);
	return e->GetSize();
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

void SoapServerInternal::AddMethod(const string& service, const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput)
{
	map<string, ServiceBinding>::iterator it = m_soapMappings.find(service); //

	if (it == m_soapMappings.end())
	{
		ServiceBinding sm(service);
		sm.AddBinding(name, request, response, callback, isInput);
		m_soapMappings[service] = sm;
	}
	else
	{
		it->second.AddBinding(name, request, response, callback, isInput);
	}
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

		tinyxml2::XMLDocument doc;


		ClassBinding& binding = GetClassBinding(request->GetDescriptor());
		tinyxml2::XMLElement* resp = binding.GetProtobufHelper()->GenerateRequest(*request, doc);

		string actionUrl = it->second.GetActionUrl() + "/" + methodName;
		m_soapProtocol.SendRequest(actionUrl, doc, resp, bind(&SoapServerInternal::OnProtobufResponse, this, response, done, placeholders::_1));
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

void SoapServerInternal::write(const char* data, size_t size)
{
	if (m_writer)
	{
		m_writer->write(data, size);
	}
}

void SoapServerInternal::SetWriter(SoapWriteI* writer)
{
	m_writer = writer;
}