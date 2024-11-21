using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Pipes;

namespace IPCPipes
{
    class Program
    {
        static void Main(string[] args)
        {
            const string pipeName = "testpipe";
            using (var server = new NamedPipeServerStream(pipeName, PipeDirection.InOut, 1, PipeTransmissionMode.Byte))
            {
                server.WaitForConnection();
                Console.WriteLine("Client connected!");
                while (server.IsConnected)
                {
                    byte[] buffer = new byte[1024];
                    int bytesRead = server.Read(buffer, 0, buffer.Length);
                    if (bytesRead > 0)
                    {
                        string recievedString = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                        Console.WriteLine($"Recieved {recievedString}");

                        string response = $"Echo {recievedString}";
                        byte[] responseBytes = Encoding.UTF8.GetBytes(response);
                        server.Write(responseBytes, 0, responseBytes.Length);
                        server.Flush();
                    }
                }
                Console.WriteLine("Client disconnected!");
            }
              
        }
    }
}
