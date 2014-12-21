#pragma once


class SoapServer;

class ObjectWrapper
{
public:
  ObjectWrapper(void* obj, function<void()> deleter)
    : m_object(obj)
    , m_deleter(deleter)
  {
  }

  ~ObjectWrapper()
  {
    m_deleter();
  }

  template <typename T>
  static shared_ptr<ObjectWrapper> Create()
  {
    T* t = new T();
    function<void()> deleter = bind(&ObjectWrapper::DeleteObj<T>, t);
    ObjectWrapper* o = new ObjectWrapper(t, deleter);
    return shared_ptr<ObjectWrapper>(o);
  }

  void* get()
  {
    return m_object;
  }

protected:
  template <typename T>
  static void DeleteObj(T* obj)
  {
    delete obj;
  }

private:
  void* m_object;
  function<void()> m_deleter;
};


typedef function<void(const void*)> NativeResponseCallback;
typedef function<void(const void*, NativeResponseCallback)> NativeCallback;
typedef function<shared_ptr<ObjectWrapper>()> CreateObjectCallback;



namespace
{
  template <class N>
  struct is_vector 
  { 
    typedef N Type;
    static const bool Value = false;
  };

  template <class N, class A>
  struct is_vector<std::vector<N, A> > 
  {
    typedef N Type;
    static const bool Value = true;
  };

  template <typename T>
  string GetSoapTypeName()
  {
    if (is_vector<T>::Value)
    {
      return string("vector<") + GetSoapTypeName<is_vector<T>::Type>() + ">";
    }
    else
    {
      return typeid(T).name();
    }
  }

  template <>
  string GetSoapTypeName<string>()
  {
    return "string";
  }

  template <>
  string GetSoapTypeName<bool>()
  {
    return "boolean";
  }

  template <>
  string GetSoapTypeName<uint64_t>()
  {
    return "unsignedLong";
  }

  template <>
  string GetSoapTypeName<uint32_t>()
  {
    return "unsignedInt";
  }

  template <>
  string GetSoapTypeName<int64_t>()
  {
    return "long";
  }

  template <>
  string GetSoapTypeName<int32_t>()
  {
    return "int";
  }

  template <typename R>
  void CallNativeResponse(const R& r, NativeResponseCallback callback)
  {
    callback(&r);
  }

  template <typename T, typename A, typename R>
  void CallNative(shared_ptr<T> t, void (T::*funct)(const A&, function<void(const R&)>), const void* aIn, NativeResponseCallback callback)
  {
    function<void(const R&)> nativeCallback = bind(&CallNativeResponse<R>, placeholders::_1, callback);

    const A& a = *reinterpret_cast<const A*>(aIn);
    (*t.get().*funct)(a, nativeCallback);
  }

  template <typename T>
  void DeleteObject(void* obj)
  {
    delete reinterpret_cast<T*>(obj);
  }
}



template <typename T>
class SoapNativeFunctionBinderInbound
{
public:
  SoapNativeFunctionBinderInbound(SoapServer* server, const shared_ptr<T>& t, const string& serviceName)
    : m_server(server)
    , m_serviceName(serviceName)
    , m_t(t)
  {
  }

  template <typename A, typename R>
  SoapNativeFunctionBinderInbound<T>& Function(const string& name, void (T::*funct)(const A&, function<void(const R&)>))
  {
    const ClassBinding& request = m_server->GetClassBinding<A>();
    const ClassBinding& response = m_server->GetClassBinding<R>();

    NativeCallback callback = bind(&CallNative<T, A, R>, m_t, funct, placeholders::_1, placeholders::_2);
    m_server->AddMethod(m_serviceName, name, request, response, callback, true);

    return *this;
  }

private:
  SoapServer* m_server;
  string m_serviceName;
  shared_ptr<T> m_t;
};


class TypeWrapper
{
public:
  TypeWrapper(const type_info& info)
    : m_info(info)
  {

  }

  const type_info& get()
  {
    return m_info;
  }

private:
  const type_info& m_info;
};




