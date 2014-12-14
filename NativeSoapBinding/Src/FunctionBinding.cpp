#include "FunctionBinding.h"

#include "tinyxml2.h"

#include "ProtobufClassHelper.h"


FunctionBinding::FunctionBinding()
{
}

FunctionBinding::FunctionBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput)
	: m_name(name)
	, m_requestBinding(request)
	, m_responseBinding(response)
	, m_protobufCallback(callback)
	, m_isInput(isInput)
{
}

tinyxml2::XMLElement* FunctionBinding::Invoke(tinyxml2::XMLElement* req, tinyxml2::XMLDocument& respDoc)
{
	if (!m_isInput)
	{
		throw std::exception("Invoke not supported on FunctionBinding");
	}

	if (m_protobufCallback)
	{
		shared_ptr<google::protobuf::Message> request = m_requestBinding.GetProtobufHelper()->CreateProtobuf(req);
		shared_ptr<google::protobuf::Message> response = m_responseBinding.GetProtobufHelper()->CreateProtobuf();

		m_protobufCallback(*(request.get()), *response.get());

		vector<tinyxml2::XMLElement*> nodes = m_responseBinding.GetProtobufHelper()->GenerateResponse(response, respDoc);

		tinyxml2::XMLElement* el = respDoc.NewElement(m_responseBinding.GetName().c_str());
		el->SetAttribute("xmlns", "http://Battle.net");

		for (size_t x = 0; x < nodes.size(); ++x)
		{
			el->LinkEndChild(nodes[x]);
		}

		return el;
	}
	else
	{
		throw std::exception("Invoke not supported on FunctionBinding");
	}
}

vector<tinyxml2::XMLElement*> FunctionBinding::GenerateWsdlMessage(const string& prefix, tinyxml2::XMLDocument* doc)
{
	vector<tinyxml2::XMLElement*> ret;

	{
		tinyxml2::XMLElement* message = doc->NewElement("wsdl:message");
		message->SetAttribute("name", (prefix + "_" + m_name + "_" + m_requestBinding.GetName() + "_Request").c_str());

		tinyxml2::XMLElement* part = doc->NewElement("wsdl:part");
		part->SetAttribute("name", "parameters");
		part->SetAttribute("element", (string("tns:") + m_requestBinding.GetName()).c_str());

		message->LinkEndChild(part);
		ret.push_back(message);
	}

	{
		tinyxml2::XMLElement* message = doc->NewElement("wsdl:message");
		message->SetAttribute("name", (prefix + "_" + m_name + "_" + m_responseBinding.GetName() + "_Response").c_str());

		tinyxml2::XMLElement* part = doc->NewElement("wsdl:part");
		part->SetAttribute("name", "parameters");
		part->SetAttribute("element", (string("tns:") + m_responseBinding.GetName()).c_str());

		message->LinkEndChild(part);
		ret.push_back(message);
	}

	return ret;
}

tinyxml2::XMLElement* FunctionBinding::GenerateWsdlOperaton(const string& prefix, tinyxml2::XMLDocument* doc)
{
	tinyxml2::XMLElement* operation = doc->NewElement("wsdl:operation");
	operation->SetAttribute("name", m_name.c_str());

	tinyxml2::XMLElement* input;
	tinyxml2::XMLElement* output;

	if (m_isInput)
	{
		input = doc->NewElement("wsdl:input");
		output = doc->NewElement("wsdl:output");
	}
	else
	{
		input = doc->NewElement("wsdl:output");
		output = doc->NewElement("wsdl:input");
	}

	input->SetAttribute("wsaw:Action", (string("http://Battle.net/") + prefix + "/" + m_name).c_str());
	input->SetAttribute("message", (string("tns:") + prefix + "_" + m_name + "_" + m_requestBinding.GetName() + "_Request").c_str());

	output->SetAttribute("wsaw:Action", (string("http://Battle.net/") + prefix + "/" + m_name + "Response").c_str());
	output->SetAttribute("message", (string("tns:") + prefix + "_" + m_name + "_" + m_responseBinding.GetName() + "_Response").c_str());

	operation->LinkEndChild(input);
	operation->LinkEndChild(output);

	return operation;
}

tinyxml2::XMLElement* FunctionBinding::GenerateWsdlAction(const string& prefix, tinyxml2::XMLDocument* doc)
{
	tinyxml2::XMLElement* operation = doc->NewElement("wsdl:operation");
	operation->SetAttribute("name", m_name.c_str());

	tinyxml2::XMLElement* action = doc->NewElement("soap12:operation");
	action->SetAttribute("soapAction", (string("http://Battle.net/") + prefix + "/" + m_name).c_str());
	action->SetAttribute("style", "document");
	operation->LinkEndChild(action);

	{
		tinyxml2::XMLElement* input = doc->NewElement("wsdl:input");
		tinyxml2::XMLElement* body = doc->NewElement("soap12:body");
		body->SetAttribute("use", "literal");
		input->LinkEndChild(body);
		operation->LinkEndChild(input);
	}

	{
		tinyxml2::XMLElement* output = doc->NewElement("wsdl:output");
		tinyxml2::XMLElement* body = doc->NewElement("soap12:body");
		body->SetAttribute("use", "literal");
		output->LinkEndChild(body);
		operation->LinkEndChild(output);
	}

	return operation;
}
