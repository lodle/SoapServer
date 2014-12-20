#pragma once

#include "std.h"
#include "ProtobufFieldHelper.h"

namespace google
{
	namespace protobuf
	{
		class Descriptor;
		class Message;
	}
}

namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

class SoapServerInternal;

class ProtobufClassHelper
{
public:
	ProtobufClassHelper(SoapServerInternal& server, const google::protobuf::Descriptor* descriptor);
	~ProtobufClassHelper();

	shared_ptr<google::protobuf::Message> CreateProtobuf();
	shared_ptr<google::protobuf::Message> CreateProtobuf(tinyxml2::XMLElement* request);

	void FillProtobuf(google::protobuf::Message* obj, tinyxml2::XMLElement* request);

	vector<tinyxml2::XMLElement*> GenerateResponse(const google::protobuf::Message &response, tinyxml2::XMLDocument &doc);
	tinyxml2::XMLElement* GenerateRequest(const string& name, const google::protobuf::Message &request, tinyxml2::XMLDocument &doc);

	vector<ProtobufFieldHelper> GenerateFieldList(SoapServerInternal& server);

private:
	const google::protobuf::Descriptor* m_descriptor;
	vector<ProtobufFieldHelper> m_fields;
};
