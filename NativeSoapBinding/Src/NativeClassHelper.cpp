#include "NativeClassHelper.h"

#include "ClassBinding.h"

#include "tinyxml2.h"

NativeClassHelper::NativeClassHelper(SoapServerInternal *server, CreateObjectCallback req)
  : m_server(server)
  , m_create(req)
  , m_binding(0)
{
}

void NativeClassHelper::SetClassBinding(ClassBinding* binding)
{
  m_binding = binding;
}

shared_ptr<ObjectWrapper> NativeClassHelper::CreateObject()
{
  return m_create();
}

shared_ptr<ObjectWrapper> NativeClassHelper::CreateObject(tinyxml2::XMLElement* request)
{
  SetupFields();

  shared_ptr<ObjectWrapper> obj(m_create());
  FillObject(obj->get(), request);
  return obj;
}

void NativeClassHelper::FillObject(void* obj, tinyxml2::XMLElement* request)
{
  SetupFields();

  for (size_t x = 0; x < m_fields.size(); ++x)
  {
    string name = m_fields[x].GetName();
    auto el = request->FirstChildElement(name.c_str());

    if (el)
    {
      m_fields[x].SetValueToObj(obj, el);
    }
  }
}

vector<tinyxml2::XMLElement*> NativeClassHelper::GenerateResponse(const void* response, tinyxml2::XMLDocument &doc)
{
  SetupFields();

  vector<tinyxml2::XMLElement*> res;

  for (size_t x = 0; x < m_fields.size(); ++x)
  {
    string name = m_fields[x].GetName();
    tinyxml2::XMLElement* el = doc.NewElement(name.c_str());

    m_fields[x].SetValueToXml(response, el, doc);
    res.push_back(el);
  }

  return res;
}

tinyxml2::XMLElement* NativeClassHelper::GenerateRequest(const string& name, const void* request, tinyxml2::XMLDocument &doc)
{
  SetupFields();

  tinyxml2::XMLElement* node = doc.NewElement(name.c_str());
  node->SetAttribute("xmlns", "http://battle.net/types");

  for (size_t x = 0; x < m_fields.size(); ++x)
  {
    string className = m_fields[x].GetName();
    tinyxml2::XMLElement* el = doc.NewElement(className.c_str());

    m_fields[x].SetValueToXml(request, el, doc);
    node->LinkEndChild(el);
  }

  return node;
}


void NativeClassHelper::SetupFields()
{
  if (!m_fields.empty())
  {
    return;
  }

  for (size_t x = 0; x < m_binding->m_fields.size(); ++x)
  {
    m_fields.push_back(NativeFieldHelper(m_server, *m_binding->m_fields[x]));
  }
}