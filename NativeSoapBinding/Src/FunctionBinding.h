#pragma once


#include "std.h"
#include "ClassBinding.h"
#include "SoapServer.h"


class FunctionBinding
{
public:
	FunctionBinding();
	FunctionBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput);

	tinyxml2::XMLElement* Invoke(tinyxml2::XMLElement* req, tinyxml2::XMLDocument& respDoc);

	vector<tinyxml2::XMLElement*> GenerateWsdlMessage(const string& prefix, tinyxml2::XMLDocument* doc);
	tinyxml2::XMLElement* GenerateWsdlOperaton(const string& prefix, tinyxml2::XMLDocument* doc);
	tinyxml2::XMLElement* GenerateWsdlAction(const string& prefix, tinyxml2::XMLDocument* doc);

private:
	ProtobufCallback m_protobufCallback;

	string m_name;
	ClassBinding m_requestBinding;
	ClassBinding m_responseBinding;

	bool m_isInput;
};