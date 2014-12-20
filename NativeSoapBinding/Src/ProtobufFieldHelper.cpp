#pragma once

#include "ProtobufFieldHelper.h"

#include "Util.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "tinyxml2.h"

#include "SoapServerInternal.h"
#include "ClassBinding.h"
#include "ProtobufClassHelper.h"

#include <sstream>
#include <iomanip>

namespace
{

  template <typename T>
  vector<tinyxml2::XMLElement*> SetRepeatedValueToXml(const string &name, const google::protobuf::FieldDescriptor* descriptor, const google::protobuf::Message& obj, tinyxml2::XMLDocument& doc
    , T(google::protobuf::Reflection::*getFunct)(const ::google::protobuf::Message&, const ::google::protobuf::FieldDescriptor*, int) const)
  {
    vector<tinyxml2::XMLElement*> res;
    const google::protobuf::Reflection& r = *obj.GetReflection();

    for (int x = 0; x < r.FieldSize(obj, descriptor); ++x)
    {
      T val = (r.*getFunct)(obj, descriptor, x);

      tinyxml2::XMLElement* element = doc.NewElement(name.c_str());
      element->SetText(ValueToString<T>(val).c_str());
      res.push_back(element);
    }

    return res;
  }


  char s_convertMap[]{
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'
  };

  std::string ConvertToHex(const std::string& input)
  {
    std::string str;

    for (size_t i = 0; i < input.size(); ++i)
    {
      char h1 = s_convertMap[input[i] >> 4 & 0xF];
      char h2 = s_convertMap[input[i] & 0xF];

      str.append(1, h1);
      str.append(1, h2);
    }

    return str;
  }
}



ProtobufFieldHelper::ProtobufFieldHelper(SoapServerInternal& server, const google::protobuf::FieldDescriptor* descriptor)
	: m_descriptor(descriptor)
  , m_server(&server)
{
}

void ProtobufFieldHelper::SetValueToObj(google::protobuf::Message* obj, tinyxml2::XMLElement* element)
{
  if (!element)
  {
    return;
  }

  const google::protobuf::Reflection& r = *obj->GetReflection();
  assert(!m_descriptor->is_repeated());

	switch (m_descriptor->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		r.SetInt32(obj, m_descriptor, ValueFromString<int32_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		r.SetInt64(obj, m_descriptor, ValueFromString<int64_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		r.SetUInt32(obj, m_descriptor, ValueFromString<uint32_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		r.SetUInt64(obj, m_descriptor, ValueFromString<uint64_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		r.SetDouble(obj, m_descriptor, ValueFromString<double>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		r.SetFloat(obj, m_descriptor, ValueFromString<float>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		r.SetBool(obj, m_descriptor, ValueFromString<bool>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		assert(false);
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		r.SetString(obj, m_descriptor, element->GetText());
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
  {
    const ClassBinding& classBinding = m_server->GetClassBinding(m_descriptor->message_type());
    auto node = element->FirstChildElement(classBinding.GetName().c_str());

    if (node)
    {
      classBinding.GetProtobufHelper()->FillProtobuf(r.MutableMessage(obj, m_descriptor), node);
    }
    break;
  }

	default:
		assert(false);
		break;
	}
}

vector<tinyxml2::XMLElement*> ProtobufFieldHelper::SetValueToXml(const google::protobuf::Message& obj, tinyxml2::XMLDocument& doc)
{
  vector<tinyxml2::XMLElement*> res;
	const google::protobuf::Reflection& r = *obj.GetReflection();

  if (m_descriptor->is_repeated())
  {
    switch (m_descriptor->cpp_type())
    {
    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
      res = SetRepeatedValueToXml<int32_t>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedInt32);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
      res = SetRepeatedValueToXml<int64_t>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedInt64);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
      res = SetRepeatedValueToXml<uint32_t>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedUInt32);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
      res = SetRepeatedValueToXml<uint64_t>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedUInt64);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
      res = SetRepeatedValueToXml<double>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedDouble);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
      res = SetRepeatedValueToXml<float>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedFloat);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
      res = SetRepeatedValueToXml<bool>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedBool);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
      assert(false);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
      res = SetRepeatedValueToXml<string>(GetName(), m_descriptor, obj, doc, &google::protobuf::Reflection::GetRepeatedString);
      break;

    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
    {
      for (int x = 0; x < r.FieldSize(obj, m_descriptor); ++x)
      {
        const google::protobuf::Message& message = r.GetRepeatedMessage(obj, m_descriptor, x);

        const ClassBinding& classBinding = m_server->GetClassBinding(message.GetDescriptor());
        tinyxml2::XMLElement* element = doc.NewElement(GetName().c_str());

        auto nodes = classBinding.GetProtobufHelper()->GenerateResponse(message, doc);

        for (size_t x = 0; x < nodes.size(); ++x)
        {
          element->LinkEndChild(nodes[x]);
        }

        res.push_back(element);
      }
      break;
    }

    default:
      assert(false);
      break;
    }
  }
  else
  {
    tinyxml2::XMLElement* element = doc.NewElement(GetName().c_str());
    string value;

    if (m_descriptor->type() == ::google::protobuf::FieldDescriptor::TYPE_BYTES)
    {
      value = ConvertToHex(r.GetString(obj, m_descriptor));
    }
    else
    {
      switch (m_descriptor->cpp_type())
      {
      case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
        value = ValueToString<int32_t>(r.GetInt32(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
        value = ValueToString<int64_t>(r.GetInt64(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
        value = ValueToString<uint32_t>(r.GetUInt32(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
        value = ValueToString<uint64_t>(r.GetUInt64(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
        value = ValueToString<double>(r.GetDouble(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
        value = ValueToString<float>(r.GetFloat(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
        value = ValueToString<bool>(r.GetBool(obj, m_descriptor));
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
        assert(false);
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
        value = r.GetString(obj, m_descriptor);
        break;

      case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
      {
        const google::protobuf::Message& message = r.GetMessage(obj, m_descriptor);
        const ClassBinding& classBinding = m_server->GetClassBinding(message.GetDescriptor());

        if (r.HasField(obj, m_descriptor))
        {
          auto nodes = classBinding.GetProtobufHelper()->GenerateResponse(message, doc);

          for (size_t x = 0; x < nodes.size(); ++x)
          {
            element->LinkEndChild(nodes[x]);
          }
        }

        break;
      }

      default:
        assert(false);
        break;
      }
    }

    element->SetText(value.c_str());
    res.push_back(element);
  }

  return res;
}

string ProtobufFieldHelper::GetName()
{
	return m_descriptor->name();
}

