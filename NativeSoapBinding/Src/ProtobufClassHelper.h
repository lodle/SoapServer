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

class ProtobufClassHelper
{
public:
	ProtobufClassHelper(const google::protobuf::Descriptor* descriptor);
	~ProtobufClassHelper();

	shared_ptr<google::protobuf::Message> CreateProtobuf();
	shared_ptr<google::protobuf::Message> CreateProtobuf(tinyxml2::XMLElement* request);

	void FillProtobuf(google::protobuf::Message* obj, tinyxml2::XMLElement* request);

	vector<tinyxml2::XMLElement*> GenerateResponse(const shared_ptr<google::protobuf::Message> &response, tinyxml2::XMLDocument &doc);
	tinyxml2::XMLElement* GenerateRequest(const google::protobuf::Message &request, tinyxml2::XMLDocument &doc);

	vector<ProtobufFieldHelper> GenerateFieldList();

private:
	const google::protobuf::Descriptor* m_descriptor;
	vector<ProtobufFieldHelper> m_fields;
};
