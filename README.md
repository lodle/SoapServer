SoapServer
==========
Auto binding Protobuf rpc and native classes exposing a soap interface with mex


Todo
==========
Protobuf Binding:
 * Repeated Fields
 * Enums
 * Message Fields
 * Inherited Messages
 * Extensions
 
Native Bindings:
 * Server invoke
 * Enums
 * Class Fields
 * Inherited Classes
 * pod other than double
 
Threading:
 * Implement sain thread model (i.e. all message callbacks on same thread)
 * Split response into callback as not to block thread
