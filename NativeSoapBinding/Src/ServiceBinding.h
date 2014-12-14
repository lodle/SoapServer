#pragma once

#include "std.h"
#include "SoapServer.h"
#include "FunctionBinding.h"

class ClassBinding;

namespace tinyxml2
{
	class XMLDocument;
	class XMLElement;
}

class ServiceBinding
{
public:
	ServiceBinding();
	ServiceBinding(const string& name);

	void AddBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback& callback, bool isInput);

	void Invoke(const string& actionUrl, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* reqBody, tinyxml2::XMLElement* respBody);

	string GetActionUrl() const;

	vector<tinyxml2::XMLElement*> GenerateWsdl(tinyxml2::XMLDocument* doc);

private:
	string m_name;
	map<string, FunctionBinding> m_bindings;
};