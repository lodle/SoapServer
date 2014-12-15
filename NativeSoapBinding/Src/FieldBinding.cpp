#include "FieldBinding.h"
#include "tinyxml2.h"

FieldBinding::FieldBinding()
{
}

FieldBinding::FieldBinding(const string& name, const string& type)
	: m_name(name)
	, m_type(type)
	, m_offset(-1)
	, m_size(-1)
{
}

FieldBinding::FieldBinding(const string& name, const string& type, size_t offset, size_t size)
	: m_name(name)
	, m_type(type)
	, m_offset(offset)
	, m_size(size)
{
}

tinyxml2::XMLElement* FieldBinding::GenerateWsdl(tinyxml2::XMLDocument* doc) const
{
	tinyxml2::XMLElement* el = doc->NewElement("xs:element");

	el->SetAttribute("minOccurs", "0");
	el->SetAttribute("name", m_name.c_str());
	el->SetAttribute("type", (string("xs:") + m_type).c_str());

	return el;
}
