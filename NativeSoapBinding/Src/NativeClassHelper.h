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

	shared_ptr<void> CreateObject()
	{
		return shared_ptr<void>(m_create());
	}

	shared_ptr<void> CreateObject(tinyxml2::XMLElement* request)
	{
		SetupFields();

		shared_ptr<void> obj(m_create());

		for (size_t x = 0; x < m_fields.size(); ++x)
		{
			string name = m_fields[x].GetName();
			m_fields[x].SetValueToObj(obj.get(), request->FirstChildElement(name.c_str()));
		}
		
		return obj;
	}

	vector<tinyxml2::XMLElement*> GenerateResponse(const shared_ptr<void> &response, tinyxml2::XMLDocument &doc)
	{
		SetupFields();

		vector<tinyxml2::XMLElement*> res;

		for (size_t x = 0; x < m_fields.size(); ++x)
		{
			string name = m_fields[x].GetName();
			tinyxml2::XMLElement* el = doc.NewElement(name.c_str());

			m_fields[x].SetValueToXml(response.get(), el);
			res.push_back(el);
		}

		return res;
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
