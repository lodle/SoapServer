#include "ProtobufClassHelper.h"

#include <google/protobuf/message.h>
#include "tinyxml2.h"

#include "Util.h"


ProtobufClassHelper::ProtobufClassHelper(const google::protobuf::Descriptor* descriptor)
	: m_descriptor(descriptor)
	, m_fields(GenerateFieldList())
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

vector<tinyxml2::XMLElement*> ProtobufClassHelper::GenerateResponse(const shared_ptr<google::protobuf::Message> &response, tinyxml2::XMLDocument &doc)
{
	vector<tinyxml2::XMLElement*> res;

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		string name = m_fields[x].GetName();
		tinyxml2::XMLElement* el = doc.NewElement(name.c_str());

		m_fields[x].SetValueToXml(*response.get(), el);
		res.push_back(el);
	}

	return res;
}

tinyxml2::XMLElement * ProtobufClassHelper::GenerateRequest(const google::protobuf::Message & request, tinyxml2::XMLDocument & doc)
{
	tinyxml2::XMLElement* node = doc.NewElement(request.GetDescriptor()->name().c_str());
	node->SetAttribute("xmlns", "http://Battle.net");

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		string name = m_fields[x].GetName();
		tinyxml2::XMLElement* el = doc.NewElement(name.c_str());

		m_fields[x].SetValueToXml(request, el);
		node->LinkEndChild(node);
	}

	return node;
}

vector<ProtobufFieldHelper> ProtobufClassHelper::GenerateFieldList()
{
	vector<ProtobufFieldHelper> ret;

	for (int x = 0; x < m_descriptor->field_count(); ++x)
	{
		ret.push_back(ProtobufFieldHelper(m_descriptor->field(x)));
	}

	return ret;
}
