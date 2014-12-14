#include "SoapTcpFrame.h"

#include "Util.h"
#include "SoapProtocol.h"
#include "SoapInterfaces.h"


SoapTcpFrame::SoapTcpFrame(SoapProtocol& protocol, SoapTcpConnectionI& tcpConnection)
	: m_soapProtocol(protocol)
	, m_soapTcpConnection(tcpConnection)
{
	m_rawMessageHandlers[0x0] = bind(&SoapTcpFrame::ProcessHeader, this, placeholders::_1, placeholders::_2);
	m_rawMessageHandlers[0x6] = bind(&SoapTcpFrame::ProcessRequest, this, placeholders::_1, placeholders::_2);
	m_rawMessageHandlers[0x7] = bind(&SoapTcpFrame::ProcessEnd, this, placeholders::_1, placeholders::_2);
}

bool SoapTcpFrame::OnRead(const char* buff, size_t size)
{
	size_t read = 0;

	while (read < size)
	{
		const char* b = buff + read;
		size_t s = size - read;

		map<char, function<size_t(const char* buff, size_t size)> >::iterator it = m_rawMessageHandlers.find(b[0]);

		if (it == m_rawMessageHandlers.end())
		{
			m_soapTcpConnection.End();
			return false;
		}

		read += it->second(b, s);
	}

	return true;
}

size_t SoapTcpFrame::ProcessHeader(const char* buff, size_t size)
{
	size_t buffidx = 0;

	_declspec(align(1)) struct VersionRecord
	{
		char RecordType;
		char MajorVersion;
		char MinorVersion;

		bool IsValid() const
		{
			return RecordType == 0x00 && MajorVersion == 0x01 && MinorVersion == 0x00;
		}
	};

	const struct VersionRecord* v = reinterpret_cast<const VersionRecord*>(buff);
	buffidx += 3;

	if (!v->IsValid())
	{
		assert(false);
		return buffidx;
	}

	_declspec(align(1)) struct ModeRecord
	{
		char RecordType;
		char Mode;

		bool IsValid() const
		{
			return RecordType == 0x01 && Mode == 0x02; //Duplex
		}
	};

	const struct ModeRecord* m = reinterpret_cast<const ModeRecord*>(buff + buffidx);
	buffidx += 2;

	if (!m->IsValid())
	{
		assert(false);
		return buffidx;
	}


	_declspec(align(1)) struct ViaRecord
	{
		char RecordType;
		mutable char ViaLength;

		bool IsValid() const
		{
			return RecordType == 0x02 && ViaLength != 0x00;
		}

		string GetUrl() const
		{
			size_t count = 0;
			size_t len = DecodePackedInt(&ViaLength, count) + count;
			return string(&ViaLength + count, len - count);
		}

		size_t GetSize() const
		{
			size_t count = 0;
			return 1 + DecodePackedInt(&ViaLength, count) + count;
		}
	};

	const struct ViaRecord* via = reinterpret_cast<const ViaRecord*>(buff + buffidx);
	buffidx += via->GetSize();

	if (!via->IsValid())
	{
		assert(false);
		return buffidx;
	}

	string url = via->GetUrl();
	m_soapProtocol.SetAddressUrl(url);
	m_soapTcpConnection.SetBindingName(url.substr(url.find_last_of('/')+1));

	_declspec(align(1)) struct EnvelopeEncodingRecord
	{
		char RecordType;
		char Encoding;
		char PreambleEnd;

		bool IsValid() const
		{
			return RecordType == 0x03 && Encoding == 0x0 && PreambleEnd == 0x0C; //UTF8
		}
	};

	const struct EnvelopeEncodingRecord* e = reinterpret_cast<const EnvelopeEncodingRecord*>(buff + buffidx);
	buffidx += 3;

	if (!e->IsValid())
	{
		assert(false);
		return buffidx;
	}

	_declspec(align(1)) struct PreambleAck
	{
		char RecordType;

		PreambleAck()
			: RecordType(0x0B)
		{
		}
	};

	struct PreambleAck p;
	m_soapTcpConnection.Write((char*)&p, 1);
	return buffidx;
}

size_t SoapTcpFrame::ProcessEnd(const char* buff, size_t size)
{
	assert(buff[0] == 0x07);

	static char end[1] = { 0x07 };
	m_soapTcpConnection.Write(end, 1);
	
	m_soapTcpConnection.End();
	return 1;
}

size_t SoapTcpFrame::ProcessRequest(const char* buff, size_t size)
{
	assert(buff[0] == 0x06);

	_declspec(align(1)) struct SizedEnvelopRecord
	{
		char RecordType;
		char Size;

		bool IsValid() const
		{
			return RecordType == 0x06 && Size != 0x00;
		}

		string GetData() const
		{
			size_t count = 0;
			size_t len = DecodePackedInt(&Size, count) + count;
			return string(&Size + count, len - count);
		}

		size_t GetSize() const
		{
			size_t count = 0;
			return 1 + DecodePackedInt(&Size, count) + count;
		}
	};

	const struct SizedEnvelopRecord* e = reinterpret_cast<const SizedEnvelopRecord*>(buff);

	assert(e->IsValid());
	m_soapProtocol.HandleRequest(e->GetData(), m_soapTcpConnection.GetServiceBindings());
	return e->GetSize();
}
