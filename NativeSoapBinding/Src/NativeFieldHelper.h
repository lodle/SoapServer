#pragma once

#include "std.h"

class FieldBinding;

namespace tinyxml2
{
	class XMLElement;
}

class NativeFieldHelper
{
public:
	NativeFieldHelper(const FieldBinding& field);

	void SetValueToObj(void* obj, tinyxml2::XMLElement* element);
	void SetValueToXml(const void* obj, tinyxml2::XMLElement* element);

	string GetName();

private:
	const FieldBinding& m_field;
};