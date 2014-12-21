#include "NativeFieldHelper.h"

#include "Util.h"
#include "FieldBinding.h"
#include "tinyxml2.h"
#include "NativeSoapServer.h"
#include "NativeClassHelper.h"
#include "SoapServerInternal.h"


namespace
{
  template <typename T>
  void SetValueToXmlFromMem(const void* obj, tinyxml2::XMLElement* element, tinyxml2::XMLDocument& doc, const FieldBinding& field)
  {
    assert(sizeof(T) == field.GetSize());
    T& t = *reinterpret_cast<T*>((char*)obj + field.GetOffset());

    string val = ValueToString<T>(t);
    element->SetText(val.c_str());
  }

  template <typename T>
  void SetVectorToXmlFromMem(const void* obj, tinyxml2::XMLElement* element, tinyxml2::XMLDocument& doc, const FieldBinding& field)
  {
    assert(sizeof(vector<T>) == field.GetSize());
    vector<T>& t = *reinterpret_cast<vector<T>*>((char*)obj + field.GetOffset());

    for (size_t x = 0; x < t.size(); ++x)
    {
      auto node = doc.NewElement("item");
      node->SetAttribute("type", (string("xs:") + GetSoapTypeName<T>()).c_str());
      node->SetText(ValueToString<T>(t[x]).c_str());
      element->LinkEndChild(node);
    }
  }


  template <typename T>
  void SetValueToObjFromMem(const void* obj, tinyxml2::XMLElement* element, const FieldBinding& field)
  {
    assert(sizeof(T) == field.GetSize());
    T& t = *reinterpret_cast<T*>((char*)obj + field.GetOffset());

    t = ValueFromString<T>(element->GetText());
  }

  template <typename T>
  void SetVectorToObjFromMem(const void* obj, tinyxml2::XMLElement* element, const FieldBinding& field)
  {
    assert(sizeof(vector<T>) == field.GetSize());
    vector<T>& t = *reinterpret_cast<vector<T>*>((char*)obj + field.GetOffset());

    auto node = element->FirstChildElement();

    while (node)
    {
      assert(string("item") == node->Value());

      if (!node->GetText())
      {
        t.push_back(T());
      }
      else
      {
        t.push_back(ValueFromString<T>(node->GetText()));
      }
      
      node = node->NextSiblingElement();
    }
  }
}


NativeFieldHelper::NativeFieldHelper(SoapServerInternal* server, const FieldBinding& field)
	: m_server(server)
  , m_field(field)
{
  m_xmlToObj["double"]      = bind(&SetValueToObjFromMem<double>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["float"]       = bind(&SetValueToObjFromMem<float>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["int"]         = bind(&SetValueToObjFromMem<int32_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["unsignedInt"] = bind(&SetValueToObjFromMem<uint32_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["long"]        = bind(&SetValueToObjFromMem<int64_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["unsignedLong"] = bind(&SetValueToObjFromMem<uint64_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["string"]      = bind(&SetValueToObjFromMem<string>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["boolean"]     = bind(&SetValueToObjFromMem<bool>, placeholders::_1, placeholders::_2, m_field);

  m_xmlToObj["vector<double>"] = bind(&SetVectorToObjFromMem<double>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<float>"] = bind(&SetVectorToObjFromMem<float>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<int>"] = bind(&SetVectorToObjFromMem<int32_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<unsignedInt>"] = bind(&SetVectorToObjFromMem<uint32_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<long>"] = bind(&SetVectorToObjFromMem<int64_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<unsignedLong>"] = bind(&SetVectorToObjFromMem<uint64_t>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<string>"] = bind(&SetVectorToObjFromMem<string>, placeholders::_1, placeholders::_2, m_field);
  m_xmlToObj["vector<boolean>"] = bind(&SetVectorToObjFromMem<bool>, placeholders::_1, placeholders::_2, m_field);



  m_objToXml["double"] = bind(&SetValueToXmlFromMem<double>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["float"] = bind(&SetValueToXmlFromMem<float>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["int"] = bind(&SetValueToXmlFromMem<int32_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["unsignedInt"] = bind(&SetValueToXmlFromMem<uint32_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["long"] = bind(&SetValueToXmlFromMem<int64_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["unsignedLong"] = bind(&SetValueToXmlFromMem<uint64_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["string"] = bind(&SetValueToXmlFromMem<string>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["boolean"] = bind(&SetValueToXmlFromMem<bool>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);

  m_objToXml["vector<double>"] = bind(&SetVectorToXmlFromMem<double>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<float>"] = bind(&SetVectorToXmlFromMem<float>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<int>"] = bind(&SetVectorToXmlFromMem<int32_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<unsignedInt>"] = bind(&SetVectorToXmlFromMem<uint32_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<long>"] = bind(&SetVectorToXmlFromMem<int64_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<unsignedLong>"] = bind(&SetVectorToXmlFromMem<uint64_t>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<string>"] = bind(&SetVectorToXmlFromMem<string>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
  m_objToXml["vector<boolean>"] = bind(&SetVectorToXmlFromMem<bool>, placeholders::_1, placeholders::_2, placeholders::_3, m_field);
}

void NativeFieldHelper::SetValueToObj(void* obj, tinyxml2::XMLElement* element)
{
  map<string, function<void(void*, tinyxml2::XMLElement*)> >::const_iterator cit = m_xmlToObj.find(m_field.GetType());

  if (cit != m_xmlToObj.end())
  {
    return cit->second(obj, element);
  }
  else if (m_field.IsClass())
  {
    const ClassBinding& binding = m_server->GetClassBinding(string("class:") + m_field.GetType());

    void* fieldLoc = (char*)obj + m_field.GetOffset();
    binding.GetNativeHelper()->FillObject(fieldLoc, element);
  }
  else
	{
		assert(false);
	}
}

void NativeFieldHelper::SetValueToXml(const void* obj, tinyxml2::XMLElement* element, tinyxml2::XMLDocument& doc)
{
  map<string, function<void(const void*, tinyxml2::XMLElement*, tinyxml2::XMLDocument&)> >::const_iterator cit = m_objToXml.find(m_field.GetType());

  if (cit != m_objToXml.end())
  {
    return cit->second(obj, element, doc);
  }
  else if (m_field.IsClass())
  {
    const ClassBinding& binding = m_server->GetClassBinding(string("class:") + m_field.GetType());

    void* fieldLoc = (char*)obj + m_field.GetOffset();
    auto nodes = binding.GetNativeHelper()->GenerateResponse(fieldLoc, doc);

    for (auto n : nodes)
    {
      element->LinkEndChild(n);
    }
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
