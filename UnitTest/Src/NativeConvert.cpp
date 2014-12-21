
#include <gtest/gtest.h>
#include <tinyxml2.h>

#include "NativeClassHelper.h"
#include "NativeFieldHelper.h"

#include "ClassBinding.h"
#include "FieldBinding.h"

#include "NativeSoapServer.h"

namespace
{

  class NativeFieldHelperFixture : public ::testing::Test
  {
  public:
    template <typename T>
    T GetValueFromXml(const char* valAsString, const char* name)
    {
      EXPECT_STREQ(name, GetSoapTypeName<T>().c_str());

      string typeName = name;
      int flags = SoapServer::GetFieldFlags(typeName);

      FieldBinding field(name, typeName, 0, sizeof(T), flags);

      T d;

      tinyxml2::XMLDocument doc;
      auto el = doc.NewElement(name);
      el->SetText(valAsString);
      doc.LinkEndChild(el);

      NativeFieldHelper helper(0, field);
      helper.SetValueToObj(&d, el);

      return d;
    }

    template <typename T>
    T GetValuesFromXml(const vector<string>& valuesAsString, const char* name)
    {
      EXPECT_STREQ(name, GetSoapTypeName<T>().c_str());
      
      string typeName = name;
      int flags = SoapServer::GetFieldFlags(typeName);

      FieldBinding field(name, typeName, 0, sizeof(T), flags);

      T d;

      tinyxml2::XMLDocument doc;
      auto el = doc.NewElement(name);

      for (size_t x = 0; x < valuesAsString.size(); ++x)
      {
        auto node = doc.NewElement("item");
        node->SetText(valuesAsString[x].c_str());
        el->LinkEndChild(node);
      }

      doc.LinkEndChild(el);

      NativeFieldHelper helper(0, field);
      helper.SetValueToObj(&d, el);

      return d;
    }


    template <typename T>
    string GetValueFromObj(T t, const char* name)
    {
      EXPECT_STREQ(name, GetSoapTypeName<T>().c_str());

      string typeName = name;
      int flags = SoapServer::GetFieldFlags(typeName);

      FieldBinding field(name, typeName, 0, sizeof(T), flags);
      NativeFieldHelper helper(0, field);


      tinyxml2::XMLDocument doc;
      auto el = doc.NewElement("testobj");
      doc.LinkEndChild(el);

      helper.SetValueToXml(&t, el, doc);

      tinyxml2::XMLPrinter printer(0, true);
      doc.Print(&printer);

      return string(printer.CStr(), printer.CStrSize());
    }


    template <typename T>
    void AssertEq(const vector<T>& expected, const vector<T>& input)
    {
      ASSERT_EQ(expected.size(), input.size());

      for (size_t x = 0; x < expected.size(); ++x)
      {
        ASSERT_EQ(expected[x], input[x]);
      }
    }


    template <>
    void AssertEq<double>(const vector<double>& expected, const vector<double>& input)
    {
      ASSERT_EQ(expected.size(), input.size());

      for (size_t x = 0; x < expected.size(); ++x)
      {
        ASSERT_DOUBLE_EQ(expected[x], input[x]);
      }
    }

    template <>
    void AssertEq<float>(const vector<float>& expected, const vector<float>& input)
    {
      ASSERT_EQ(expected.size(), input.size());

      for (size_t x = 0; x < expected.size(); ++x)
      {
        ASSERT_FLOAT_EQ(expected[x], input[x]);
      }
    }
  };



  class AllValues
  {
  public:
    string m_string;
    double m_double;
    float m_float;
    int32_t m_int32;
    uint32_t m_uint32;
    int64_t m_int64;
    uint64_t m_uint64;
    bool m_bool;

    vector<string> m_vector_string;
    vector<double> m_vector_double;
    vector<float> m_vector_float;
    vector<int32_t> m_vector_int32;
    vector<uint32_t> m_vector_uint32;
    vector<int64_t> m_vector_int64;
    vector<uint64_t> m_vector_uint64;
    vector<bool> m_vector_bool;
  };

