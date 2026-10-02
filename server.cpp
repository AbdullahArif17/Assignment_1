/*
 * Student Name: Imran Keerio
 * Assignment No: 1
 * Subject: Computer Communication and Networks
 *
 * Description: 
 * This program demonstrates a full-duplex TCP server using Windows Sockets (Winsock2).
 * It operates at the Transport Layer (Layer 4) of the OSI model, using TCP to provide 
 * reliable, ordered, and error-checked delivery of a stream of bytes between applications.
 * Multithreading is used to achieve full-duplex communication, allowing simultaneous 
 * sending and receiving of messages.
 */

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <cstring>
#include <atomic>

// Link with Ws2_32.lib for Winsock API functions
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Atomic flag to gracefully terminate threads when communication ends
atomic<bool> isRunning(true);

/**
 * Executes a terminal command on this machine and captures the standard output.
 * Similar to how a Telnet/SSH server provides remote terminal access.
 */
string executeCommand(const string& cmd) {
    string result = "\n[Remote Terminal Output for: '" + cmd + "']\n";
    FILE* pipe = _popen((cmd + " 2>&1").c_str(), "r");
    if (!pipe) {
        return result + "Failed to run command.\n";
    }
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    _pclose(pipe);
    result += "[End of Command Output]\n";
    return result;
}

/**
 * Thread function to handle receiving messages from the client.
 * Operates continuously until "exit" is received or a connection error occurs.
 * If the message starts with "cmd:", it executes the command and sends the output back.
 */
void receiveMessages(SOCKET clientSocket) {
    char buffer[8192];
    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        // Receive data (Application Layer receiving from Transport Layer)
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            string receivedMsg(buffer);
            // Check if the client wants to terminate the connection
            if (receivedMsg == "exit\n" || receivedMsg == "exit") {
                cout << "\nClient has disconnected." << endl;
                isRunning = false;
                break;
            }

            // If this is remote command output returned from the client:
            if (receivedMsg.rfind("\n[Remote Terminal Output", 0) == 0) {
                cout << receivedMsg << "\nServer: " << flush;
                continue;
            }

            // Check if this is a Remote Terminal command requested by client
            bool isCmd = false;
            string command = "";

            if (receivedMsg.rfind("cmd:", 0) == 0) {
                isCmd = true;
                command = receivedMsg.substr(4);
            } else {
                // Check if message starts with common commands
                string lower = receivedMsg;
                for (char &c : lower) c = tolower(c);
                if (lower == "hostname" || lower.rfind("ipconfig", 0) == 0 ||
                    lower == "dir" || lower.rfind("dir ", 0) == 0 ||
                    lower == "whoami" || lower.rfind("ping ", 0) == 0 ||
                    lower == "date /t" || lower == "time /t" ||
                    lower == "systeminfo" || lower.rfind("echo ", 0) == 0) {
                    isCmd = true;
                    command = receivedMsg;
                }
            }

            if (isCmd) {
                // Trim leading whitespace
                size_t first = command.find_first_not_of(" \t");
                if (first != string::npos) command = command.substr(first);
                
                cout << "\n[Remote Access] Client requested terminal execution: '" << command << "'" << endl;
                string output = executeCommand(command);
                
                // Send the command output back to the client
                send(clientSocket, output.c_str(), (int)output.length(), 0);
                cout << "Server: ";
                cout.flush();
            } else {
                // Standard chat message
                cout << "\nClient: " << receivedMsg << "\nServer: ";
                cout.flush();
            }
        } else if (bytesReceived == 0) {
            cout << "\nConnection closed by client." << endl;
            isRunning = false;
            break;
        } else {
            // Error handling for recv (e.g., connection reset by peer)
            if (isRunning) {
                cout << "\nError receiving data: " << WSAGetLastError() << endl;
                isRunning = false;
            }
            break;
        }
    }
}

/**
 * Launches TightVNC Viewer in a non-blocking background process to connect to the target IP.
 */
void launchVNCViewer(const string& targetIp) {
    string vncPath = "C:\\Program Files\\TightVNC\\tvnviewer.exe";
    DWORD fileAttr = GetFileAttributesA(vncPath.c_str());
    if (fileAttr == INVALID_FILE_ATTRIBUTES) {
        vncPath = "C:\\Program Files (x86)\\TightVNC\\tvnviewer.exe";
    }

    string cmdLine = "\"" + vncPath + "\" " + targetIp;
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        cout << "[System] TightVNC Viewer launched for " << targetIp << endl;
    } else {
        cout << "[System] TightVNC Viewer not found at default path." << endl;
        cout << "[Tip] Please open TightVNC Viewer manually and connect to IP: " << targetIp << endl;
    }
}

/**
 * Thread function to handle sending messages to the client.
 * Reads input from the console and sends it over the socket.
 * Supports:
 * - Regular chat messages (full duplex)
 * - 'desktop' or 'vnc' to view the client's screen
 * - 'exit' to terminate connection
 */
