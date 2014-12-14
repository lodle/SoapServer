#pragma once

class SoapTcpConnectionI
{
public:
	virtual void Write(const char* data, size_t size) = 0;
	virtual void SetBindingName(const string& name) = 0;
	virtual void End() = 0;

	//todo move some where else
	virtual map<string, ServiceBinding>& GetServiceBindings() = 0;
};