#pragma once


class SoapWriteI
{
public:
	virtual void write(const char* data, size_t size) = 0;
};