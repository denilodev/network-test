#include <ctype.h>
#include <stdio.h>
#include <winsock2.h>

#define SERVER_CHAR 's'
#define CLIENT_CHAR 'c'
#define MIN_PORT_NO 5000
#define MAX_PORT_NO 65535
#define GOT_WRONG   < 0
#define TRUE        1
#define FALSE       0
#define BUFFER_LENGHT 512
#define MAX_CLIENTS 3
#define SADDR       struct sockaddr_in

// User type
char type = '\0';

// Socket global variables
char ADDRESS[16] = "127.0.0.1";
u_int PORT = 123;
char running = TRUE;
int result = 0, send_result = 0;

// Functions
void ask_address();
void ask_port();
int server();
int client();
DWORD WINAPI send_thread_function(LPVOID lp_parameter);
int initialization();
SOCKET create_socket();
//SADDR create_saddr();


int main()
{
    // Asks if is server or client
    printf("> Type (%c) if you're server or (%c) if you're a client: ", SERVER_CHAR, CLIENT_CHAR);
    while (TRUE)
    {
        scanf("%c", &type);
        // Ignore new line characters and just ask the input again
        if (type == '\n') continue;
        if (type != SERVER_CHAR && type != CLIENT_CHAR)
        {
            // If is not server or client, reset the loop
            printf("> Invalid answer, type (%c) if you're server or (%c) if you're a client: ", SERVER_CHAR, CLIENT_CHAR);
            continue;
        }
        break;
    }
   
    // Get the ADDRESS
    ask_address();

    // Get the PORT
    ask_port();

    if (type == CLIENT_CHAR) return client();
    else if (type == SERVER_CHAR) return server();
    else { printf(">>> Weird error!"); return 1; }
}

void ask_address()
{
    printf("> Type the ADDRESS in a dotted-decimal format (X.X.X.X): \n");
    while (TRUE)
    {
        scanf("%s", ADDRESS);
        // Ignore new line character and just ask the input again
        if (ADDRESS == "\n") continue;
        // Check if PORT is valid
        int address_lenght = strlen(ADDRESS);
        char is_address_valid = FALSE;
        for (char i = 0, new_octet = TRUE, octet_count = 1; i <= address_lenght; i++)
        {
            if (new_octet)
            {
                if (isdigit(ADDRESS[i])) new_octet = FALSE;
                else i = 99;
            }
            else
            {
                if (isdigit(ADDRESS[i])) continue;
                else if (ADDRESS[i] == '.')
                {
                    new_octet = TRUE;
                    octet_count++;
                }
                else if (ADDRESS[i] == '\0' && octet_count == 4)
                {
                    is_address_valid = TRUE;
                }
                else i = 99;
            }
        }
        if (is_address_valid) break;
        else printf("> Type a valid ADDRESS: ");
    }
}

void ask_port()
{
    printf("> Type the PORT to be used (from %d to %d): ", MIN_PORT_NO, MAX_PORT_NO);
    while (TRUE)
    {
        scanf("%u", &PORT);
        // Check if PORT is valid
        if (PORT < MIN_PORT_NO || PORT > MAX_PORT_NO)
        {
            printf("> Invalid answer, type a valid port number: ");
            continue;
        }
        break;
    }
}

