#include "ServiceBinding.h"

#include "tinyxml2.h"

ServiceBinding::ServiceBinding()
{
}

ServiceBinding::ServiceBinding(const string& name)
	: m_name(name)
{
}

void ServiceBinding::AddBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback& callback, bool isInput, bool isOneWay)
{
	m_bindings[name] = FunctionBinding(name, request, response, callback, isInput, isOneWay);
}

void ServiceBinding::AddBinding(const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback& callback, bool isInput, bool isOneWay)
{
	m_bindings[name] = FunctionBinding(name, request, response, callback, isInput, isOneWay);
}

void ServiceBinding::OnResponse(tinyxml2::XMLElement* respBody, function<void()> callback, tinyxml2::XMLElement* respNode)
{
  respBody->LinkEndChild(respNode);
  callback();
}

void ServiceBinding::Invoke(const string& actionUrl, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* reqBody, tinyxml2::XMLElement* respBody, function<void()> callback)
{
	string action = actionUrl.substr(GetActionUrl().size() + 1);

	map<string, FunctionBinding>::iterator it = m_bindings.find(action);

	if (it == m_bindings.end())
	{
		throw std::exception("Action not found");
	}

  it->second.Invoke(reqBody, respDoc, bind(&ServiceBinding::OnResponse, this, respBody, callback, placeholders::_1));
}

string ServiceBinding::GetActionUrl() const
{
	return "http://battle.net/I" + m_name;
}

vector<tinyxml2::XMLElement*> ServiceBinding::GenerateWsdlElements(tinyxml2::XMLDocument* doc)
{
  vector<tinyxml2::XMLElement*> res;

  map<string, FunctionBinding>::iterator fit = m_bindings.begin();

  for (; fit != m_bindings.end(); ++fit)
  {
    vector<tinyxml2::XMLElement*> t = fit->second.GenerateWsdlElements(doc);

    for (size_t x = 0; x < t.size(); ++x)
    {
      res.push_back(t[x]);
    }
  }

  return res;
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

	return res;
}


vector<tinyxml2::XMLElement*> ServiceBinding::GenerateWsdlPortTypeOperations(tinyxml2::XMLDocument* doc)
{
  vector<tinyxml2::XMLElement*> res;

  map<string, FunctionBinding>::iterator fit = m_bindings.begin();

  for (; fit != m_bindings.end(); ++fit)
  {
    res.push_back(fit->second.GenerateWsdlOperaton(string("I") + m_name, doc));
  }

  return res;
}

vector<tinyxml2::XMLElement*> ServiceBinding::GenerateWsdlBindingOperations(tinyxml2::XMLDocument* doc)
{
  vector<tinyxml2::XMLElement*> res;

  map<string, FunctionBinding>::iterator fit = m_bindings.begin();

  for (; fit != m_bindings.end(); ++fit)
  {
    res.push_back(fit->second.GenerateWsdlAction(string("I") + m_name, doc));
  }

  return res;
}

tinyxml2::XMLElement* ServiceBinding::GenerateWsdlPort(tinyxml2::XMLDocument* doc)
{
  tinyxml2::XMLElement* port = doc->NewElement("wsdl:port");
  port->SetAttribute("name", (m_name + "Service").c_str());
  port->SetAttribute("binding", (string("tns:") + m_name + "Service").c_str());

  tinyxml2::XMLElement* address = doc->NewElement("soap:address");
  address->SetAttribute("location", (string("net.tcp://localhost:666/battlenet/") + m_name).c_str());

  port->LinkEndChild(address);
  return port;
}