#pragma once


#include "NativeFieldHelper.h"
#include "SoapServer.h"


class NativeClassHelper
{
public:
	NativeClassHelper(CreateObjectCallback req)
		: m_create(req)
		, m_binding(0)
	{
	}

	void SetClassBinding(ClassBinding* binding)
	{
		m_binding = binding;
	}

	shared_ptr<ObjectWrapper> CreateObject()
	{
		return m_create();
	}

	shared_ptr<ObjectWrapper> CreateObject(tinyxml2::XMLElement* request)
	{
		SetupFields();

		shared_ptr<ObjectWrapper> obj(m_create());

		for (size_t x = 0; x < m_fields.size(); ++x)
		{
			string name = m_fields[x].GetName();
      auto el = request->FirstChildElement(name.c_str());

      if (el)
      {
        m_fields[x].SetValueToObj(obj.get()->get(), el);
      }
		}
		
		return obj;
	}

	vector<tinyxml2::XMLElement*> GenerateResponse(const void* response, tinyxml2::XMLDocument &doc)
	{
		SetupFields();

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

  tinyxml2::XMLElement* GenerateRequest(const string& name, const void* request, tinyxml2::XMLDocument &doc)
  {
    tinyxml2::XMLElement* node = doc.NewElement(name.c_str());
    node->SetAttribute("xmlns", "http://battle.net/types");

    for (size_t x = 0; x < m_fields.size(); ++x)
    {
      string name = m_fields[x].GetName();
      tinyxml2::XMLElement* el = doc.NewElement(name.c_str());

      m_fields[x].SetValueToXml(request, el);
      node->LinkEndChild(el);
    }

    return node;
  }

protected:
	void SetupFields()
	{
		if (!m_fields.empty())
		{
			return;
		}

		for (size_t x = 0; x < m_binding->m_fields.size(); ++x)
		{
			m_fields.push_back(NativeFieldHelper(*m_binding->m_fields[x]));
		}
	}

private:
	ClassBinding* m_binding;
	CreateObjectCallback m_create;
	vector<NativeFieldHelper> m_fields;
};