int server()
{
    // Initialization
    if (!initialization()) return 1;

    // Socket construction and bind to the addres
    SOCKET listener = create_socket();
    if (listener == INVALID_SOCKET) return 1;

    // Setup for multiple connections
    char multiple = 1;
    result = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &multiple, sizeof(multiple));
    if (result GOT_WRONG)
    {
        printf(">>> Multiple client setup failed. Error %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup;
        return 1;
    }

    // Set as listener
    result = listen(listener, SOMAXCONN);
    if (result GOT_WRONG)
    {
        printf(">>> Listen failed. Error %d\n", WSAGetLastError());
        closesocket(listener);
        WSACleanup();
        return 1;
    }
    printf("> Accepting connection on %s:%d\n", ADDRESS, PORT);

    // Array of clients
    SOCKET client[MAX_CLIENTS];
    // Clear trash values from client array
    memset(client, 0, MAX_CLIENTS * sizeof(SOCKET));
    // Active slots in the array
    int client_count = 0;
    // Variables to be able to set clients
    fd_set socket_set;
    struct sockaddr_in client_address;
    int client_address_lenght;

    SOCKET sock, sock_max;
    // Buffer that store the received messages
    char recv_buffer[BUFFER_LENGHT];
    // Server state 
    running = TRUE;

    // Some strings to be used (messages and commands)
    enum text_enum {
        MSG_WELCOME,
        MSG_GOODBYE,
        MSG_FULL,
        MSG_COMMAND_NOT_FOUND,
        CMD_CLOSE,
        MSG_CLOSE,

        TEXT_COUNT
    };
    char *text[TEXT_COUNT];
    text[MSG_WELCOME] = "Welcome to the server :)";
    text[MSG_GOODBYE] = "See you later!";
    text[MSG_FULL] = "Sorry, the server is full :(";
    text[MSG_COMMAND_NOT_FOUND] = "Command not found.";
    text[CMD_CLOSE] = "/close";
    text[MSG_CLOSE] = "Closing the server.";
    // The lenght of each string
    int lenght[TEXT_COUNT];
    for (int i = 0; i < TEXT_COUNT; i++) lenght[i] = strlen(text[i]);

    // Main Loop
    while (running)
    {
        // Clear the set
        FD_ZERO(&socket_set);

        // Add listener socket
        FD_SET(listener, &socket_set);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            sock = client[i];
            if (sock > 0)
            {
                // Add an active client to the set
                FD_SET(sock, &socket_set);
            }
            if (sock > sock_max)
            {
                sock_max = sock;
            }
        }

        // Handle multiple connections
        int activity = select(sock_max + 1, &socket_set, NULL, NULL, NULL);
        if (activity < 0) continue;

        // Determine if listener has activity
        if (FD_ISSET(listener, &socket_set))
        {
            // Accept connection
            sock = accept(listener, NULL, NULL);
            if (sock == INVALID_SOCKET)
            {
                printf(">>> Accept connection failed. Error %d\n", WSAGetLastError());
            }

            // Get client information
            getpeername(sock, (struct sockaddr *)&client_address, &client_address_lenght);
            printf("> Client connected at %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
            
            // Add to array
            if (client_count < MAX_CLIENTS)
            {
                // Scan through list
                int i;
                for (i = 0; i < MAX_CLIENTS; i++)
                {
                    if (!client[i])
                    {
                        client[i] = sock;
                        printf("> Added to the list at index %d\n", i);
                        client_count++;
                        break;
                    }
                }

                // Send MSG_WELCOME message to new client
                send_result = send(sock, text[MSG_WELCOME], lenght[MSG_WELCOME], 0);
                if (send_result != lenght[MSG_WELCOME])
                {
                    printf(">>> Failed trying to send Welcome message. Error %d\n", WSAGetLastError());
                    shutdown(sock, SD_BOTH);
                    closesocket(sock);
                }
            }
            else
            {
                printf("> New client tried to connected, but server is full.\n");

                // Send overflow message to new client and disconnects it
                send_result = send(sock, text[MSG_FULL], lenght[MSG_FULL], 0);
                if (send_result != lenght[MSG_FULL])
                {
                    printf(">>> Failed trying to send Full message. Error %d\n", WSAGetLastError());
                }
                shutdown(sock, SD_BOTH);
                closesocket(sock);
            }
        }

        // Iterate through clients
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!client[i]) continue;

            sock = client[i];

            // Determine if client has activity
            if (FD_ISSET(sock, &socket_set))
            {
                // Get message
                result = recv(sock, recv_buffer, BUFFER_LENGHT, 0);
                if (result > 0)
                {
                    // Print message received
                    recv_buffer[result-1] = '\0';
                    printf("<Client %d>: %s\n", i, recv_buffer);
                    
                    // Check if is a command (start with "\")
                    if (recv_buffer[0] == '/')
                    {
                        // Check if client type the CLOSE command
                        if (!memcmp(recv_buffer, text[CMD_CLOSE], lenght[CMD_CLOSE]))
                        {
                            send_result = send(sock, text[MSG_CLOSE], lenght[MSG_CLOSE], 0);
                            if (send_result GOT_WRONG)
                            {
                                printf(">>> Answer the received message failed. Error %d", WSAGetLastError());
                                shutdown(sock, SD_BOTH);
                                closesocket(sock);
                                client[i] = 0;
                                client_count--;
                            }
                            // Stop the server
                            running = FALSE;
                            break;
                        }
                        else
                        {
                            send_result = send(sock, text[MSG_COMMAND_NOT_FOUND], lenght[MSG_COMMAND_NOT_FOUND], 0);
                            if (send_result GOT_WRONG)
                            {
                                printf(">>> Answer the received message failed. Error %d", WSAGetLastError());
                                shutdown(sock, SD_BOTH);
                                closesocket(sock);
                                client[i] = 0;
                                client_count--;
                            }
                        }
                    }
                    else
                    {                        
                        // Echo the received message
                        send_result = send(sock, recv_buffer, result, 0);
                        if (send_result GOT_WRONG)
                        {
                            printf(">>> Echo the received message failed. Error %d", WSAGetLastError());
                            shutdown(sock, SD_BOTH);
                            closesocket(sock);
                            client[i] = 0;
                            client_count--;
                        }
                    }
                }
                else
                {
                    // Disconnect client
                    getpeername(sock, (struct sockaddr *)&client_address, &client_address_lenght);
                    printf("> Client disconnected at %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));
                    shutdown(sock, SD_BOTH);
                    client[i] = 0;
                    client_count--;
                }
            }
        }
    }
    
    // Disconnect all clients
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // Send Goodbye message 
        send(client[i], text[MSG_GOODBYE], lenght[MSG_GOODBYE], 0);
        shutdown(client[i], SD_BOTH);
        closesocket(client[i]);
        client[i] = 0;
    }
    // Shutdown socket
    closesocket(listener);
    // Clean up WSA
    WSACleanup();

    printf("> End of the program!\n");
    return 0;
}

