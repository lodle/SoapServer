#pragma once


#include "std.h"
#include "ClassBinding.h"
#include "SoapServer.h"


class FunctionBinding
{
public:
	FunctionBinding();
	FunctionBinding(const string& name, const ClassBinding& request, const ClassBinding& response, ProtobufCallback callback, bool isInput, bool isOneWay);
	FunctionBinding(const string& name, const ClassBinding& request, const ClassBinding& response, NativeCallback callback, bool isInput, bool isOneWay);

	void Invoke(tinyxml2::XMLElement* req, tinyxml2::XMLDocument& respDoc, function<void(tinyxml2::XMLElement*)> callback);

	vector<tinyxml2::XMLElement*> GenerateWsdlMessage(const string& prefix, tinyxml2::XMLDocument* doc);
	tinyxml2::XMLElement* GenerateWsdlOperaton(const string& prefix, tinyxml2::XMLDocument* doc);
	tinyxml2::XMLElement* GenerateWsdlAction(const string& prefix, tinyxml2::XMLDocument* doc);
  vector<tinyxml2::XMLElement*> GenerateWsdlElements(tinyxml2::XMLDocument* doc);

protected:
  void OnResponseProtobuf(tinyxml2::XMLDocument* respDoc, function<void(tinyxml2::XMLElement*)> callback, const google::protobuf::Message& response);
  void OnResponseNative(tinyxml2::XMLDocument* respDoc, function<void(tinyxml2::XMLElement*)> callback, const void* obj);

  string GetRequestMessageName(const string& prefix, bool withNamespace);
  string GetResponseMessageName(const string& prefix, bool withNamespace);

private:
	ProtobufCallback m_protobufCallback;
	NativeCallback m_nativeCallback;

	string m_name;
	ClassBinding m_requestBinding;
	ClassBinding m_responseBinding;

	bool m_isInput;
  bool m_isOneWay;
};