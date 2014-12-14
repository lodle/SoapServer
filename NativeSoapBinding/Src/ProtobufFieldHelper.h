#pragma once

#include "std.h"

namespace google
{
	namespace protobuf
	{
		class FieldDescriptor;
		class Message;
	}
}

namespace tinyxml2
{
	class XMLElement;
}

class ProtobufFieldHelper
{
public:
	ProtobufFieldHelper(const google::protobuf::FieldDescriptor* descriptor);

	void SetValueToObj(shared_ptr<google::protobuf::Message>& obj, tinyxml2::XMLElement* element);
	void SetValueToXml(const shared_ptr<google::protobuf::Message>& obj, tinyxml2::XMLElement* element);

	string GetName();

private:
	const google::protobuf::FieldDescriptor* m_descriptor;
};