#include "FieldBinding.h"
#include "tinyxml2.h"

#include "SoapServerInternal.h"
#include <google/protobuf/descriptor.h>

FieldBinding::FieldBinding()
{
}

FieldBinding::FieldBinding(const ::google::protobuf::FieldDescriptor* descriptor, SoapServerInternal* server)
	: m_name(descriptor->name())
	, m_type(descriptor->type_name())
	, m_offset(-1)
	, m_size(-1)
  , m_flags(0)
{
  if (descriptor->is_optional())
  {
    m_flags |= FF_Optional;
  }

  if (descriptor->is_repeated())
  {
    m_flags |= FF_Repeated;
  }

  switch (descriptor->type())
  {
  case ::google::protobuf::FieldDescriptor::TYPE_STRING:
    m_type = "string";
    break;

  case ::google::protobuf::FieldDescriptor::TYPE_BOOL:
    m_type = "boolean";
    break;

  case ::google::protobuf::FieldDescriptor::TYPE_BYTES:
    m_type = "hexBinary";
    break;

  case ::google::protobuf::FieldDescriptor::TYPE_UINT64:
  case ::google::protobuf::FieldDescriptor::TYPE_FIXED64:
    m_type = "unsignedLong";
    break;

  case ::google::protobuf::FieldDescriptor::TYPE_UINT32:
  case ::google::protobuf::FieldDescriptor::TYPE_FIXED32:
    m_type = "unsignedInt";
    break;

  case ::google::protobuf::FieldDescriptor::TYPE_SFIXED32:
  case ::google::protobuf::FieldDescriptor::TYPE_INT32:
    m_type = "int";
    break;

  case ::google::protobuf::FieldDescriptor::TYPE_SFIXED64:
  case ::google::protobuf::FieldDescriptor::TYPE_INT64:
    m_type = "long";
    break;
  };

  if (descriptor->enum_type())
  {
    m_flags |= FF_Enum;

    auto ed = descriptor->enum_type();

    for (int x = 0; x < ed->value_count(); ++x)
    {
      m_enumValues.push_back(ed->value(x)->name());
    }
  }

  if (descriptor->message_type())
  {
    m_flags |= FF_Class;
    m_type = descriptor->message_type()->name();
    server->GetClassBinding(descriptor->message_type());
  }
}

FieldBinding::FieldBinding(const string& name, const string& type, size_t offset, size_t size, int flags)
	: m_name(name)
	, m_type(type)
	, m_offset(offset)
	, m_size(size)
  , m_flags(flags)
{
}

tinyxml2::XMLElement* FieldBinding::GenerateWsdl(tinyxml2::XMLDocument* doc) const
{
  tinyxml2::XMLElement* el = doc->NewElement("xs:element");
  el->SetAttribute("name", m_name.c_str());

  if (IsEnum())
  {
    tinyxml2::XMLElement* restriction = doc->NewElement("xs:restriction");
    restriction->SetAttribute("base", "xs:string");

    for (size_t x = 0; x < m_enumValues.size(); ++x)
    {
      tinyxml2::XMLElement* enumeration = doc->NewElement("xs:enumeration");
      enumeration->SetAttribute("value", m_enumValues[x].c_str());
      restriction->LinkEndChild(enumeration);
    }
    el->LinkEndChild(restriction);
  }
  else
  {
    if (IsOptional())
    {
      el->SetAttribute("minOccurs", "0");
    }

    if (IsRepeated())
    {
      el->SetAttribute("maxOccurs", "unbounded");
    }

    if (IsClass())
    {
      el->SetAttribute("type", (string("bntypes:") + m_type).c_str());
    }
    else
    {
      el->SetAttribute("type", (string("xs:") + m_type).c_str());
    }
  }

	return el;
}

string FieldBinding::GetFullType() const
{
  string type = GetType();

  if (IsClass())
  {
    type = string("class:") + type;
  }

  if (IsRepeated())
  {
    type = string("vector<") + type + ">";
  }

  return type;
}