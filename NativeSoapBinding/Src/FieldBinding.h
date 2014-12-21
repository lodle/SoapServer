#pragma once

#include "std.h"

#include "SoapServer.h"


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
	FieldBinding(const string& name, const string& type, size_t offset, size_t size, int flags);

	tinyxml2::XMLElement* GenerateWsdl(tinyxml2::XMLDocument* doc) const;

  string GetFullType() const;

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
    return (m_flags & FF_Class) == FF_Class;
  }

  bool IsOptional() const
  {
    return (m_flags & FF_Optional) == FF_Optional;
  }

  bool IsEnum() const
  {
    return (m_flags & FF_Enum) == FF_Enum;
  }

  bool IsRepeated() const
  {
    return (m_flags & FF_Repeated) == FF_Repeated;
  }

private:
	string m_name;
	string m_type;

	size_t m_offset;
	size_t m_size;

  int m_flags;

  vector<string> m_enumValues;
};