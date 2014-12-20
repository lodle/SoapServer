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

vector<tinyxml2::XMLElement*> ClassBinding::GenerateWsdl(tinyxml2::XMLDocument* doc)
{
  vector<tinyxml2::XMLElement*> res;

	auto complex = doc->NewElement("xs:complexType");
  complex->SetAttribute("name", m_name.c_str());

	auto sequence = doc->NewElement("xs:sequence");

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		sequence->LinkEndChild(m_fields[x]->GenerateWsdl(doc));
	}

	complex->LinkEndChild(sequence);
  res.push_back(complex);
 
	return res;
}

string ClassBinding::GetName() const
{
	return m_name;
}

shared_ptr<ProtobufClassHelper> ClassBinding::GetProtobufHelper() const
{
	assert(m_protobufHelper);
	return m_protobufHelper;
}

shared_ptr<NativeClassHelper> ClassBinding::GetNativeHelper() const
{
	assert(m_nativeHelper);
	m_nativeHelper->SetClassBinding(const_cast<ClassBinding*>(this));
	return m_nativeHelper;
}
