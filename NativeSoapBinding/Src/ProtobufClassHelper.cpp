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

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		string name = m_fields[x].GetName();
		m_fields[x].SetValueToObj(obj, request->FirstChildElement(name.c_str()));
	}

	return obj;
}

vector<tinyxml2::XMLElement*> ProtobufClassHelper::GenerateResponse(const shared_ptr<google::protobuf::Message> &response, tinyxml2::XMLDocument &doc)
{
	vector<tinyxml2::XMLElement*> res;

	for (size_t x = 0; x < m_fields.size(); ++x)
	{
		string name = m_fields[x].GetName();
		tinyxml2::XMLElement* el = doc.NewElement(name.c_str());

		m_fields[x].SetValueToXml(response, el);
		res.push_back(el);
	}

	return res;
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
