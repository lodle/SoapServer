#pragma once

#include "std.h"


namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

class SoapTcpConnectionI;
class ServiceBinding;

class SoapProtocol
{
public:
	typedef function<void(tinyxml2::XMLElement*)> ResponseCallback;

	SoapProtocol(SoapTcpConnectionI &soap)
		: m_soap(soap)
	{
	}

	void HandleRequest(const string& xml, const map<string, ServiceBinding>& bindings);

	void SendRequest(const string& actionUrl, tinyxml2::XMLDocument& doc, tinyxml2::XMLElement* payload, ResponseCallback callback);
	void SendResponse(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* respBody, const string& respUrl);

	void SetAddressUrl(const string& url);

protected:
	tinyxml2::XMLElement* GenerateResponseHeader(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, const string& respUrl);
	

	void SendXml(const tinyxml2::XMLDocument& doc);
	void SendFault(const tinyxml2::XMLDocument& reqDoc, const exception& e);

	tinyxml2::XMLElement* GenerateRequestHeader(tinyxml2::XMLDocument& reqDoc, const string& actionUrl, const string& guid);

	string GenerateGuid();

private:
	map<string, ResponseCallback> m_callbacks;
	SoapTcpConnectionI& m_soap;
	string m_addressUrl;
};