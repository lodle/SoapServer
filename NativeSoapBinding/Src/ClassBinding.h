#pragma once

#include "std.h"

namespace tinyxml2
{
	class XMLElement;
	class XMLDocument;
}

class ProtobufClassHelper;
class NativeClassHelper;
class FieldBinding;

class ClassBinding
{
public:
	ClassBinding();
	ClassBinding(const string& name, shared_ptr<ProtobufClassHelper> protobufHelper);

	void SetParent(const ClassBinding& parent);
	void AddField(const FieldBinding& field);

	tinyxml2::XMLElement* GenerateWsdl(tinyxml2::XMLDocument* doc);

	string GetName();

	shared_ptr<ProtobufClassHelper> GetProtobufHelper();
	shared_ptr<NativeClassHelper> GetNativeHelper();

private:
	shared_ptr<ProtobufClassHelper> m_protobufHelper;
	shared_ptr<NativeClassHelper> m_nativeHelper;

	string m_name;
	vector<FieldBinding const*> m_fields;
	ClassBinding const* m_parent;
};