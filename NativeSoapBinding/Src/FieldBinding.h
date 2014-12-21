#pragma once

#include "std.h"

namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

namespace google
{
  namespace protobuf
  {
    class FieldDescriptor;
  }
}

class SoapServerInternal;

class FieldBinding
{
public:
	FieldBinding();
	FieldBinding(const ::google::protobuf::FieldDescriptor* descriptor, SoapServerInternal* server);
	FieldBinding(const string& name, const string& type, size_t offset, size_t size);

	tinyxml2::XMLElement* GenerateWsdl(tinyxml2::XMLDocument* doc) const;


	string GetName() const
	{
		return m_name;
	}

	string GetType() const
	{
		return m_type;
	}

	size_t GetOffset() const
	{
		assert(m_offset != -1);
		return m_offset;
	}

	size_t GetSize() const
	{
		assert(m_size != -1);
		return m_size;
	}

  bool IsClass() const
  {
    return m_class;
  }

private:
	string m_name;
	string m_type;

	size_t m_offset;
	size_t m_size;

  bool m_optional;
  bool m_repeated;
  bool m_enum;
  bool m_class;

  vector<string> m_enumValues;
};