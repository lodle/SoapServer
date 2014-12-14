
#include "SoapServer.h"
#include "calculator.pb.h"


class CalculatorImpl : public Calculator
{
public:
	virtual void Add(::google::protobuf::RpcController* controller,
		const ::CalculatorRequest* request,
		::CalculatorResponse* response,
		::google::protobuf::Closure* done)
	{
		response->set_result(request->param1() + request->param2());
	}

	virtual void Subtract(::google::protobuf::RpcController* controller,
		const ::CalculatorRequest* request,
		::CalculatorResponse* response,
		::google::protobuf::Closure* done)
	{
		response->set_result(request->param1() - request->param2());
	}

	virtual void Multiply(::google::protobuf::RpcController* controller,
		const ::CalculatorRequest* request,
		::CalculatorResponse* response,
		::google::protobuf::Closure* done)
	{
		response->set_result(request->param1() * request->param2());
	}

	virtual void Divide(::google::protobuf::RpcController* controller,
		const ::CalculatorRequest* request,
		::CalculatorResponse* response,
		::google::protobuf::Closure* done)
	{
		response->set_result(request->param1() / request->param2());
	}
};

static void OnPingResponse(PingResponse* response)
{
	delete response;
}

static void InvokePing(Echo* echo)
{
	::google::protobuf::RpcController* controller = 0;
	::google::protobuf::Closure* closure = 0;

	PingRequest request;
	PingResponse* response = new PingResponse();

	echo->Ping(controller, &request, response, google::protobuf::NewCallback(&OnPingResponse, response));
}

int main()
{
	shared_ptr<Calculator> calc(new CalculatorImpl());
	shared_ptr<Echo> echo;

	SoapServer server(666, 667);

	//Bind protobuf to allow client to call us
	server.BindProtobufInbound(calc);

	//Bind protobuf to allow us to call client
	echo = server.BindProtobufOutput<Echo_Stub>();

	//Callback is a hack to get back on this thread
	server.Start(bind(&InvokePing, echo.get()));
}