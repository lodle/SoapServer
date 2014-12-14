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
			SendFault(doc, exception("Failed to find related message"));
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

string SoapProtocol::GenerateGuid()
{
	return "uuid:ab4c4001-f089-721c-aae6-f51ec37d3501";
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
	address->SetText("net.tcp://localhost:666/battlenet/Echo");
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

void SoapProtocol::SendResponse(const tinyxml2::XMLDocument& reqDoc, tinyxml2::XMLDocument& respDoc, tinyxml2::XMLElement* respBody, const string& respUrl)
{
	tinyxml2::XMLElement* root = respDoc.NewElement("s:Envelope");
	root->SetAttribute("xmlns:s", "http://schemas.xmlsoap.org/soap/envelope/");
	root->SetAttribute("xmlns:a", "http://www.w3.org/2005/08/addressing");

	root->LinkEndChild(GenerateResponseHeader(reqDoc, respDoc, respUrl));
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

	root->LinkEndChild(GenerateResponseHeader(reqDoc, respDoc, ""));
	root->LinkEndChild(body);
	respDoc.LinkEndChild(root);

	SendXml(respDoc);
}