#include "SoapProtocol.h"

#include "tinyxml2.h"
#include "Util.h"

#include "SoapServerInternal.h"
#include "ServiceBinding.h"

void Log(const string& data);

SoapProtocol::SoapProtocol(SoapTcpConnectionI &soap)
  : m_soap(soap)
{
}

void SoapProtocol::HandleRequest(const string & xml, const map<string, ServiceBinding>& bindings)
{
  Log(string("<Inbound>") + xml + "</Inbound>");

	shared_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
	doc->Parse(xml.c_str(), xml.size());

	tinyxml2::XMLHandle handle(doc->RootElement());
	tinyxml2::XMLElement* action = handle.FirstChildElement("s:Header").FirstChildElement("a:Action").ToElement();
	tinyxml2::XMLElement* relatesTo = handle.FirstChildElement("s:Header").FirstChildElement("a:RelatesTo").ToElement();

	string actionUrl = "";
	string messageId = "";

	if (action)
	{
		actionUrl = action->GetText();
	}

	if (relatesTo)
	{
		messageId = relatesTo->GetText();
	}

	if (!messageId.empty())
	{
		map<string, ResponseCallback>::iterator it = m_callbacks.find(messageId);

		if (it == m_callbacks.end())
		{
			SendFault(*doc, exception("Failed to find related message"));
		}
		else
		{
			tinyxml2::XMLElement* body = handle.FirstChildElement("s:Body").FirstChildElement().ToElement();
			it->second(body);
		}
	}
	else
	{
		tinyxml2::XMLElement* reqBody = handle.FirstChildElement("s:Body").FirstChildElement().ToElement();

		map<string, ServiceBinding>::const_iterator it = bindings.begin();

		for (; it != bindings.end(); ++it)
		{
			if (actionUrl.find(it->second.GetActionUrl()) == 0)
			{
        shared_ptr<tinyxml2::XMLDocument> respDoc(new tinyxml2::XMLDocument());
				tinyxml2::XMLElement* respBody = respDoc->NewElement("s:Body");

				try
				{
          string url = actionUrl + "Response";
          function<void()> callback = bind(&SoapProtocol::SendResponse, this, doc, respDoc, respBody, url);
					const_cast<ServiceBinding*>(&it->second)->Invoke(actionUrl, *respDoc, reqBody, respBody, callback);
				}
				catch (std::exception& e)
				{
					SendFault(*doc, e);
				}
				catch (...)
				{
					SendFault(*doc, std::exception("Unhanded exception on invoke"));
				}

        return;
			}
		}

		if (it == bindings.end())
		{
			SendFault(*doc, std::exception("Action not understood"));
		}
	}
}

void SoapProtocol::SendRequest(const string& actionUrl, tinyxml2::XMLDocument &doc, tinyxml2::XMLElement *payload, ResponseCallback callback)
{
	string guid = GenerateGuid();

	tinyxml2::XMLElement* root = doc.NewElement("s:Envelope");
	root->SetAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	root->SetAttribute("xmlns:a", "http://www.w3.org/2005/08/addressing");

	tinyxml2::XMLElement* body = doc.NewElement("s:Body");
	body->LinkEndChild(payload);

	root->LinkEndChild(GenerateRequestHeader(doc, actionUrl, guid));
	root->LinkEndChild(body);
	doc.LinkEndChild(root);

	m_callbacks[guid] = callback;
	SendXml(doc);
}

#include <windows.h>

string SoapProtocol::GenerateGuid()
{
  GUID guid;
  HRESULT hCreateGuid = CoCreateGuid(&guid);

  OLECHAR* bstrGuid;
  StringFromCLSID(guid, &bstrGuid);

  char buff[255];

  int size = WideCharToMultiByte(CP_ACP, // ANSI Code Page
    0, // No special handling of unmapped chars
    bstrGuid, // wide-character string to be converted
    wcslen(bstrGuid),
    buff,
    255,
    NULL, NULL);

  string g = string("uuid:") + string(buff + 1, size - 2);

  ::CoTaskMemFree(bstrGuid);

  return g;
}

tinyxml2::XMLElement* SoapProtocol::GenerateRequestHeader(tinyxml2::XMLDocument& reqDoc, const string& actionUrl, const string& guid)
{
	tinyxml2::XMLElement* header = reqDoc.NewElement("s:Header");
	tinyxml2::XMLElement* action = reqDoc.NewElement("a:Action");

	action->SetAttribute("s:mustUnderstand", "1");
	action->SetText(actionUrl.c_str());

	tinyxml2::XMLElement* mid = reqDoc.NewElement("a:MessageID");
	mid->SetText(guid.c_str());

	tinyxml2::XMLElement* to = reqDoc.NewElement("a:To");
	to->SetAttribute("s:mustUnderstand", "1");
	to->SetText("http://www.w3.org/2005/08/addressing/anonymous");

	tinyxml2::XMLElement* replyTo = reqDoc.NewElement("a:ReplyTo");
	tinyxml2::XMLElement* address = reqDoc.NewElement("a:Address");
	address->SetText(m_addressUrl.c_str());
	replyTo->LinkEndChild(address);

	header->LinkEndChild(action);
	header->LinkEndChild(mid);
	header->LinkEndChild(replyTo);
	header->LinkEndChild(to);

	return header;
}