DWORD WINAPI send_thread_function(LPVOID lp_parameter)
{
    SOCKET client = *(SOCKET *)lp_parameter;

    char send_buffer[BUFFER_LENGHT];
    int send_buffer_lenght;

    while (running)
    {
        //scanf("%s", send_buffer);
        fgets(send_buffer, BUFFER_LENGHT, stdin);

        if (!running) break;

        send_buffer_lenght = strlen(send_buffer);
        send_result = send(client, send_buffer, send_buffer_lenght, 0);
        if (send_result != send_buffer_lenght)
        {
            printf(">>> Send failed.\n");
            break;
        }
    }
}

int client()
{
    // Initialization
    if (!initialization()) return 1;

    // Socket construction and connection to the address
    SOCKET client = create_socket();
    if (client == INVALID_SOCKET) return 1;

    // Announce the connection
    printf("> Successfully connected to %s:%d\n", ADDRESS, PORT);
    running = TRUE;

    // Start send thread
    DWORD thread_id;
    HANDLE send_thread = CreateThread(NULL, 0, send_thread_function, &client, 0, &thread_id);
    if (send_thread) printf("> Send thread started with thread ID \"%d\".\n", thread_id);
    else printf("Send thread failed. Error %d", GetLastError());

    // Receive loop
    char recv_buffer[BUFFER_LENGHT];
    do
    {
        result = recv(client, recv_buffer, BUFFER_LENGHT, 0);
        recv_buffer[result] = '\0';
        if (result > 0)     {printf("<Server>: %s\n", recv_buffer);}
        else if (!result)   {printf("> Connection closed.\n");                      running = FALSE;}
        else                {printf(">>> Receive failed: %d\n", WSAGetLastError()); running = FALSE;}
    } while (running & result > 0);
    running = FALSE;

    // Connection finished, terminate send thread
    if (CloseHandle(send_thread)) printf("> Send thread successfully closed.\n");
    else printf(">>> Send thread closing failed.\n");

    // Cleanup
    result = shutdown(client, SD_BOTH);
    if (result GOT_WRONG)
    {
        printf("> Shutdown failed. Error %d\n", WSAGetLastError());
        closesocket(client);
        WSACleanup();
        return 1;
    }
    // Shutdown socket
    closesocket(client);
    // Clean up WSA
    WSACleanup();

    printf("> End of the program!\n");
    return 0;
}

int initialization()
{
    printf("> Initializing!\n");
    WSADATA wsaData;
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result)
    {
        printf(">>> Startup failed. Error %d.\n", result);
        return FALSE;
    }
    return TRUE;
}

SOCKET create_socket()
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET)
    {
        printf(">>> Client socket construction failed. Error %d.\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    SADDR saddr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = inet_addr(ADDRESS),
        .sin_port        = htons(PORT)
    };
    
    // Bind the socket if is server
    if (type == SERVER_CHAR)
    {
        result = bind(s, (struct sockaddr *)&saddr, sizeof(saddr));
        if (result GOT_WRONG)
        {
            printf(">>> Bind failed. Error %d\n", WSAGetLastError());
            closesocket(s);
            WSACleanup();
            return INVALID_SOCKET;
        }
    }

    // Connect the socket if is client
    else if (type == CLIENT_CHAR)
    {
        result = connect(s, (struct sockaddr *)&saddr, sizeof(saddr));
        if (result GOT_WRONG || result == INVALID_SOCKET)
        {
            printf(">>> Client connection failed. Error %d\n", WSAGetLastError());
            closesocket(s);
            WSACleanup();
            return INVALID_SOCKET;
        }
    }

    return s;
}