  class NativeFieldHelperAllFixture : public NativeFieldHelperFixture
  {
  public:
    NativeFieldHelperAllFixture()
      : nativeHelper(new NativeClassHelper(0, bind(&ObjectWrapper::Create<AllValues>)))
      , classBinding("AllValues", nativeHelper)
    {
      Addfield<string>("string", offsetof(AllValues, m_string));
      Addfield<double>("double", offsetof(AllValues, m_double));
      Addfield<float>("float", offsetof(AllValues, m_float));
      Addfield<int32_t>("int32", offsetof(AllValues, m_int32));
      Addfield<uint32_t>("uint32", offsetof(AllValues, m_uint32));
      Addfield<int64_t>("int64", offsetof(AllValues, m_int64));
      Addfield<uint64_t>("uint64", offsetof(AllValues, m_uint64));
      Addfield<bool>("bool", offsetof(AllValues, m_bool));

      Addfield<vector<string>>("vector_string", offsetof(AllValues, m_vector_string));
      Addfield<vector<double>>("vector_double", offsetof(AllValues, m_vector_double));
      Addfield<vector<float>>("vector_float", offsetof(AllValues, m_vector_float));
      Addfield<vector<int32_t>>("vector_int32", offsetof(AllValues, m_vector_int32));
      Addfield<vector<uint32_t>>("vector_uint32", offsetof(AllValues, m_vector_uint32));
      Addfield<vector<int64_t>>("vector_int64", offsetof(AllValues, m_vector_int64));
      Addfield<vector<uint64_t>>("vector_uint64", offsetof(AllValues, m_vector_uint64));
      Addfield<vector<bool>>("vector_bool", offsetof(AllValues, m_vector_bool));

      for (size_t x = 0; x < fieldBindings.size(); ++x)
      {
        classBinding.AddField(fieldBindings[x]);
      }

      nativeHelper->SetClassBinding(&classBinding);
    }

    template <typename T>
    void Addfield(const char* name, size_t offset)
    {
      string typeName = GetSoapTypeName<T>();
      int flags = SoapServer::GetFieldFlags(typeName);

      fieldBindings.push_back(FieldBinding(name, typeName, offset, sizeof(T), flags));
    }

    const char* GetXml()
    {
      return
        "<testobj xmlns=\"http://battle.net/types\">"
          "<string>imastring</string>"
          "<double>12300000000.455999</double>"
          "<float>123.456001</float>"
          "<int32>-123</int32>"
          "<uint32>123</uint32>"
          "<int64>-12300000000</int64>"
          "<uint64>12300000000</uint64>"
          "<bool>1</bool>"
          "<vector_string>"
            "<item type=\"xs:string\">imastring</item>"
            "<item type=\"xs:string\">123</item>"
            "<item type=\"xs:string\">adf</item>"
            "<item type=\"xs:string\"></item>"
          "</vector_string>"
          "<vector_double>"
            "<item type=\"xs:double\">12300000000.455999</item>"
            "<item type=\"xs:double\">3.141500</item>"
            "<item type=\"xs:double\">-234.234000</item>"
            "<item type=\"xs:double\">54335.000000</item>"
          "</vector_double>"
          "<vector_float>"
            "<item type=\"xs:float\">123.456001</item>"
            "<item type=\"xs:float\">3.141500</item>"
            "<item type=\"xs:float\">-234.233994</item>"
            "<item type=\"xs:float\">54335.000000</item>"
          "</vector_float>"
          "<vector_int32>"
            "<item type=\"xs:int\">123</item>"
            "<item type=\"xs:int\">0</item>"
            "<item type=\"xs:int\">-543</item>"
            "<item type=\"xs:int\">-1</item>"
          "</vector_int32>"
          "<vector_uint32>"
            "<item type=\"xs:unsignedInt\">123</item>"
            "<item type=\"xs:unsignedInt\">0</item>"
            "<item type=\"xs:unsignedInt\">543</item>"
            "<item type=\"xs:unsignedInt\">1</item>"
          "</vector_uint32>"
          "<vector_int64>"
            "<item type=\"xs:long\">12300000000</item>"
            "<item type=\"xs:long\">0</item>"
            "<item type=\"xs:long\">-12300000000</item>"
            "<item type=\"xs:long\">-1</item>"
          "</vector_int64>"
          "<vector_uint64>"
            "<item type=\"xs:unsignedLong\">12300000000</item>"
            "<item type=\"xs:unsignedLong\">0</item>"
            "<item type=\"xs:unsignedLong\">554300000000</item>"
            "<item type=\"xs:unsignedLong\">1</item>"
          "</vector_uint64>"
          "<vector_bool>"
            "<item type=\"xs:boolean\">1</item>"
            "<item type=\"xs:boolean\">0</item>"
            "<item type=\"xs:boolean\">1</item>"
            "<item type=\"xs:boolean\">1</item>"
          "</vector_bool>"
        "</testobj>";
    }

