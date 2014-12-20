#include "ProtobufClassHelper.h"

#include <google/protobuf/message.h>
#include "tinyxml2.h"

#include "Util.h"


ProtobufClassHelper::ProtobufClassHelper(SoapServerInternal& server, const google::protobuf::Descriptor* descriptor)
	: m_descriptor(descriptor)
	, m_fields(GenerateFieldList(server))
{
}

ProtobufClassHelper::~ProtobufClassHelper()
{
}

shared_ptr<google::protobuf::Message> ProtobufClassHelper::CreateProtobuf()
{
	return shared_ptr<google::protobuf::Message>(google::protobuf::MessageFactory::generated_factory()->GetPrototype(m_descriptor)->New());
}

shared_ptr<google::protobuf::Message> ProtobufClassHelper::CreateProtobuf(tinyxml2::XMLElement* request)
{
	shared_ptr<google::protobuf::Message> obj = CreateProtobuf();
	FillProtobuf(obj.get(), request);
	return obj;
}

void ProtobufClassHelper::FillProtobuf(google::protobuf::Message* obj, tinyxml2::XMLElement* request)
{
	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		string name = m_fields[x].GetName();
		m_fields[x].SetValueToObj(obj, request->FirstChildElement(name.c_str()));
	}
}

vector<tinyxml2::XMLElement*> ProtobufClassHelper::GenerateResponse(const google::protobuf::Message &response, tinyxml2::XMLDocument &doc)
{
	vector<tinyxml2::XMLElement*> res;

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
    auto nodes = m_fields[x].SetValueToXml(response, doc);

    for (size_t x = 0; x < nodes.size(); ++x)
    {
      res.push_back(nodes[x]);
    }
	}

	return res;
}

tinyxml2::XMLElement * ProtobufClassHelper::GenerateRequest(const string& name, const google::protobuf::Message & request, tinyxml2::XMLDocument & doc)
{
	tinyxml2::XMLElement* node = doc.NewElement(name.c_str());
	node->SetAttribute("xmlns", "http://battle.net/types");

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
    auto nodes = m_fields[x].SetValueToXml(request, doc);

    for (size_t x = 0; x < nodes.size(); ++x)
    {
      node->LinkEndChild(nodes[x]);
    }
	}

	return node;
}

vector<ProtobufFieldHelper> ProtobufClassHelper::GenerateFieldList(SoapServerInternal& server)
{
	vector<ProtobufFieldHelper> ret;

	for (int x = 0; x < m_descriptor->field_count(); ++x)
	{
		ret.push_back(ProtobufFieldHelper(server, m_descriptor->field(x)));
	}

	return ret;
}
