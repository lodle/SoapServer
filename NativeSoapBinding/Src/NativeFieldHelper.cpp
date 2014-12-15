#include "NativeFieldHelper.h"

#include "Util.h"
#include "FieldBinding.h"
#include "tinyxml2.h"


NativeFieldHelper::NativeFieldHelper(const FieldBinding& field)
	: m_field(field)
{
}

void NativeFieldHelper::SetValueToObj(void* obj, tinyxml2::XMLElement* element)
{
	string type = m_field.GetType();

	if (type == "double")
	{
		double d = ValueFromString<double>(element->GetText());
		assert(sizeof(d) == m_field.GetSize());

		memcpy((char*)obj + m_field.GetOffset(), &d, sizeof(d));
	}
	else
	{
		assert(false);
	}
}

void NativeFieldHelper::SetValueToXml(void* obj, tinyxml2::XMLElement* element)
{
	string type = m_field.GetType();

	if (type == "double")
	{
		double d;
		assert(sizeof(d) == m_field.GetSize());
		memcpy(&d, (char*)obj + m_field.GetOffset(), sizeof(d));

		string val = ValueToString<double>(d);
		element->SetText(val.c_str());
	}
	else
	{
		assert(false);
	}
}

string NativeFieldHelper::GetName()
{
	return m_field.GetName();
}