    shared_ptr<NativeClassHelper> nativeHelper;
    ClassBinding classBinding;
    vector<FieldBinding> fieldBindings;
  };
}



TEST_F(NativeFieldHelperFixture, FromXml_string)
{
  ASSERT_STREQ("imastring", GetValueFromXml<string>("imastring", "string").c_str());
}

TEST_F(NativeFieldHelperFixture, FromXml_double)
{
  ASSERT_DOUBLE_EQ(12300000000.456, GetValueFromXml<double>("12300000000.456", "double"));
}

TEST_F(NativeFieldHelperFixture, FromXml_float)
{
  ASSERT_FLOAT_EQ(123.456f, GetValueFromXml<float>("123.456", "float"));
}

TEST_F(NativeFieldHelperFixture, FromXml_int32_t)
{
  ASSERT_EQ(-123, GetValueFromXml<int32_t>("-123", "int"));
}

TEST_F(NativeFieldHelperFixture, FromXml_uint32_t)
{
  ASSERT_EQ(123, GetValueFromXml<uint32_t>("123", "unsignedInt"));
}

TEST_F(NativeFieldHelperFixture, FromXml_int64_t)
{
  ASSERT_EQ(-12300000000, GetValueFromXml<int64_t>("-12300000000", "long"));
}

TEST_F(NativeFieldHelperFixture, FromXml_uint64_t)
{
  ASSERT_EQ(12300000000, GetValueFromXml<uint64_t>("12300000000", "unsignedLong"));
}

TEST_F(NativeFieldHelperFixture, FromXml_bool)
{
  ASSERT_EQ(true, GetValueFromXml<bool>("true", "boolean"));
}


