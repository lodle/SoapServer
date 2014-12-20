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
  class XMLDocument;
}

class SoapServerInternal;

class ProtobufFieldHelper
{
public:
	ProtobufFieldHelper(SoapServerInternal& server, const google::protobuf::FieldDescriptor* descriptor);

	void SetValueToObj(google::protobuf::Message* obj, tinyxml2::XMLElement* element);
  vector<tinyxml2::XMLElement*> SetValueToXml(const google::protobuf::Message& obj, tinyxml2::XMLDocument& doc);

	string GetName();

private:
	const google::protobuf::FieldDescriptor* m_descriptor;
  SoapServerInternal* m_server;
};