template <typename REQ, typename RESP>
class NativeSoapCall
{
public:
  void operator()(function<void(const RESP&)> callback, const REQ& req)
  {
    assert(m_callsoap);
    m_callsoap(bind(&DoCallback, callback, placeholders::_1), &req);
  }

protected:
  static void DoCallback(function<void(const RESP&)> callback, const void* arg)
  {
    callback(*reinterpret_cast<const RESP*>(arg));
  }

  template <typename T>
  friend class SoapNativeFunctionBinderOutbound;

  function<void(function<void(const void*)>, const REQ*)> m_callsoap;
};




template <typename T>
class SoapNativeFunctionBinderOutbound
{
public:
  SoapNativeFunctionBinderOutbound(SoapServer* server, const shared_ptr<T>& t, const string& serviceName)
    : m_server(server)
    , m_serviceName(serviceName)
    , m_t(t)
  {
  }

  template <typename A, typename R>
  SoapNativeFunctionBinderOutbound<T>& Function(const string& name, NativeSoapCall<A, R> &invoker)
  {
    const ClassBinding& request = m_server->GetClassBinding<A>();
    const ClassBinding& response = m_server->GetClassBinding<R>();

    CallInfo info(m_server, m_serviceName, name, typeid(A), typeid(R));

    invoker.m_callsoap = bind(&CallSoap, info, placeholders::_1, placeholders::_2);

    NativeCallback nc;
    m_server->AddMethod(m_serviceName, name, request, response, nc, false);
    return *this;
  }

protected:
  class CallInfo
  {
  public:
    CallInfo(SoapServer* server, const string& serviceName, const string& name, const type_info& reqType, const type_info& respType)
      : m_server(server)
      , m_serviceName(serviceName)
      , m_name(name)
      , m_reqType(reqType)
      , m_respType(respType)
    {

    }
    SoapServer* m_server;
    string m_serviceName;
    string m_name;
    const type_info& m_reqType;
    const type_info& m_respType;
  };

  static void CallSoap(CallInfo info, function<void(const void*)> callback, const void* resp)
  {
    info.m_server->CallMethod(info.m_serviceName, info.m_name, resp, callback, info.m_reqType, info.m_respType);
  }

private:
  SoapServer* m_server;
  string m_serviceName;
  shared_ptr<T> m_t;
};



template <typename T>
class SoapFieldVisitorT
{
public:
  SoapFieldVisitorT(const string& name, size_t offset)
    : m_name(name)
    , m_offset(offset)
    , m_size(sizeof(T))
  {
  }

  const string m_name;
  const size_t m_offset;
  const size_t m_size;
};

template <typename T>
class SoapClassVisitorT
{
public:
  SoapClassVisitorT(const string &name)
    : m_name(name)
  {
  }

  const string m_name;
};

template <typename T>
SoapFieldVisitorT<T> SoapFieldVisitorCreate(const string& name, size_t offset, const T&)
{
  return SoapFieldVisitorT<T>(name, offset);
}

#define SoapFieldVisitor( Obj, Field )		\
	SoapFieldVisitorCreate(					\
		#Field ,							\
		((size_t)(&reinterpret_cast<char const volatile&>(Obj.Field) - &reinterpret_cast<char const volatile&>(Obj))),	\
		Obj.Field							\
	)

#define SoapClassVistior( Klass ) SoapClassVisitorT<Klass>( #Klass )

class SoapTypeVisitor
{
public:
  SoapTypeVisitor(SoapServer* server)
    : m_server(server)
  {
  }

  template <typename T>
  SoapTypeVisitor& operator<<(SoapFieldVisitorT<T> info)
  {
    m_fields.push_back(&m_server->GetFieldBinding<T>(info.m_name, info.m_offset));
    return *this;
  }

  template <typename T>
  SoapTypeVisitor& operator<<(SoapClassVisitorT<T> info)
  {
    return *this;
  }

  vector<const FieldBinding*>& GetFields()
  {
    return m_fields;
  }

private:
  vector<const FieldBinding*> m_fields;
  SoapServer* m_server;
};