TEST_F(NativeFieldHelperFixture, FromXml_vector_string)
{
  vector<string> input;
  input.push_back("imastring");
  input.push_back("123");
  input.push_back("adf");
  input.push_back("");

  auto res = GetValuesFromXml<vector<string> >(input, "vector<string>");

  AssertEq<string>(input, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_double)
{
  vector<string> input;
  input.push_back("12300000000.456");
  input.push_back("3.1415");
  input.push_back("-234.234");
  input.push_back("54335");

  vector<double> output;
  output.push_back(12300000000.456);
  output.push_back(3.1415);
  output.push_back(-234.234);
  output.push_back(54335);

  auto res = GetValuesFromXml<vector<double> >(input, "vector<double>");

  AssertEq(output, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_float)
{
  vector<string> input;
  input.push_back("123.456");
  input.push_back("3.1415");
  input.push_back("-234.234");
  input.push_back("54335");

  vector<float> output;
  output.push_back(123.456f);
  output.push_back(3.1415f);
  output.push_back(-234.234f);
  output.push_back(54335.0f);

  auto res = GetValuesFromXml<vector<float> >(input, "vector<float>");

  AssertEq(output, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_int32_t)
{
  vector<string> input;
  input.push_back("123");
  input.push_back("0");
  input.push_back("-543");
  input.push_back("-1");

  vector<int32_t> output;
  output.push_back(123);
  output.push_back(0);
  output.push_back(-543);
  output.push_back(-1);

  auto res = GetValuesFromXml<vector<int32_t> >(input, "vector<int>");

  AssertEq(output, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_uint32_t)
{
  vector<string> input;
  input.push_back("123");
  input.push_back("0");
  input.push_back("543");
  input.push_back("1");

  vector<uint32_t> output;
  output.push_back(123);
  output.push_back(0);
  output.push_back(543);
  output.push_back(1);

  auto res = GetValuesFromXml<vector<uint32_t> >(input, "vector<unsignedInt>");

  AssertEq(output, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_int64_t)
{
  vector<string> input;
  input.push_back("12300000000");
  input.push_back("0");
  input.push_back("-12300000000");
  input.push_back("-1");

  vector<int64_t> output;
  output.push_back(12300000000);
  output.push_back(0);
  output.push_back(-12300000000);
  output.push_back(-1);

  auto res = GetValuesFromXml<vector<int64_t> >(input, "vector<long>");

  AssertEq(output, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_uint64_t)
{
  vector<string> input;
  input.push_back("12300000000");
  input.push_back("0");
  input.push_back("554300000000");
  input.push_back("1");

  vector<uint64_t> output;
  output.push_back(12300000000);
  output.push_back(0);
  output.push_back(554300000000);
  output.push_back(1);

  auto res = GetValuesFromXml<vector<uint64_t> >(input, "vector<unsignedLong>");

  AssertEq(output, res);
}

TEST_F(NativeFieldHelperFixture, FromXml_vector_bool)
{
  vector<string> input;
  input.push_back("true");
  input.push_back("false");
  input.push_back("true");
  input.push_back("true");

  vector<bool> output;
  output.push_back(true);
  output.push_back(false);
  output.push_back(true);
  output.push_back(true);

  auto res = GetValuesFromXml<vector<bool> >(input, "vector<boolean>");

  AssertEq(output, res);
}




TEST_F(NativeFieldHelperAllFixture, FromXml_All)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError err = doc.Parse(GetXml());

  ASSERT_EQ(tinyxml2::XML_SUCCESS, err);

  shared_ptr<ObjectWrapper> obj = nativeHelper->CreateObject(doc.RootElement());
  const AllValues& all = *static_cast<const AllValues*>(obj->get());

  ASSERT_STREQ("imastring", all.m_string.c_str());
  ASSERT_DOUBLE_EQ(12300000000.456, all.m_double);
  ASSERT_FLOAT_EQ(123.456f, all.m_float);
  ASSERT_EQ(-123, all.m_int32);
  ASSERT_EQ(123, all.m_uint32);
  ASSERT_EQ(-12300000000, all.m_int64);
  ASSERT_EQ(12300000000, all.m_uint64);
  ASSERT_EQ(true, all.m_bool);

  {
    vector<string> input;
    input.push_back("imastring");
    input.push_back("123");
    input.push_back("adf");
    input.push_back("");
    AssertEq<string>(input, all.m_vector_string);
  }

  {
    vector<double> output;
    output.push_back(12300000000.456);
    output.push_back(3.1415);
    output.push_back(-234.234);
    output.push_back(54335);
    AssertEq(output, all.m_vector_double);
  }

  {
    vector<float> output;
    output.push_back(123.456f);
    output.push_back(3.1415f);
    output.push_back(-234.234f);
    output.push_back(54335.0f);
    AssertEq(output, all.m_vector_float);
  }

  {
    vector<int32_t> output;
    output.push_back(123);
    output.push_back(0);
    output.push_back(-543);
    output.push_back(-1);
    AssertEq(output, all.m_vector_int32);
  }

  {
    vector<uint32_t> output;
    output.push_back(123);
    output.push_back(0);
    output.push_back(543);
    output.push_back(1);
    AssertEq(output, all.m_vector_uint32);
  }

  {
    vector<int64_t> output;
    output.push_back(12300000000);
    output.push_back(0);
    output.push_back(-12300000000);
    output.push_back(-1);
    AssertEq(output, all.m_vector_int64);
  }

  {
    vector<uint64_t> output;
    output.push_back(12300000000);
    output.push_back(0);
    output.push_back(554300000000);
    output.push_back(1);
    AssertEq(output, all.m_vector_uint64);
  }

  {
    vector<bool> output;
    output.push_back(true);
    output.push_back(false);
    output.push_back(true);
    output.push_back(true);
    AssertEq(output, all.m_vector_bool);
  }
}





TEST_F(NativeFieldHelperFixture, ToXml_string)
{
  ASSERT_STREQ("<testobj>imastring</testobj>", GetValueFromObj<string>("imastring", "string").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_double)
{
  ASSERT_STREQ("<testobj>12300000000.455999</testobj>", GetValueFromObj<double>(12300000000.456, "double").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_float)
{
  ASSERT_STREQ("<testobj>123.456001</testobj>", GetValueFromObj<float>(123.456f, "float").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_int32_t)
{
  ASSERT_STREQ("<testobj>-123</testobj>", GetValueFromObj<int32_t>(-123, "int").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_uint32_t)
{
  ASSERT_STREQ("<testobj>123</testobj>", GetValueFromObj<uint32_t>(123, "unsignedInt").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_int64_t)
{
  ASSERT_STREQ("<testobj>-12300000000</testobj>", GetValueFromObj<int64_t>(-12300000000, "long").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_uint64_t)
{
  ASSERT_STREQ("<testobj>12300000000</testobj>", GetValueFromObj<uint64_t>(12300000000, "unsignedLong").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_bool)
{
  ASSERT_STREQ("<testobj>1</testobj>", GetValueFromObj<bool>(true, "boolean").c_str());
}


TEST_F(NativeFieldHelperFixture, ToXml_vector_string)
{
  vector<string> input;
  input.push_back("imastring");
  input.push_back("123");
  input.push_back("adf");
  input.push_back("");

  const char* expected = "<testobj>"
    "<item type=\"xs:string\">imastring</item>"
    "<item type=\"xs:string\">123</item>"
    "<item type=\"xs:string\">adf</item>"
    "<item type=\"xs:string\"></item>"
    "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<string>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_double)
{
  vector<double> input;
  input.push_back(12300000000.456);
  input.push_back(3.1415);
  input.push_back(-234.234);
  input.push_back(54335);

  const char* expected = "<testobj>"
      "<item type=\"xs:double\">12300000000.455999</item>"
      "<item type=\"xs:double\">3.141500</item>"
      "<item type=\"xs:double\">-234.234000</item>"
      "<item type=\"xs:double\">54335.000000</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<double>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_float)
{
  vector<float> input;
  input.push_back(123.456f);
  input.push_back(3.1415f);
  input.push_back(-234.234f);
  input.push_back(54335.0f);

  const char* expected = "<testobj>"
      "<item type=\"xs:float\">123.456001</item>"
      "<item type=\"xs:float\">3.141500</item>"
      "<item type=\"xs:float\">-234.233994</item>"
      "<item type=\"xs:float\">54335.000000</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<float>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_int32_t)
{
  vector<int32_t> input;
  input.push_back(123);
  input.push_back(0);
  input.push_back(-543);
  input.push_back(-1);

  const char* expected = "<testobj>"
      "<item type=\"xs:int\">123</item>"
      "<item type=\"xs:int\">0</item>"
      "<item type=\"xs:int\">-543</item>"
      "<item type=\"xs:int\">-1</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<int>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_uint32_t)
{
  vector<uint32_t> input;
  input.push_back(123);
  input.push_back(0);
  input.push_back(543);
  input.push_back(1);

  const char* expected = "<testobj>"
      "<item type=\"xs:unsignedInt\">123</item>"
      "<item type=\"xs:unsignedInt\">0</item>"
      "<item type=\"xs:unsignedInt\">543</item>"
      "<item type=\"xs:unsignedInt\">1</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<unsignedInt>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_int64_t)
{
  vector<int64_t> input;
  input.push_back(12300000000);
  input.push_back(0);
  input.push_back(-12300000000);
  input.push_back(-1);

  const char* expected = "<testobj>"
      "<item type=\"xs:long\">12300000000</item>"
      "<item type=\"xs:long\">0</item>"
      "<item type=\"xs:long\">-12300000000</item>"
      "<item type=\"xs:long\">-1</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<long>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_uint64_t)
{
  vector<uint64_t> input;
  input.push_back(12300000000);
  input.push_back(0);
  input.push_back(554300000000);
  input.push_back(1);

  const char* expected = "<testobj>"
      "<item type=\"xs:unsignedLong\">12300000000</item>"
      "<item type=\"xs:unsignedLong\">0</item>"
      "<item type=\"xs:unsignedLong\">554300000000</item>"
      "<item type=\"xs:unsignedLong\">1</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<unsignedLong>").c_str());
}

TEST_F(NativeFieldHelperFixture, ToXml_vector_bool)
{
  vector<bool> input;
  input.push_back(true);
  input.push_back(false);
  input.push_back(true);
  input.push_back(true);

  const char* expected = "<testobj>"
      "<item type=\"xs:boolean\">1</item>"
      "<item type=\"xs:boolean\">0</item>"
      "<item type=\"xs:boolean\">1</item>"
      "<item type=\"xs:boolean\">1</item>"
      "</testobj>";

  ASSERT_STREQ(expected, GetValueFromObj(input, "vector<boolean>").c_str());
}




TEST_F(NativeFieldHelperAllFixture, ToXml_All)
{
  AllValues all;

  all.m_string = "imastring";
  all.m_double = 12300000000.456;
  all.m_float = 123.456f;
  all.m_int32 = -123;
  all.m_uint32 = 123;
  all.m_int64 = -12300000000;
  all.m_uint64 = 12300000000;
  all.m_bool = true;
  
  all.m_vector_string.push_back("imastring");
  all.m_vector_string.push_back("123");
  all.m_vector_string.push_back("adf");
  all.m_vector_string.push_back("");

  all.m_vector_double.push_back(12300000000.456);
  all.m_vector_double.push_back(3.1415);
  all.m_vector_double.push_back(-234.234);
  all.m_vector_double.push_back(54335);

  all.m_vector_float.push_back(123.456f);
  all.m_vector_float.push_back(3.1415f);
  all.m_vector_float.push_back(-234.234f);
  all.m_vector_float.push_back(54335.0f);

  all.m_vector_int32.push_back(123);
  all.m_vector_int32.push_back(0);
  all.m_vector_int32.push_back(-543);
  all.m_vector_int32.push_back(-1);

  all.m_vector_uint32.push_back(123);
  all.m_vector_uint32.push_back(0);
  all.m_vector_uint32.push_back(543);
  all.m_vector_uint32.push_back(1);

  all.m_vector_int64.push_back(12300000000);
  all.m_vector_int64.push_back(0);
  all.m_vector_int64.push_back(-12300000000);
  all.m_vector_int64.push_back(-1);

  all.m_vector_uint64.push_back(12300000000);
  all.m_vector_uint64.push_back(0);
  all.m_vector_uint64.push_back(554300000000);
  all.m_vector_uint64.push_back(1);

  all.m_vector_bool.push_back(true);
  all.m_vector_bool.push_back(false);
  all.m_vector_bool.push_back(true);
  all.m_vector_bool.push_back(true);


  tinyxml2::XMLDocument doc;
  auto el = nativeHelper->GenerateRequest("testobj", &all, doc);
  doc.LinkEndChild(el);

  tinyxml2::XMLPrinter printer(0, true);
  doc.Accept(&printer);

  ASSERT_STREQ(GetXml(), printer.CStr());
}
