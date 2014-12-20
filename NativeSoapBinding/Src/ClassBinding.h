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
	ClassBinding(const string& name, shared_ptr<NativeClassHelper> nativeHelper);

	void SetParent(const ClassBinding& parent);
	void AddField(const FieldBinding& field);

  vector<tinyxml2::XMLElement*> GenerateWsdl(tinyxml2::XMLDocument* doc);

	string GetName() const;

	shared_ptr<ProtobufClassHelper> GetProtobufHelper() const;
	shared_ptr<NativeClassHelper> GetNativeHelper() const;

private:
	friend class NativeClassHelper;

	shared_ptr<ProtobufClassHelper> m_protobufHelper;
	shared_ptr<NativeClassHelper> m_nativeHelper;

	string m_name;
	vector<FieldBinding const*> m_fields;
	ClassBinding const* m_parent;
};