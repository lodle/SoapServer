#include "ClassBinding.h"


#include "tinyxml2.h"
#include "FieldBinding.h"


ClassBinding::ClassBinding()
{
}

ClassBinding::ClassBinding(const string& name, shared_ptr<ProtobufClassHelper> protobufHelper)
	: m_name(name)
	, m_parent(0)
	, m_protobufHelper(protobufHelper)
{
}

void ClassBinding::SetParent(const ClassBinding& parent)
{
	m_parent = &parent;
}

void ClassBinding::AddField(const FieldBinding& field)
{
	m_fields.push_back(&field);
}

tinyxml2::XMLElement* ClassBinding::GenerateWsdl(tinyxml2::XMLDocument* doc)
{
	auto el = doc->NewElement("xs:element");
	el->SetAttribute("name", m_name.c_str());

	auto complex = doc->NewElement("xs:complexType");
	auto sequence = doc->NewElement("xs:sequence");

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		sequence->LinkEndChild(m_fields[x]->GenerateWsdl(doc));
	}

	complex->LinkEndChild(sequence);
	el->LinkEndChild(complex);

	return el;
}

string ClassBinding::GetName()
{
	return m_name;
}

shared_ptr<ProtobufClassHelper> ClassBinding::GetProtobufHelper()
{
	return m_protobufHelper;
}

shared_ptr<NativeClassHelper> ClassBinding::GetNativeHelper()
{
	return m_nativeHelper;
}
