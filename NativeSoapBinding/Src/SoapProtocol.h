#pragma once

#include "std.h"


namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

class SoapWriteI;
class ServiceBinding;

class SoapProtocol
{
public:
	SoapProtocol(SoapWriteI &soap)
		: m_soap(soap)
	{
	}

	void HandleRequest(const string& xml, const map<string, ServiceBinding>& bindings);

protected:
	tinyxml2::XMLElement* GenerateSendHeader(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, const string& respUrl);
	void SendResponse(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* respBody, const string& respUrl);

	void SendXml(const tinyxml2::XMLDocument& doc);
	void SendFault(const tinyxml2::XMLDocument& reqDoc, const exception& e);

private:
	SoapWriteI& m_soap;
};