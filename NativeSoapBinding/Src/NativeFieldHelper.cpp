#include "NativeFieldHelper.h"

#include "Util.h"
#include "FieldBinding.h"
#include "tinyxml2.h"


namespace
{
  template <typename T>
  void SetValueToXmlFromMem(const void* obj, tinyxml2::XMLElement* element, const FieldBinding& field)
  {
    assert(sizeof(T) == field.GetSize());

    T& t = *reinterpret_cast<T*>((char*)obj + field.GetOffset());
    string val = ValueToString<T>(t);
    element->SetText(val.c_str());
  }


  template <typename T>
  void SetValueToObjFromMem(const void* obj, tinyxml2::XMLElement* element, const FieldBinding& field)
  {
    assert(sizeof(T) == field.GetSize());

    T& t = *reinterpret_cast<T*>((char*)obj + field.GetOffset());
    t = ValueFromString<T>(element->GetText());
  }
}


NativeFieldHelper::NativeFieldHelper(const FieldBinding& field)
	: m_field(field)
{
}

void NativeFieldHelper::SetValueToObj(void* obj, tinyxml2::XMLElement* element)
{
	string type = m_field.GetType();

	if (type == "double")
	{
    SetValueToObjFromMem<double>(obj, element, m_field);
	}
  else if (type == "int")
  {
    SetValueToObjFromMem<int32_t>(obj, element, m_field);
  }
  else if (type == "string")
  {
    SetValueToObjFromMem<string>(obj, element, m_field);
  }
	else
	{
		assert(false);
	}
}


void NativeFieldHelper::SetValueToXml(const void* obj, tinyxml2::XMLElement* element)
{
	string type = m_field.GetType();

	if (type == "double")
	{
    SetValueToXmlFromMem<double>(obj, element, m_field);
	}
  else if (type == "int")
  {
    SetValueToXmlFromMem<int32_t>(obj, element, m_field);
  }
  else if (type == "string")
  {
    SetValueToXmlFromMem<string>(obj, element, m_field);
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
