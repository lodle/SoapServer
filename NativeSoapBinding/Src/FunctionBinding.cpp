#include "FunctionBinding.h"

#include "tinyxml2.h"

#include "ProtobufClassHelper.h"
#include "NativeClassHelper.h"

void Log(const string& data);

FunctionBinding::FunctionBinding()
{
}

FunctionBinding::FunctionBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput, bool isOneWay)
	: m_name(name)
	, m_requestBinding(request)
	, m_responseBinding(response)
	, m_protobufCallback(callback)
	, m_isInput(isInput)
  , m_isOneWay(isOneWay)
{
  //m_isOneWay = false;
}

FunctionBinding::FunctionBinding(const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput, bool isOneWay)
	: m_name(name)
	, m_requestBinding(request)
	, m_responseBinding(response)
	, m_nativeCallback(callback)
	, m_isInput(isInput)
  , m_isOneWay(isOneWay)
{
  //m_isOneWay = false;
}

void FunctionBinding::OnResponseProtobuf(tinyxml2::XMLDocument* respDoc, function<void(tinyxml2::XMLElement*)> callback, const google::protobuf::Message& response)
{
  Log(string("<ProtobufInboundRes>") + response.DebugString() + "</ProtobufInboundRes>");

  vector<tinyxml2::XMLElement*> nodes = m_responseBinding.GetProtobufHelper()->GenerateResponse(response, *respDoc);

  tinyxml2::XMLElement* el = respDoc->NewElement((m_name + "Response").c_str());
  el->SetAttribute("xmlns", "http://battle.net/types");

  for (size_t x = 0; x < nodes.size(); ++x)
  {
    el->LinkEndChild(nodes[x]);
  }

  callback(el);
}

void FunctionBinding::OnResponseNative(tinyxml2::XMLDocument* respDoc, function<void(tinyxml2::XMLElement*)> callback, const void* obj)
{
  vector<tinyxml2::XMLElement*> nodes = m_responseBinding.GetNativeHelper()->GenerateResponse(obj, *respDoc);

  string postfix = "Output";

  if (!m_isInput)
  {
    postfix = "Input";
  }

  tinyxml2::XMLElement* el = respDoc->NewElement((m_name + "Response").c_str());
  el->SetAttribute("xmlns", "http://battle.net/types");

  for (size_t x = 0; x < nodes.size(); ++x)
  {
    el->LinkEndChild(nodes[x]);
  }

  callback(el);
}

void FunctionBinding::Invoke(tinyxml2::XMLElement* req, tinyxml2::XMLDocument& respDoc, function<void(tinyxml2::XMLElement*)> callback)
{
	if (!m_isInput)
	{
		throw std::exception("Invoke not supported on FunctionBinding");
	}

	if (m_protobufCallback)
	{
		shared_ptr<google::protobuf::Message> request = m_requestBinding.GetProtobufHelper()->CreateProtobuf(req);

    Log(string("<ProtobufInboundReq>") + request->DebugString() + "</ProtobufInboundReq>");

    if (m_isOneWay)
    {
      m_protobufCallback(*(request.get()), ProtobufResponseCallback());
    }
    else
    {
      m_protobufCallback(*(request.get()), bind(&FunctionBinding::OnResponseProtobuf, this, &respDoc, callback, placeholders::_1));
    }
	}
	else if (m_nativeCallback)
	{
		shared_ptr<ObjectWrapper> request = m_requestBinding.GetNativeHelper()->CreateObject(req);

    if (m_isOneWay)
    {
      m_nativeCallback(request.get()->get(), NativeResponseCallback());
    }
    else
    {
      m_nativeCallback(request.get()->get(), bind(&FunctionBinding::OnResponseNative, this, &respDoc, callback, placeholders::_1));
    }
	}
	else
	{
		throw std::exception("Invoke not supported on FunctionBinding");
	}
}

vector<tinyxml2::XMLElement*> FunctionBinding::GenerateWsdlElements(tinyxml2::XMLDocument* doc)
{
  vector<tinyxml2::XMLElement*> res;

  {
    auto el = doc->NewElement("xs:element");
    el->SetAttribute("name", m_name.c_str());
    el->SetAttribute("type", (string("bntypes:") + m_requestBinding.GetName()).c_str());
    res.push_back(el);
  }

  if (!m_isOneWay)
  {
    auto el = doc->NewElement("xs:element");
    el->SetAttribute("name", (m_name + "Response").c_str());
    el->SetAttribute("type", (string("bntypes:") + m_responseBinding.GetName()).c_str());
    res.push_back(el);
  }

  return res;
}

