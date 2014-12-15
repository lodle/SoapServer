//----------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------

using Microsoft.Samples.NetTcp.BattleNet;
using System;
using System.ServiceModel;

namespace Microsoft.Samples.NetTcp
{
    //The service contract is defined in generatedClient.cs, generated from the service by the svcutil tool.

    class EchoImpl : IEchoCallback
    {
        public PingResponse Ping(PingSolicit request)
        {
            Console.WriteLine("Ping!");
            return new PingResponse();
        }
    }

    class NativeEchoImpl : INativeEchoCallback
    {
        public PingResponse1 Ping(PingSolicit1 request)
        {
            Console.WriteLine("Native Ping!");
            return new PingResponse1();
        }
    }

    //Client implementation code.
    class Client
    {
        static void Main()
        {
            RunNativeEcho();

            RunEcho();

            RunNative();

            RunProtobuf();

            Console.WriteLine();
            Console.WriteLine("Press <ENTER> to terminate client.");
            Console.ReadLine();
        }

        private static void RunNativeEcho()
        {
            InstanceContext site = new InstanceContext(new NativeEchoImpl());

            NativeEchoClient echo = new NativeEchoClient(site);
            echo.Open();
        }

        private static void RunEcho()
        {
            InstanceContext site = new InstanceContext(new EchoImpl());

            EchoClient echo = new EchoClient(site);
            echo.Open();
        }

        private static void RunNative()
        {
            //Create a client
            NativeCalculatorClient client = new NativeCalculatorClient();

            // Call the Add service operation.
            double value1 = 100.00D;
            double value2 = 15.99D;
            double result = client.Add(value1, value2);
            Console.WriteLine("Add({0},{1}) = {2}", value1, value2, result);

            // Call the Subtract service operation.
            value1 = 145.00D;
            value2 = 76.54D;
            result = client.Subtract(value1, value2);
            Console.WriteLine("Subtract({0},{1}) = {2}", value1, value2, result);

            // Call the Multiply service operation.
            value1 = 9.00D;
            value2 = 81.25D;
            result = client.Multiply(value1, value2);
            Console.WriteLine("Multiply({0},{1}) = {2}", value1, value2, result);

            // Call the Divide service operation.
            value1 = 22.00D;
            value2 = 7.00D;
            result = client.Divide(value1, value2);
            Console.WriteLine("Divide({0},{1}) = {2}", value1, value2, result);

            //Closing the client gracefully closes the connection and cleans up resources
            client.Close();
        }

        private static void RunProtobuf()
        {

            //Create a client
            CalculatorClient client = new CalculatorClient();

            // Call the Add service operation.
            double value1 = 100.00D;
            double value2 = 15.99D;
            double result = client.Add(value1, value2);
            Console.WriteLine("Add({0},{1}) = {2}", value1, value2, result);

            // Call the Subtract service operation.
            value1 = 145.00D;
            value2 = 76.54D;
            result = client.Subtract(value1, value2);
            Console.WriteLine("Subtract({0},{1}) = {2}", value1, value2, result);

            // Call the Multiply service operation.
            value1 = 9.00D;
            value2 = 81.25D;
            result = client.Multiply(value1, value2);
            Console.WriteLine("Multiply({0},{1}) = {2}", value1, value2, result);

            // Call the Divide service operation.
            value1 = 22.00D;
            value2 = 7.00D;
            result = client.Divide(value1, value2);
            Console.WriteLine("Divide({0},{1}) = {2}", value1, value2, result);

            //Closing the client gracefully closes the connection and cleans up resources
            client.Close();
        }
    }
}
