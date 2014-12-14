#include "ServiceBinding.h"

#include "tinyxml2.h"

ServiceBinding::ServiceBinding()
{
}

ServiceBinding::ServiceBinding(const string& name)
	: m_name(name)
{
}

void ServiceBinding::AddBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback& callback, bool isInput)
{
	m_bindings[name] = FunctionBinding(name, request, response, callback, isInput);
}

void ServiceBinding::Invoke(const string& actionUrl, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* reqBody, tinyxml2::XMLElement* respBody)
{
	string action = actionUrl.substr(GetActionUrl().size() + 1);

	map<string, FunctionBinding>::iterator it = m_bindings.find(action);

	if (it == m_bindings.end())
	{
		throw std::exception("Action not found");
	}

	tinyxml2::XMLElement* respNode = it->second.Invoke(reqBody, respDoc);
	respBody->LinkEndChild(respNode);
}

string ServiceBinding::GetActionUrl() const
{
	return "http://Battle.net/I" + m_name;
}

vector<tinyxml2::XMLElement*> ServiceBinding::GenerateWsdl(tinyxml2::XMLDocument* doc)
{
	vector<tinyxml2::XMLElement*> res;

	map<string, FunctionBinding>::iterator fit = m_bindings.begin();

	for (; fit != m_bindings.end(); ++fit)
	{
		auto r = fit->second.GenerateWsdlMessage(string("I") + m_name, doc);

		for (size_t x = 0; x < r.size(); ++x)
		{
			res.push_back(r[x]);
		}
	}

	tinyxml2::XMLElement* portType = doc->NewElement("wsdl:portType");
	portType->SetAttribute("name", (string("I") + m_name).c_str());

	fit = m_bindings.begin();

	for (; fit != m_bindings.end(); ++fit)
	{
		portType->LinkEndChild(fit->second.GenerateWsdlOperaton(string("I") + m_name, doc));
	}

	res.push_back(portType);


	tinyxml2::XMLElement* binding = doc->NewElement("wsdl:binding");
	binding->SetAttribute("name", (m_name + "Service").c_str());
	binding->SetAttribute("type", (string("tns:I") + m_name).c_str());

	tinyxml2::XMLElement* transport = doc->NewElement("soap12:binding");
	transport->SetAttribute("transport", "http://schemas.microsoft.com/soap/tcp");
	binding->LinkEndChild(transport);

	fit = m_bindings.begin();

	for (; fit != m_bindings.end(); ++fit)
	{
		binding->LinkEndChild(fit->second.GenerateWsdlAction(string("I") + m_name, doc));
	}

	res.push_back(binding);

	tinyxml2::XMLElement* service = doc->NewElement("wsdl:service");
	service->SetAttribute("name", (m_name + "Service").c_str());

	tinyxml2::XMLElement* port = doc->NewElement("wsdl:port");
	port->SetAttribute("name", (m_name + "Service").c_str());
	port->SetAttribute("binding", (string("tns:") + m_name + "Service").c_str());

	tinyxml2::XMLElement* address = doc->NewElement("soap12:address");
	address->SetAttribute("location", (string("net.tcp://localhost:666/battlenet/") + m_name).c_str());

	port->LinkEndChild(address);
	service->LinkEndChild(port);
	res.push_back(service);

	return res;
}