void sendMessages(SOCKET clientSocket, const string& clientIp) {
    string message;
    while (isRunning) {
        // Read input from the console (Application Layer input)
        getline(cin, message);
        
        if (message.empty()) continue;

        // Check if server wants to view the client's desktop
        if (message == "desktop" || message == "vnc") {
            cout << "\n[Remote Access] Launching TightVNC Viewer to access Client's Desktop (" << clientIp << ")..." << endl;
            launchVNCViewer(clientIp);
            string notice = "[Notice] Server has launched a Remote Desktop session to view your screen.";
            send(clientSocket, notice.c_str(), (int)notice.length(), 0);
            if (isRunning) {
                cout << "Server: ";
                cout.flush();
            }
            continue;
        }

        // Send data over the network (Pushing data down to the Transport Layer)
        int bytesSent = send(clientSocket, message.c_str(), (int)message.length(), 0);
        
        if (bytesSent == SOCKET_ERROR) {
            if (isRunning) {
                cout << "Error sending data: " << WSAGetLastError() << endl;
                isRunning = false;
            }
            break;
        }

        // Check if the server wants to terminate the connection
        if (message == "exit") {
            cout << "Closing connection..." << endl;
            isRunning = false;
            break;
        }
        
        if (isRunning) {
            cout << "Server: ";
            cout.flush();
        }
    }
}

int main() {
    cout << "============================================" << endl;
    cout << "  Full Duplex TCP Chat Server"               << endl;
    cout << "  Assignment 1 - Imran Keerio"               << endl;
    cout << "  Remote Access Enabled"                     << endl;
    cout << "============================================" << endl;

    // -----------------------------------------------------------
    // 1. Initialize Winsock
    //    This loads the Windows Sockets DLL so we can use
    //    networking functions.
    // -----------------------------------------------------------
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[Server] WSAStartup failed. Error: " << WSAGetLastError() << endl;
        return 1;
    }
    cout << "[Server] Winsock initialized successfully." << endl;

    // -----------------------------------------------------------
    // 2. Display the server machine's IP addresses so the remote
    //    client knows which IP to connect to.
    //    Uses gethostname() + getaddrinfo() to enumerate all
    //    IPv4 addresses assigned to this machine.
    // -----------------------------------------------------------
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        cout << "[Server] Hostname: " << hostname << endl;

        struct addrinfo hints = {}, *result = nullptr;
        hints.ai_family   = AF_INET;       // IPv4 only
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(hostname, nullptr, &hints, &result) == 0) {
            cout << "[Server] Available IP addresses on this machine:" << endl;
            for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
                char ip[INET_ADDRSTRLEN];
                sockaddr_in* addr = (sockaddr_in*)ptr->ai_addr;
                inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
                cout << "         -> " << ip << endl;
            }
            freeaddrinfo(result);
        }
    }
    cout << endl;
    cout << "[Server] Share one of the above IPs with the client device." << endl;
    cout << "[Server] Make sure port 8080 is allowed through the firewall." << endl;
    cout << "         (Run as Admin: netsh advfirewall firewall add rule" << endl;
    cout << "          name=\"ChatServer\" dir=in action=allow protocol=TCP localport=8080)" << endl;
    cout << endl;

    // -----------------------------------------------------------
    // 3. Create a TCP socket
    //    AF_INET  = IPv4 address family      (OSI Layer 3)
    //    SOCK_STREAM = TCP stream socket      (OSI Layer 4)
    //    IPPROTO_TCP = explicitly use TCP
    // -----------------------------------------------------------
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "[Server] Socket creation failed. Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    cout << "[Server] TCP socket created." << endl;

    // Allow the port to be reused quickly after restart
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    // -----------------------------------------------------------
    // 4. Bind to 0.0.0.0:8080  (INADDR_ANY)
    //    This makes the server accept connections from ANY
    //    network interface — local, LAN, Wi-Fi, etc.
    // -----------------------------------------------------------
    sockaddr_in serverAddr;
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;   // Accept from all interfaces
    serverAddr.sin_port        = htons(8080);   // Port 8080

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "[Server] Bind failed. Error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "[Server] Bound to 0.0.0.0:8080 (all interfaces)." << endl;

    // -----------------------------------------------------------
    // 5. Listen for incoming connections
    // -----------------------------------------------------------
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "[Server] Listen failed. Error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    cout << "[Server] Listening... waiting for a client to connect." << endl;

    // -----------------------------------------------------------
    // 6. Accept a client connection
    //    accept() blocks until a remote or local client connects.
    //    It returns a NEW socket dedicated to this client.
    // -----------------------------------------------------------
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSocket == INVALID_SOCKET) {
        cerr << "[Server] Accept failed. Error: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Print the remote client's IP and port (Network Layer info)
    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);
    cout << "[Server] *** Client connected from " << clientIp << ":" << clientPort << " ***" << endl;
    cout << "[Server] Full-duplex chat & Remote Access active." << endl;
    cout << "[Server] - Type normal messages to chat in terminal" << endl;
    cout << "[Server] - Type 'desktop' to open TightVNC Viewer to view Client's screen" << endl;
    cout << "[Server] - Type 'cmd: <command>' to execute remote terminal commands on Client" << endl;
    cout << "          (e.g., 'cmd: hostname', 'cmd: ipconfig', 'cmd: dir', 'cmd: date /t')" << endl;
    cout << "[Server] - Type 'exit' to quit." << endl;
    cout << endl;
    cout << "Server: ";
    cout.flush();

    // 6 & 7. Spawn two threads for full-duplex communication
    // One thread handles receiving data from the client, the other handles sending data.
    // This allows simultaneous two-way communication.
    thread receiveThread(receiveMessages, clientSocket);
    thread sendThread(sendMessages, clientSocket, string(clientIp));

    // Wait for both threads to finish execution
    receiveThread.join();
    sendThread.join();

    // 8. Clean up and close sockets
    // Close the client socket as communication has ended.
    closesocket(clientSocket);
    // Close the main server listening socket.
    closesocket(serverSocket);
    // Terminate use of the Winsock 2 DLL.
    WSACleanup();
    
    cout << "Server shutdown complete." << endl;
    return 0;
}
