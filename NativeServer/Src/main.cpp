
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

static void InvokePing(Echo_Stub* echo)
{
	::google::protobuf::RpcController* controller = 0;
	::google::protobuf::Closure* closure = 0;

	PingRequest request;
	PingResponse response;

	echo->Ping(controller, &request, &response, closure);
}

int main()
{
	CalculatorImpl calc;

	SoapServer server(666, 667);

	server.BindProtobufInbound<Calculator>(calc);
	server.BindProtobufOutput<Echo>();


	Echo_Stub echo(server.GetRpcChannel());


	server.Start(bind(&InvokePing, &echo));
}