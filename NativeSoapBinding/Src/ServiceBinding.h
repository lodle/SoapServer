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

	void AddBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback& callback, bool isInput, bool isOneWay);
	void AddBinding(const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback& callback, bool isInput, bool isOneWay);

	void Invoke(const string& actionUrl, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* reqBody, tinyxml2::XMLElement* respBody, function<void()> callback);

	string GetActionUrl() const;

	vector<tinyxml2::XMLElement*> GenerateWsdl(tinyxml2::XMLDocument* doc);
  vector<tinyxml2::XMLElement*> GenerateWsdlPortTypeOperations(tinyxml2::XMLDocument* doc);
  vector<tinyxml2::XMLElement*> GenerateWsdlBindingOperations(tinyxml2::XMLDocument* doc);
  vector<tinyxml2::XMLElement*> GenerateWsdlElements(tinyxml2::XMLDocument* doc);
  tinyxml2::XMLElement* GenerateWsdlPort(tinyxml2::XMLDocument* doc);

protected:
  void OnResponse(tinyxml2::XMLElement* respBody, function<void()> callback, tinyxml2::XMLElement* respNode);

private:
	string m_name;
	map<string, FunctionBinding> m_bindings;
};