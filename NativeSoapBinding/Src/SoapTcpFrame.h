#pragma once

#include "std.h"


class SoapProtocol;
class SoapTcpConnectionI;

class SoapTcpFrame
{
public:
	SoapTcpFrame(SoapProtocol& protocol, SoapTcpConnectionI& tcpConnection);

	bool OnRead(const char* buff, size_t size);


protected:
	size_t ProcessHeader(const char* buff, size_t size);
	size_t ProcessEnd(const char* buff, size_t size);
	size_t ProcessRequest(const char* buff, size_t size);

private:
	map<char, function<size_t(const char* buff, size_t size)>> m_rawMessageHandlers;
	SoapProtocol& m_soapProtocol;
	SoapTcpConnectionI& m_soapTcpConnection;
};