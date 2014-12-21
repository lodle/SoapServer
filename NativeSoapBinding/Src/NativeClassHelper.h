#pragma once

#include "SoapServer.h"
#include "NativeFieldHelper.h"

class ClassBinding;
class SoapServerInternal;

namespace tinyxml2
{
  class XMLElement;
  class XMLDocument;
}


class NativeClassHelper
{
public:
  NativeClassHelper(SoapServerInternal *server, CreateObjectCallback req);

  void SetClassBinding(ClassBinding* binding);

  shared_ptr<ObjectWrapper> CreateObject();
  shared_ptr<ObjectWrapper> CreateObject(tinyxml2::XMLElement* request);

  void FillObject(void* obj, tinyxml2::XMLElement* request);

  vector<tinyxml2::XMLElement*> GenerateResponse(const void* response, tinyxml2::XMLDocument &doc);
  tinyxml2::XMLElement* GenerateRequest(const string& name, const void* request, tinyxml2::XMLDocument &doc);

protected:
  void SetupFields();

private:
  SoapServerInternal *m_server;
  ClassBinding* m_binding;
	CreateObjectCallback m_create;
	vector<NativeFieldHelper> m_fields;
};
