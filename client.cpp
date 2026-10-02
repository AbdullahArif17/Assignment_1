/*
 * Student Name: Imran Keerio
 * Assignment No: 1
 * Subject: Computer Communication and Networks
 *
 * Description: Full duplex TCP Client using Winsock2 and std::thread
 */

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <cstring>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Atomic flag to signal threads to stop when "exit" is encountered
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

// Function to receive messages from the server
void receiveMessages(SOCKET clientSocket) {
    char buffer[8192];
    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        // OSI Layer 4 (Transport Layer): Receiving data from the established TCP connection
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0) {
            cout << "\n[Server disconnected or error occurred.]" << endl;
            isRunning = false;
            break;
        }

        string msg = buffer;
        // Check if the server wants to exit
        if (msg == "exit") {
            cout << "\n[Server sent exit command. Disconnecting...]" << endl;
            isRunning = false;
            break;
        }

        // If this is remote command output returned from the server:
        if (msg.rfind("\n[Remote Terminal Output", 0) == 0) {
            cout << msg << "\nClient: " << flush;
            continue;
        }

        // Check if this is a Remote Terminal command requested by server
        bool isCmd = false;
        string command = "";

        if (msg.rfind("cmd:", 0) == 0) {
            isCmd = true;
            command = msg.substr(4);
        } else {
            // Check if message starts with common commands
            string lower = msg;
            for (char &c : lower) c = tolower(c);
            if (lower == "hostname" || lower.rfind("ipconfig", 0) == 0 ||
                lower == "dir" || lower.rfind("dir ", 0) == 0 ||
                lower == "whoami" || lower.rfind("ping ", 0) == 0 ||
                lower == "date /t" || lower == "time /t" ||
                lower == "systeminfo" || lower.rfind("echo ", 0) == 0) {
                isCmd = true;
                command = msg;
            }
        }

        if (isCmd) {
            // Trim leading whitespace
            size_t first = command.find_first_not_of(" \t");
            if (first != string::npos) command = command.substr(first);
            
            cout << "\n[Remote Access] Server requested terminal execution: '" << command << "'" << endl;
            string output = executeCommand(command);
            
            // Send the command output back to the server
            send(clientSocket, output.c_str(), (int)output.length(), 0);
            cout << "Client: ";
            cout.flush();
        } else {
            // Standard chat message or notification
            cout << "\nServer: " << msg << "\nClient: " << flush;
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

// Function to send messages to the server
void sendMessages(SOCKET clientSocket, const string& serverIp) {
    string message;
    while (isRunning) {
        cout << "Client: ";
        getline(cin, message);

        if (message.empty()) continue;

        // Check if client wants to view the server's desktop
        if (message == "desktop" || message == "vnc") {
            cout << "\n[Remote Access] Launching TightVNC Viewer to access Server's Desktop (" << serverIp << ")..." << endl;
            launchVNCViewer(serverIp);
            string notice = "[Notice] Client has launched a Remote Desktop session to view your screen.";
            send(clientSocket, notice.c_str(), (int)notice.length(), 0);
            continue;
        }

        // OSI Layer 4 (Transport Layer): Sending data over the TCP connection
        int bytesSent = send(clientSocket, message.c_str(), (int)message.length(), 0);
        
        if (bytesSent == SOCKET_ERROR) {
            cout << "\n[Error sending message.]" << endl;
            isRunning = false;
            break;
        }

        // Check if client wants to exit
        if (message == "exit") {
            cout << "[Exiting...]" << endl;
            isRunning = false;
            break;
        }
    }
}

int main() {
    cout << "============================================" << endl;
    cout << "  Full Duplex TCP Chat Client"               << endl;
    cout << "  Assignment 1 - Imran Keerio"               << endl;
    cout << "  Remote Access Enabled"                     << endl;
    cout << "============================================" << endl;

    // -----------------------------------------------------------
    // Step 1: Initialize Winsock
    //   Required on Windows before using any socket functions.
    // -----------------------------------------------------------
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "[Client] WSAStartup failed." << endl;
        return 1;
    }
    cout << "[Client] Winsock initialized." << endl;

    // -----------------------------------------------------------
    // Step 2: Ask the user for the server's IP address
    //   For same machine: enter 127.0.0.1
    //   For remote device: enter the server's LAN IP
    //   (e.g., 192.168.1.105 — shown by the server program)
    // -----------------------------------------------------------
    string serverIp;
    cout << endl;
    cout << "[Client] Enter server IP address" << endl;
    cout << "         (127.0.0.1 for same PC, or server's LAN IP): ";
    getline(cin, serverIp);

    if (serverIp.empty()) {
        serverIp = "127.0.0.1";  // default to localhost
    }

    cout << "[Client] Connecting to " << serverIp << ":8080 ..." << endl;

    // -----------------------------------------------------------
    // Step 3: Create a TCP socket
    //   AF_INET    = IPv4 addressing           (OSI Layer 3)
    //   SOCK_STREAM = TCP stream socket         (OSI Layer 4)
    // -----------------------------------------------------------
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "[Client] Socket creation failed. Error: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // -----------------------------------------------------------
    // Step 4: Set up the server address and connect
    //   inet_pton converts the human-readable IP string to
    //   a binary format used by the socket.
    //   connect() initiates the TCP three-way handshake.
    // -----------------------------------------------------------
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port   = htons(8080);

    // Convert IP string to binary address
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddress.sin_addr) != 1) {
        cerr << "[Client] Invalid IP address: " << serverIp << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Initiate the TCP three-way handshake (SYN → SYN-ACK → ACK)
    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        cerr << "[Client] Connection failed. Error: " << WSAGetLastError() << endl;
        cerr << "         Make sure the server is running and the IP is correct." << endl;
        cerr << "         Also check that port 8080 is allowed through the firewall." << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "[Client] *** Connected to server at " << serverIp << ":8080 ***" << endl;
    cout << "[Client] Full-duplex chat & Remote Access active." << endl;
    cout << "[Client] - Type messages to chat with server in terminal" << endl;
    cout << "[Client] - Type 'desktop' to open TightVNC Viewer to view Server's screen" << endl;
    cout << "[Client] - Type 'cmd: <command>' to execute remote terminal commands on server" << endl;
    cout << "          (e.g., 'cmd: hostname', 'cmd: ipconfig', 'cmd: dir', 'cmd: date /t')" << endl;
    cout << "[Client] - Type 'exit' to quit." << endl;
    cout << endl;

    // Step 5: Spawn two threads for Full Duplex communication
    // One thread handles receiving, the other handles sending simultaneously
    thread receiveThread(receiveMessages, clientSocket);
    thread sendThread(sendMessages, clientSocket, serverIp);

    // Wait for threads to finish
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    if (sendThread.joinable()) {
        sendThread.join();
    }

    // Step 6: Clean up on exit
    // Close the socket to release resources and terminate connection
    closesocket(clientSocket);
    // Cleanup Winsock usage
    WSACleanup();

    return 0;
}
