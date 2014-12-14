#pragma once

#include "std.h"

namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

class FieldBinding
{
public:
	FieldBinding();
	FieldBinding(const string& name, const string& type);

	tinyxml2::XMLElement* GenerateWsdl(tinyxml2::XMLDocument* doc) const;

private:
	string m_name;
	string m_type;
};