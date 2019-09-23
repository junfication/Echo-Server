#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// includes
#include <windows.h>
#include <winsock2.h> // winsock
#include <ws2tcpip.h> // winsock functions
#include <stdio.h> // printf
#include <iostream> // std::cin, std::getline
#include <string> // std::string
#include <algorithm> // std::copy

#define LENGTH 512 // length of the arrays used, for I/O
#define HEADER 5 // size of the header, for the message

// Need to link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int __cdecl main(void)
{
  // array to hold the host name
  char hostname[512];
  // socket for listening
  SOCKET ListenSock;
  // socket to transfer data
  SOCKET DataSock;
  // struct to hold info of winsock
  WSADATA data;
  // zero the memory of the data
  SecureZeroMemory(&data, sizeof(data));
  // init winsock
  int result = WSAStartup(MAKEWORD(2, 2), &data);
  // if init fails
  if (result)
  {
    printf("WSAStartup failed: %d\n", result);
    return 1;
  }
  // store the version of the winsock
  int major = LOBYTE(data.wVersion);
  int minor = HIBYTE(data.wVersion);
  // print the version of winsock
  printf("WinSock version : %d.%d\n", major, minor);
  do
  {
    // string to store the user input of port
    std::string port_input;

    // create a socket for listening
    ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // if the listening socket creation has an error
    if (ListenSock == INVALID_SOCKET)
    {
      printf("socket failed with error: %ld\n", WSAGetLastError());
      WSACleanup();
      return 2;
    }
    // get the hostname and store it in the hostname array
    result = gethostname(hostname, LENGTH);
    // if there is an error in gethostname
    if (result == SOCKET_ERROR)
    {
      printf("Gethostname failed with error: %ld\n", WSAGetLastError());
      WSACleanup();
      return 9;
    }
    // create a addrinfo to allocate the socket to, hints to store the info of the result_addr
    addrinfo * result_addr, hints;
    // zero the memory space of hints
    SecureZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // using reliable byte stream TCP
    hints.ai_protocol = IPPROTO_TCP; // using TCP
    hints.ai_flags = AI_PASSIVE; // telling the socket that it will be use for binding later

    // prompt for user to input port num
    printf("Please input port number...\n");

    // store port num in port_input string
    std::getline(std::cin, port_input);

    printf("\n");

    // get the address info of the host and port, using hints as a reference to set the type of
    // connection and set the result at result_addr
    result = getaddrinfo(hostname, port_input.c_str(), &hints, &result_addr);

    // if there is an error getting the address info
    if (result != 0)
    {
      printf("Getaddrinfo failed with error: %ld\n", result);
      WSACleanup();
      return 10;
    }

    // printf the host name
    printf("Hostname: %s\n", hostname);

    // create a sockaddr to store the address where the socket will connect
    sockaddr_in* addr;

    // store the binary address to the sockaddr_
    addr = (sockaddr_in*)result_addr->ai_addr;
    // make a pointer to point to the address
    void* ip = &addr->sin_addr;
    // create a char array to store the ip address of the host
    char ip_addr[LENGTH];
    // convert the host form binary to text and store it in the ip_addr array
    inet_ntop(AF_INET, ip, ip_addr, INET_ADDRSTRLEN);

    // print out the ip address and port
    printf("IP Address: %s\n", ip_addr);
    printf("Port Number: %s\n", port_input.c_str());

    // bind the listening socket, if got error print out error
    if (bind(ListenSock, (SOCKADDR *)addr, sizeof(*addr)) == SOCKET_ERROR)
    {
      printf("bind failed with error: %ld\n", WSAGetLastError());
      closesocket(ListenSock);
      WSACleanup();
      return 3;
    }

    // after bindign the listening socket, we wont use the addr anymore so we can free it
    freeaddrinfo(result_addr);

    // listening on the listen socket
    if (listen(ListenSock, 1) == SOCKET_ERROR)
    {
      printf("listen failed with error: %ld\n", WSAGetLastError());
      closesocket(ListenSock);
      WSACleanup();
      return 4;
    }

    // print status of server
    printf("Waiting for client to connect...\n");

    // we will accept if there is any request in the listening, and create another
    // socket for data transfer, in the dataSock socket, using the ListenSock, ip
    // and port
    DataSock = accept(ListenSock, NULL, NULL);

    // if there is an error in accept and DataSocket is no created
    if (DataSock == INVALID_SOCKET)
    {
      printf("accept failed with error: %ld\n", WSAGetLastError());
      closesocket(ListenSock);
      WSACleanup();
      return 5;
    }

    // close the ListenSock as we no need to listen anymore ( single connection, 1 to 1 connection)
    closesocket(ListenSock);
    
    // print status of server
    printf("Client connected.\n\n");

    // char array to store the message sent by the client
    char recvbuf[LENGTH];
    // char array to store the message text sent by the client
    char printbuf[LENGTH];
    // the length of the message text
    int msg_len;
    // sendresult of the echo process whether there is an error
    int sendResult;

    do
    {
      // print status of server
      printf("Receiving...\n\n");

      // result now will be the bytes of the message
      result = recv(DataSock, recvbuf, LENGTH, 0);
      
      // result > 0, means got message
      if (result > 0)
      {
        // print the size of the message
        printf("Bytes received: %d\n", result);
        // set the msg_len to be the length of the message text, convert from network to host format
        msg_len = ntohl(*((int *)&recvbuf[1]));
        // print the size of the message text
        printf("Message size: %d\n", msg_len);
        // copy the message text to print buffer
        std::copy(recvbuf + HEADER, recvbuf + result, printbuf);
        // if not command line
        if(recvbuf[0])
         // print the print buffer aka message text
         printf("Message Text: %s\n", printbuf);
        // else command line
        else
          // print nothing
          printf("Message Text: \n");
        // newline
        printf("\n");

        // print status of server
        printf("Sending...\n");

        // echo back the message to the client
        sendResult = send(DataSock, recvbuf, result, 0);

        // if sending has error
        if (sendResult == SOCKET_ERROR)
        {
          printf("send failed with error: %ld\n", WSAGetLastError());
          closesocket(DataSock);
          WSACleanup();
          return 6;
        }
      }
      else //client dc 
      {
        printf("Client disconnected...\n");
        closesocket(DataSock);
      }
    } while (result > 0); // loop the send and recieve only when the client is connected
    // print status of server
    printf("\nReconnecting...\n\n");
  } while (true); // loop when client dc, will reconnect to another port

  // close data sock
  closesocket(DataSock);

  // clean up WSAdata
  WSACleanup();

  // return 0
  return 0;
}