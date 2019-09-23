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


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define LENGTH 512 // length of the arrays used, for I/O
#define HEADER 5 // size of header, for the message

int __cdecl main(void)
{
  // struct to hold info of winsock
  WSADATA data;
  // bool to store whether we should quit or reconncet
  bool quit = false;

  // zero the memory of the data
  SecureZeroMemory(&data, sizeof(data));
  // init winsock
  int result = WSAStartup(MAKEWORD(2, 2), &data);
  // if init fails
  if (result != NO_ERROR)
  {
    wprintf(L"WSAStartup function failed with error: %d\n", result);
    return 1;
  }

  do
  {
    // string to store the user input of the port number
    std::string port_input;
    // string to  store the user input of the ip
    std::string ip_input;
    
    // create a addrinfo to allocate the socket to, hints to store the info of the result_addr
    addrinfo * result_addr = nullptr;
    // ptr to loop throught the result_addr to find the address
    addrinfo * ptr = nullptr;
    addrinfo   hints;
    // zero the hints memory
    SecureZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM; // using reliable byte stream TCP
    hints.ai_protocol = IPPROTO_TCP; // using TCP

    // prompt for the user to input server ip address
    printf("Please input server ip address...\n");

    // store ip address in ip_input
    std::getline(std::cin, ip_input);

    // prompt the user to input the server port number
    printf("Please input server port number...\n");

    // store the port number in port_input
    std::getline(std::cin, port_input);

    // get the address info of the server host and port, using hints as a reference to set the type of
    // connection and set the result at result_addr
    result = getaddrinfo(ip_input.c_str(), port_input.c_str(), &hints, &result_addr);

    // if the getaddr has error
    if (result != 0)
    {
      printf("Getaddrinfo failed with error: %ld\n", result);
      WSACleanup();
      return 10;
    }

    // set ptr to point to the beginning of the result_addr
    ptr = result_addr;

    // create a socket to connect to the server
    SOCKET ConnectSock;

    // loop through the result_addr to search for a address to connect to
    for (ptr = result_addr; ptr != nullptr; ptr = ptr->ai_next)
    {
      // create a connect socket
      ConnectSock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
      // if creation fails
      if (ConnectSock == INVALID_SOCKET)
      {
        printf("socket function failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 2;
      }

      // connect the connect socket to the ptr address
      result = connect(ConnectSock, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen));

      // if cannot connect, close the connect socket and continue to next loop
      if (result == SOCKET_ERROR)
      {
        closesocket(ConnectSock);
        ConnectSock = INVALID_SOCKET;
        continue;
      }
      // if connected, break from for loop
      break;
    }

    // free the result_addr as we no need to use it anymore
    freeaddrinfo(result_addr);

    // print status of client
    printf("Connected to server.\n\n");

    // string to store the user input of the message text
    std::string sendbuf;
    // recvbuf to store the whole message
    char recvbuf[LENGTH];
    // printbuf to store the message text after the echo to print from
    char printbuf[LENGTH];
    // length of the message text
    int msg_len;
    // command line to check whether it is the quit command
    char quitbuf[] = "\\q";

    do
    {
      // prompt for user to input message text
      printf("Please input...\n");
      // store the message text in sendbuf
      std::getline (std::cin, sendbuf);
      // newline
      printf("\n");
      // store the lenth of the message text without null char
      int len = (int)strlen(sendbuf.c_str());
      // alloc memory for the message, the command line flag, size of message text and message text itself
      char* msg = new char[HEADER + len + 1];
      // if the sendbuf is the command line
      if (sendbuf == quitbuf)
      {
        // set the command line flag to 0
        msg[0] = 0;
        // set the size flag to 0, host to network format
        *((int *)&msg[1]) = htonl(0);
      }
      else
      {
        // else not command line , set the command line flag to 1
        msg[0] = 1;
        // set the size flag to length of message text + 1(null char) and convert from host to network format
        *((int *)&msg[1]) = htonl(len + 1);
      }
      // copy the message text from the sendbuf to the message
      std::copy(sendbuf.c_str(), sendbuf.c_str() + len + 1, &msg[HEADER]);

      // status of the client
      printf("Sending...\n");

      // sending the message to server
      result = send(ConnectSock, msg, HEADER + len + 1, 0);

      // if sending got error
      if (result == SOCKET_ERROR)
      {
        // delete the message
        delete[] msg;
        printf("Server disconnected...\n");
        break;
      }
      // delete the message
      delete[] msg;

      // status of the client
      printf("Receiving...\n\n");

      // recieve the result from server, echo from server
      result = recv(ConnectSock, recvbuf, LENGTH, 0);

      // if the bytes received is more than 0
      if (result > 0)
      {
        // print the bytes received 
        printf("Bytes received: %d\n", result);
        // if is command line set quit to true
        if (recvbuf[0] == 0) quit = true;
        // converth the msg_len from network to host format
        msg_len = ntohl(*((int *)&recvbuf[1]));
        // print the size of the message text
        printf("Message size: %d\n", msg_len);
        // copy the message text from the recvbuf to the printbuf
        std::copy(recvbuf + HEADER, recvbuf + result, printbuf);
        // print the message text
        printf("Echo: %s\n", printbuf);
        // newline
        printf("\n");
      }
      else // server dc
        printf("recv function failed with error: %ld\n", WSAGetLastError());

    } while (!quit && result > 0); // loop send and recieve server has not dc and the client is not qutting

    // close the connect socket
    result = closesocket(ConnectSock);

    // if there is an error in closing the connect socket
    if (result == SOCKET_ERROR) 
    {
      printf("closesocket function failed with error: %ld\n", WSAGetLastError());
      WSACleanup();
      return 4;
    }
  }
  while (!quit); // loop when the server dc, will reconnect to another server

  // clean up WSAdata
  WSACleanup();

  // return 0
  return 0;
}
