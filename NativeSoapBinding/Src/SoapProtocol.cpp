#include "SoapProtocol.h"

#include "tinyxml2.h"
#include "Util.h"

#include "SoapServerInternal.h"
#include "ServiceBinding.h"


void SoapProtocol::HandleRequest(const string & xml, const map<string, ServiceBinding>& bindings)
{
	tinyxml2::XMLDocument doc;
	doc.Parse(xml.c_str(), xml.size());

	tinyxml2::XMLHandle handle(doc.RootElement());
	tinyxml2::XMLElement* action = handle.FirstChildElement("s:Header").FirstChildElement("a:Action").ToElement();

	string actionUrl = "";

	if (action)
	{
		actionUrl = action->GetText();
	}

	tinyxml2::XMLElement* reqBody = handle.FirstChildElement("s:Body").FirstChildElement().ToElement();

	map<string, ServiceBinding>::const_iterator it = bindings.begin();

	for (; it != bindings.end(); ++it)
	{
		if (actionUrl.find(it->second.GetActionUrl()) == 0)
		{
			tinyxml2::XMLDocument respDoc;
			tinyxml2::XMLElement* respBody = respDoc.NewElement("s:Body");

			try
			{
				const_cast<ServiceBinding*>(&it->second)->Invoke(actionUrl, respDoc, reqBody, respBody);
				SendResponse(doc, respDoc, respBody, actionUrl + "Response");
			}
			catch (std::exception& e)
			{
				SendFault(doc, e);
			}
			catch (...)
			{
				SendFault(doc, std::exception("Unhandled exception on invoke"));
			}

			break;
		}
	}

	if (it == bindings.end())
	{
		SendFault(doc, std::exception("Action not understood"));
	}
}

tinyxml2::XMLElement* SoapProtocol::GenerateSendHeader(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, const string& respUrl)
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

	tinyxml2::XMLElement* mid = respDoc.NewElement("a:MessageId");
	mid->SetText("urn:uuid:C622CE58-495C-427D-A4D0-F8C8B03AC377");

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

void SoapProtocol::SendResponse(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* respBody, const string& respUrl)
{
	tinyxml2::XMLElement* root = respDoc.NewElement("s:Envelope");
	root->SetAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	root->SetAttribute("xmlns:a", "http://www.w3.org/2005/08/addressing");

	root->LinkEndChild(GenerateSendHeader(reqDoc, respDoc, respUrl));
	root->LinkEndChild(respBody);
	respDoc.LinkEndChild(root);

	SendXml(respDoc);
}

void SoapProtocol::SendXml(const tinyxml2::XMLDocument& doc)
{
	tinyxml2::XMLPrinter printer(0, true);
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

	m_soap.write(buff.c_str(), 1 + count);
	m_soap.write(xml.c_str(), xml.size());
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

	root->LinkEndChild(GenerateSendHeader(reqDoc, respDoc, ""));
	root->LinkEndChild(body);
	respDoc.LinkEndChild(root);

	SendXml(respDoc);
}