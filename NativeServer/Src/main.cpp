
#include "SoapServer.h"
#include "calculator.pb.h"

#include <iostream>
#include <string>

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


class NativeCalculatorRequest
{
public:
	double param1;
	double param2;
};

class NativeCalculatorResponse
{
public:
	double response;
};

SoapTypeVisitor& operator<<(SoapTypeVisitor& visitor, const NativeCalculatorRequest& r)
{
	visitor << SoapClassVistior(NativeCalculatorRequest) 
		<< SoapFieldVisitor(r, param1) 
		<< SoapFieldVisitor(r, param2);
	return visitor;
}

SoapTypeVisitor& operator<<(SoapTypeVisitor& visitor, const NativeCalculatorResponse& r)
{
	visitor << SoapClassVistior(NativeCalculatorResponse) 
		<< SoapFieldVisitor(r, response);
	return visitor;
}

class NativeCalculator
{
public:
	void Add(const NativeCalculatorRequest& request, NativeCalculatorResponse& response)
	{
		response.response = request.param1 + request.param2;
	}

	void Subtract(const NativeCalculatorRequest& request, NativeCalculatorResponse& response)
	{
		response.response = request.param1 - request.param2;
	}

	void Multiply(const NativeCalculatorRequest& request, NativeCalculatorResponse& response)
	{
		response.response = request.param1 * request.param2;
	}

	void Divide(const NativeCalculatorRequest& request, NativeCalculatorResponse& response)
	{
		response.response = request.param1 / request.param2;
	}
};


class NativeEcho
{
public:
	virtual void Ping(const VoidRequestResponse&, VoidRequestResponse&) = 0;
};


class NativeEcho_Stub : public NativeEcho
{
public:
	NativeEcho_Stub()
	{

	}

	void Ping(const VoidRequestResponse&, VoidRequestResponse&)
	{

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

static void InvokeNativePing(NativeEcho* echo)
{
	VoidRequestResponse req;
	VoidRequestResponse resp;

	echo->Ping(req, resp);
}


int main()
{
	shared_ptr<NativeCalculator> nativeCalc(new NativeCalculator());
	shared_ptr<Calculator> calc(new CalculatorImpl());
	shared_ptr<Echo> echo;
	shared_ptr<NativeEcho> nativeEcho;

	SoapServer server(666, 667);

	//Bind protobuf to allow client to call us
	server.BindProtobufInbound(calc);

	server.BindNativeInbound<NativeCalculator>("NativeCalculator", nativeCalc)
		.Function("Add", &NativeCalculator::Add)
		.Function("Subtract", &NativeCalculator::Subtract)
		.Function("Multiply", &NativeCalculator::Multiply)
		.Function("Divide", &NativeCalculator::Divide);

	//Bind protobuf to allow us to call client
	echo = server.BindProtobufOutput<Echo_Stub>();

	server.BindNativeOutbound<NativeEcho>("NativeEcho", nativeEcho)
		.Function("Ping", &NativeEcho::Ping);



	server.Start();

	std::cout << "Running soap server on port: 666" << std::endl;
	std::cout << "Running mex server on: http://localhost:667" << std::endl << std::endl;

	std::cout << "Press return to exit or type ping to send ping...." << std::endl;

	std::string s;

	do
	{
		std::getline(cin, s);

		if (s == "ping")
		{
			InvokePing(echo.get());
		}
		else if (s == "nping")
		{
			InvokeNativePing(nativeEcho.get());
		}


	} while (!s.empty());

	server.Stop();
}