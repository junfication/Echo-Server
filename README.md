# Echo-Server
A simple client and server where the server will echo and send back to the client
Dependencies : Visual Studio 2015

# Usage
## Server

Upon compiling and running the visual studio project, the server program
will ask the user for a port number. And after entering the port number,
the server will connect to the port and print all the details which the
server is connected for the client to connect to.

In any case of the client were to disconnect, the server will go prompt 
the user to reenter a port number to reconnect again.

## Client

Upon compiling and running the visual studio project, the client program
will ask the user for the server's ip address and port number. This can
be gotten from the server side, after enter the port number into the
server will print out the details. You just have to input them into the
client.

Upon connected to the server, the program will prompt the user to input
the message to be sent to the server.

If the user input \q, it will be treated as a command line exit, and the
client will shutdown. While the server will try to reconnect to a port.

In any case of the server were to disconnect, the client will prompt the
user to reenter a ip address and port number to reconnect again.