tinyxml2::XMLElement* SoapProtocol::GenerateResponseHeader(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, const string& respUrl)
{
	string messageId;
	string toAddress;

	tinyxml2::XMLHandle handle(const_cast<tinyxml2::XMLElement*>(reqDoc.RootElement()));
	tinyxml2::XMLElement* messageIdNode = handle.FirstChildElement("s:Header").FirstChildElement("a:MessageID").ToElement();
	tinyxml2::XMLElement* toNode = handle.FirstChildElement("s:Header").FirstChildElement("a:ReplyTo").FirstChildElement("a:Address").ToElement();

	if (messageIdNode)
	{
		messageId = messageIdNode->GetText();
	}

	if (toNode)
	{
		toAddress = toNode->GetText();
	}

	tinyxml2::XMLElement* header = respDoc.NewElement("s:Header");
	tinyxml2::XMLElement* action = respDoc.NewElement("a:Action");
	action->SetAttribute("s:mustUnderstand", "1");
	action->SetText(respUrl.c_str());

	tinyxml2::XMLElement* mid = respDoc.NewElement("a:MessageID");
	mid->SetText(GenerateGuid().c_str());

	tinyxml2::XMLElement* relatesTo = respDoc.NewElement("a:RelatesTo");
	relatesTo->SetText(messageId.c_str());

	tinyxml2::XMLElement* to = respDoc.NewElement("a:To");
	to->SetAttribute("s:mustUnderstand", "1");
	to->SetText(toAddress.c_str());

	header->LinkEndChild(action);
	header->LinkEndChild(mid);
	header->LinkEndChild(relatesTo);
	header->LinkEndChild(to);

	return header;
}

void SoapProtocol::SendResponse(shared_ptr<tinyxml2::XMLDocument> reqDoc, shared_ptr<tinyxml2::XMLDocument> respDoc, tinyxml2::XMLElement* respBody, string respUrl)
{
	tinyxml2::XMLElement* root = respDoc->NewElement("s:Envelope");
	root->SetAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	root->SetAttribute("xmlns:a", "http://www.w3.org/2005/08/addressing");

	root->LinkEndChild(GenerateResponseHeader(*reqDoc, *respDoc, respUrl));
	root->LinkEndChild(respBody);
	respDoc->LinkEndChild(root);

	SendXml(*respDoc);
}

void SoapProtocol::SendXml(const tinyxml2::XMLDocument& doc)
{
	tinyxml2::XMLPrinter printer(0, false);
	doc.Print(&printer);

	string xml(printer.CStr(), printer.CStrSize());

	if (xml[xml.size() - 1] == '\0')
	{
		xml = xml.substr(0, xml.size() - 1);
	}

	string buff;
	buff.resize(5);

	buff[0] = 0x6;
	size_t count = EncodePackedInt(const_cast<char*>(buff.c_str() + 1), xml.size());

  Log(string("<Outbound>") + xml + "</Outbound>");

	lock_guard<mutex> guard(m_soap.GetLock());
	m_soap.Write(buff.c_str(), 1 + count);
	m_soap.Write(xml.c_str(), xml.size());
}

void SoapProtocol::SendFault(const tinyxml2::XMLDocument& reqDoc, const std::exception& e)
{
	tinyxml2::XMLDocument respDoc;

	tinyxml2::XMLElement* root = respDoc.NewElement("s:Envelope");
	root->SetAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	root->SetAttribute("xmlns:a", "http://www.w3.org/2005/08/addressing");

	tinyxml2::XMLElement* body = respDoc.NewElement("s:Body");

	tinyxml2::XMLElement* fault = respDoc.NewElement("s:Fault");
	tinyxml2::XMLElement* faultcode = respDoc.NewElement("faultcode");
	faultcode->SetText("Server");
	tinyxml2::XMLElement* faultstring = respDoc.NewElement("faultstring");
	faultstring->SetText(e.what());

	fault->LinkEndChild(faultcode);
	fault->LinkEndChild(faultstring);
	body->LinkEndChild(fault);

	root->LinkEndChild(GenerateResponseHeader(reqDoc, respDoc, ""));
	root->LinkEndChild(body);
	respDoc.LinkEndChild(root);

	SendXml(respDoc);
}

void SoapProtocol::SetAddressUrl(const string& url)
{
	m_addressUrl = url;
}