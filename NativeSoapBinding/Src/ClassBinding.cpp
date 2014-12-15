#include "ClassBinding.h"


#include "tinyxml2.h"
#include "FieldBinding.h"
#include "NativeClassHelper.h"


ClassBinding::ClassBinding()
{
}

ClassBinding::ClassBinding(const string& name, shared_ptr<ProtobufClassHelper> protobufHelper)
	: m_name(name)
	, m_parent(0)
	, m_protobufHelper(protobufHelper)
{
}

ClassBinding::ClassBinding(const string& name, shared_ptr<NativeClassHelper> nativeHelper)
	: m_name(name)
	, m_parent(0)
	, m_nativeHelper(nativeHelper)
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
	assert(m_protobufHelper);
	return m_protobufHelper;
}

shared_ptr<NativeClassHelper> ClassBinding::GetNativeHelper()
{
	assert(m_nativeHelper);
	m_nativeHelper->SetClassBinding(this);
	return m_nativeHelper;
}