vector<tinyxml2::XMLElement*> FunctionBinding::GenerateWsdlMessage(const string& prefix, tinyxml2::XMLDocument* doc)
{
	vector<tinyxml2::XMLElement*> ret;

	{
		tinyxml2::XMLElement* message = doc->NewElement("wsdl:message");
		message->SetAttribute("name", GetRequestMessageName(prefix, false).c_str());

		tinyxml2::XMLElement* part = doc->NewElement("wsdl:part");
		part->SetAttribute("name", "parameters");
		part->SetAttribute("element", (string("bntypes:") + m_name).c_str());

		message->LinkEndChild(part);
		ret.push_back(message);
	}

  if (!m_isOneWay)
  {
		tinyxml2::XMLElement* message = doc->NewElement("wsdl:message");
		message->SetAttribute("name", GetResponseMessageName(prefix, false).c_str());

		tinyxml2::XMLElement* part = doc->NewElement("wsdl:part");
		part->SetAttribute("name", "parameters");
		part->SetAttribute("element", (string("bntypes:") + m_name + "Response").c_str());

		message->LinkEndChild(part);
		ret.push_back(message);
	}

	return ret;
}

string FunctionBinding::GetRequestMessageName(const string& prefix, bool withNamespace)
{
  string name = prefix + m_name + m_requestBinding.GetName() + "Request";

  if (!withNamespace)
  {
    return name;
  }

  return string("tns:") + name;
}

string FunctionBinding::GetResponseMessageName(const string& prefix, bool withNamespace)
{
  string name = prefix + m_name + m_responseBinding.GetName() + "Response";

  if (!withNamespace)
  {
    return name;
  }

  return string("tns:") + name;
}

tinyxml2::XMLElement* FunctionBinding::GenerateWsdlOperaton(const string& prefix, tinyxml2::XMLDocument* doc)
{
	tinyxml2::XMLElement* operation = doc->NewElement("wsdl:operation");
	operation->SetAttribute("name", (prefix + "_" + m_name).c_str());

	tinyxml2::XMLElement* input;
	tinyxml2::XMLElement* output;

	if (m_isInput)
	{
		input = doc->NewElement("wsdl:input");
    input->SetAttribute("name", (m_name + "Input").c_str());

		output = doc->NewElement("wsdl:output");
    output->SetAttribute("name", (m_name + "Output").c_str());
	}
	else
	{
		input = doc->NewElement("wsdl:output");
    input->SetAttribute("name", (m_name + "Output").c_str());

		output = doc->NewElement("wsdl:input");
    output->SetAttribute("name", (m_name + "Input").c_str());
	}

  input->SetAttribute("message", GetRequestMessageName(prefix, true).c_str());
	input->SetAttribute("wsaw:Action", (string("http://battle.net/") + prefix + "/" + m_name).c_str());
	
  output->SetAttribute("message", GetResponseMessageName(prefix, true).c_str());
	output->SetAttribute("wsaw:Action", (string("http://battle.net/") + prefix + "/" + m_name + "Response").c_str());
	
	operation->LinkEndChild(input);

  if (!m_isOneWay)
  {
    operation->LinkEndChild(output);
  }
  else
  {
    doc->DeleteNode(output);
  }
	
	return operation;
}

tinyxml2::XMLElement* FunctionBinding::GenerateWsdlAction(const string& prefix, tinyxml2::XMLDocument* doc)
{
	tinyxml2::XMLElement* operation = doc->NewElement("wsdl:operation");
	operation->SetAttribute("name", (prefix + "_" + m_name).c_str());

	//tinyxml2::XMLElement* action = doc->NewElement("soap:operation");
	//action->SetAttribute("soapAction", (string("http://battle.net/") + prefix + "/" + m_name).c_str());
	//action->SetAttribute("style", "document");
	//operation->LinkEndChild(action);

  tinyxml2::XMLElement* input;
  tinyxml2::XMLElement* output;

  if (m_isInput)
  {
    input = doc->NewElement("wsdl:input");
    input->SetAttribute("name", (m_name + "Input").c_str());

    output = doc->NewElement("wsdl:output");
    output->SetAttribute("name", (m_name + "Output").c_str());
  }
  else
  {
    input = doc->NewElement("wsdl:output");
    input->SetAttribute("name", (m_name + "Output").c_str());

    output = doc->NewElement("wsdl:input");
    output->SetAttribute("name", (m_name + "Input").c_str());
  }

	{
		tinyxml2::XMLElement* body = doc->NewElement("soap:body");
		body->SetAttribute("use", "literal");
		input->LinkEndChild(body);
	}

	{
		tinyxml2::XMLElement* body = doc->NewElement("soap:body");
		body->SetAttribute("use", "literal");
		output->LinkEndChild(body);
	}

  operation->LinkEndChild(input);

  if (!m_isOneWay)
  {
    operation->LinkEndChild(output);
  }
  else
  {
    doc->DeleteNode(output);
  }

	return operation;
}
