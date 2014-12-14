#pragma once

#include "ProtobufFieldHelper.h"

#include "Util.h"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "tinyxml2.h"



ProtobufFieldHelper::ProtobufFieldHelper(const google::protobuf::FieldDescriptor* descriptor)
	: m_descriptor(descriptor)
{
}

void ProtobufFieldHelper::SetValueToObj(shared_ptr<google::protobuf::Message>& obj, tinyxml2::XMLElement* element)
{
	const google::protobuf::Reflection& r = *obj->GetReflection();

	switch (m_descriptor->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		r.SetInt32(obj.get(), m_descriptor, ValueFromString<int32_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		r.SetInt64(obj.get(), m_descriptor, ValueFromString<int64_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		r.SetUInt32(obj.get(), m_descriptor, ValueFromString<uint32_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		r.SetUInt64(obj.get(), m_descriptor, ValueFromString<uint64_t>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		r.SetDouble(obj.get(), m_descriptor, ValueFromString<double>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		r.SetFloat(obj.get(), m_descriptor, ValueFromString<float>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		r.SetBool(obj.get(), m_descriptor, ValueFromString<bool>(element->GetText()));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		assert(false);
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		r.SetString(obj.get(), m_descriptor, element->GetText());
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
		assert(false);
		break;

	default:
		assert(false);
		break;
	}
}

void ProtobufFieldHelper::SetValueToXml(const shared_ptr<google::protobuf::Message>& obj, tinyxml2::XMLElement* element)
{
	const google::protobuf::Reflection& r = *obj->GetReflection();

	string value;

	switch (m_descriptor->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		value = ValueToString<int32_t>(r.GetInt32(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		value = ValueToString<int64_t>(r.GetInt64(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		value = ValueToString<uint32_t>(r.GetUInt32(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		value = ValueToString<uint64_t>(r.GetUInt64(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		value = ValueToString<double>(r.GetDouble(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		value = ValueToString<float>(r.GetFloat(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		value = ValueToString<bool>(r.GetBool(*obj.get(), m_descriptor));
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		assert(false);
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
		value = r.GetString(*obj.get(), m_descriptor);
		break;

	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
		assert(false);
		break;

	default:
		assert(false);
		break;
	}

	element->SetText(value.c_str());
}

string ProtobufFieldHelper::GetName()
{
	return m_descriptor->name();
}

