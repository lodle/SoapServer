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

SoapServerInternal::SoapServerInternal(int rpcPort, int mexPort)
	: m_rpcPort(rpcPort)
	, m_mexPort(mexPort)
	, m_wsdlPath("SoapServer.wsdl")
	, m_soapProtocol(*this)
{
	soap_init2(&soap, SOAP_IO_KEEPALIVE, SOAP_IO_KEEPALIVE);

	soap.user = this;
	soap.port = m_rpcPort;
	soap.fget = http_get;
	soap.cookie_domain = "localhost";
	soap.cookie_path = "/";
}

int SoapServerInternal::Run()
{
	int master = soap_bind(&soap, "localhost", soap.port, 100);

	if (master < 0)
	{
		soap_print_fault(&soap, stderr);
		return -1;
	}

	fprintf(stderr, "Socket connection success\n");

	while (1)
	{
		int client = soap_accept(&soap);

		if (client < 0)
		{
			soap_print_fault(&soap, stderr);
			return -2;
		}

		fprintf(stderr, "Client connected\n");
		soap_serve(&soap);
		soap_end(&soap);
	}

	return 0;
}


int SoapServerInternal::soap_serve(struct soap *soap)
{
	unsigned int k = soap->max_keep_alive;

	do
	{
		if (soap->max_keep_alive > 0 && !--k)
			soap->keep_alive = 0;

		soap_begin(soap);

		if (soap_begin_recv(soap))
		{
			if (soap->error >= SOAP_STOP)
				continue;

			return soap->error;
		}

		soap->bufidx = 0;

		if (!ProcessHeader())
		{
			return SOAP_ERR;
		}

		if (soap_serve_request() || (soap->fserveloop && soap->fserveloop(soap)))
		{
			return soap_send_fault(soap);
		}

	} while (soap->keep_alive);

	return SOAP_OK;
}



bool SoapServerInternal::ProcessHeader()
{
	_declspec(align(1)) struct VersionRecord
	{
		char RecordType;
		char MajorVersion;
		char MinorVersion;

		bool IsValid()
		{
			return RecordType == 0x00 && MajorVersion == 0x01 && MinorVersion == 0x00;
		}
	};

	struct VersionRecord* v = reinterpret_cast<VersionRecord*>(soap.buf + soap.bufidx);
	soap.bufidx += 3;

	if (!v->IsValid())
	{
		return false;
	}

	_declspec(align(1)) struct ModeRecord
	{
		char RecordType;
		char Mode;

		bool IsValid()
		{
			return RecordType == 0x01 && Mode == 0x02; //Duplex
		}
	};

	struct ModeRecord* m = reinterpret_cast<ModeRecord*>(soap.buf + soap.bufidx);
	soap.bufidx += 2;

	if (!m->IsValid())
	{
		return false;
	}


	_declspec(align(1)) struct ViaRecord
	{
		char RecordType;
		char ViaLength;

		bool IsValid()
		{
			return RecordType == 0x02 && ViaLength != 0x00;
		}

		string GetUrl()
		{
			size_t count = 0;
			size_t len = DecodePackedInt(&ViaLength, count) + count;
			return string(&ViaLength + count, len - 1);
		}

		size_t GetSize()
		{
			size_t count = 0;
			return 1 + DecodePackedInt(&ViaLength, count) + count;
		}
	};

	struct ViaRecord* via = reinterpret_cast<ViaRecord*>(soap.buf + soap.bufidx);
	soap.bufidx += via->GetSize();

	if (!via->IsValid())
	{
		return false;
	}

	string url = via->GetUrl();


	_declspec(align(1)) struct EnvelopeEncodingRecord
	{
		char RecordType;
		char Encoding;

		bool IsValid()
		{
			return RecordType == 0x03; //UTF8
		}
	};

	struct EnvelopeEncodingRecord* e = reinterpret_cast<EnvelopeEncodingRecord*>(soap.buf + soap.bufidx);
	soap.bufidx += 2;

	if (!e->IsValid())
	{
		return false;
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
	return soap_send_raw(&soap, (char*)&p, 1) == SOAP_OK;
}

int SoapServerInternal::soap_serve_request()
{
	while (true)
	{
		if (m_idleCallback)
		{
			m_idleCallback();
		}

		if (soap_begin_recv(&soap))
		{
			if (soap.error >= SOAP_STOP)
				continue;

			return soap.error;
		}

		soap.bufidx = 0;

		while (soap.bufidx < soap.buflen)
		{
			assert(soap.buf[0] == 0x06);

			_declspec(align(1)) struct SizedEnvelopRecord
			{
				char RecordType;
				char Size;

				bool IsValid()
				{
					return RecordType == 0x06 && Size != 0x00;
				}

				bool IsEndRecord()
				{
					return RecordType == 0x07;
				}

				string GetData()
				{
					size_t count = 0;
					size_t len = DecodePackedInt(&Size, count) + count;
					return string(&Size + count, len - 1);
				}

				size_t GetSize()
				{
					size_t count = 0;
					return 1 + DecodePackedInt(&Size, count) + count;
				}
			};

			struct SizedEnvelopRecord* e = reinterpret_cast<SizedEnvelopRecord*>(soap.buf + soap.bufidx);

			if (e->IsEndRecord())
			{
				static char end[1] = { 0x07 };
				soap_send_raw(&soap, end, 1);
				soap.keep_alive = 0;
				return 0;
			}

			soap.bufidx += e->GetSize();

			assert(e->IsValid());

			m_soapProtocol.HandleRequest(e->GetData(), m_soapMappings);

			string data = e->GetData();

		}
	}

	return soap.error = SOAP_NO_METHOD;
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

const ClassBinding& SoapServerInternal::GetClassBinding(const ::google::protobuf::Descriptor* descriptor)
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


int SoapServerInternal::http_get(struct soap *soap)
{
	return static_cast<SoapServerInternal*>(soap->user)->http_get();
}

int SoapServerInternal::http_get()
{
	FILE *fd = NULL;
	char *s = strchr(soap.path, '?');

	if (!s || strcmp(s, "?wsdl"))
		return SOAP_GET_METHOD;

	fd = fopen(m_wsdlPath.c_str(), "rb"); // open WSDL file to copy 
	if (!fd)
		return 404; // return HTTP not found error 
	soap.http_content = "text/xml"; // HTTP header with text/xml content 
	soap_response(&soap, SOAP_FILE);

	for (;;)
	{
		size_t r = fread(soap.tmpbuf, 1, sizeof(soap.tmpbuf), fd);
		if (!r)
			break;
		if (soap_send_raw(&soap, soap.tmpbuf, r))
			break; // can't send, but little we can do about that 
	}
	fclose(fd);
	soap_end_send(&soap);
	return SOAP_OK;
}



void SoapServerInternal::CallMethod(const ::google::protobuf::MethodDescriptor* method,
	::google::protobuf::RpcController* controller,
	const ::google::protobuf::Message* request,
	::google::protobuf::Message* response,
	::google::protobuf::Closure* done)
{



}

void SoapServerInternal::write(const char* data, size_t size)
{
	soap_send_raw(&soap, data, size);
}