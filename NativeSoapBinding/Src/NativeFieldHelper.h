#pragma once

#include "std.h"

class FieldBinding;
class SoapServerInternal;

namespace tinyxml2
{
	class XMLElement;
  class XMLDocument;
}

class NativeFieldHelper
{
public:
	NativeFieldHelper(SoapServerInternal* server, const FieldBinding& field);

	void SetValueToObj(void* obj, tinyxml2::XMLElement* element);
	void SetValueToXml(const void* obj, tinyxml2::XMLElement* element, tinyxml2::XMLDocument& doc);

	string GetName();

private:
  map<string, function<void(void*, tinyxml2::XMLElement*)> > m_xmlToObj;
  map<string, function<void(const void*, tinyxml2::XMLElement*, tinyxml2::XMLDocument&)> > m_objToXml;

  SoapServerInternal* m_server;
	const FieldBinding& m_field;
};