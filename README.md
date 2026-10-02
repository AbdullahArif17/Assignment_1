# Assignment No. 1 — Computer Communication and Networks

**Student Name:** Imran Keerio  
**Course Code:** CSC211  
**Subject:** Computer Communication and Networks  
**Topic:** Socket Programming (Full Duplex Client-Server Communication in C++)

---

## Table of Contents

1. [Objective](#1-objective)
2. [Socket Programming — Full Duplex Communication](#2-socket-programming--full-duplex-communication)
3. [Cable Connection & Transmission Medium](#3-cable-connection--transmission-medium)
4. [OSI Model](#4-osi-model)
5. [TCP/IP Protocol Suite](#5-tcpip-protocol-suite)
6. [How to Compile & Run](#6-how-to-compile--run)
7. [Program Output](#7-program-output)
8. [Conclusion](#8-conclusion)

---

## 1. Objective

To create a C++ program using **Windows Winsock (TCP sockets)** in which a **Client** and
**Server** communicate in **full-duplex** mode — both sides can send and receive messages
**simultaneously** using multithreading.

---

## 2. Socket Programming — Full Duplex Communication

### 2.1 What is a Socket?

A **socket** is one endpoint of a two-way communication link between two programs running
on a network. It is bound to a **port number** so that the TCP layer can identify the
application that data is destined to.

### 2.2 What is Full-Duplex Communication?

| Mode         | Description                                                   |
|--------------|---------------------------------------------------------------|
| **Simplex**      | Data flows in only ONE direction (e.g., keyboard → monitor) |
| **Half-Duplex**  | Data flows in BOTH directions but only ONE at a time (e.g., walkie-talkie) |
| **Full-Duplex**  | Data flows in BOTH directions **simultaneously** (e.g., telephone call) |

> *Reference: Lecture 1, Figure 1.2 — Data flow (simplex, half-duplex, and full-duplex)*

Our program implements **full-duplex** communication by using **two threads** on each side:

- **Thread 1 (Send):** Continuously reads input from the user's console and sends it to
  the remote endpoint.
- **Thread 2 (Receive):** Continuously listens for incoming messages from the remote
  endpoint and displays them on the console.

Both threads run **concurrently**, enabling true simultaneous two-way communication —
just like a real telephone call.

### 2.3 Why TCP?

We use **TCP (Transmission Control Protocol)** because:

- It is **connection-oriented** — a connection is established before data transfer.
- It provides **reliable, ordered delivery** of data.
- It has built-in **error detection and retransmission**.
- It is the protocol used by real-world applications like HTTP, FTP, SMTP, and SSH.

### 2.4 Program Architecture

```
┌────────────────────┐         TCP Connection         ┌────────────────────┐
│      SERVER         │◄════════════════════════════►│       CLIENT        │
│                    │        (Full-Duplex)           │                    │
│  ┌──────────────┐  │                               │  ┌──────────────┐  │
│  │ Send Thread  │──│───── messages to client ──────►│──│ Recv Thread  │  │
│  └──────────────┘  │                               │  └──────────────┘  │
│  ┌──────────────┐  │                               │  ┌──────────────┐  │
│  │ Recv Thread  │◄─│───── messages from client ◄───│──│ Send Thread  │  │
│  └──────────────┘  │                               │  └──────────────┘  │
└────────────────────┘                               └────────────────────┘
```

### 2.6 Remote Terminal Access (Telnet-style Execution)

In addition to full-duplex text communication, the program implements **Remote Terminal Access**:
- When the **Client** inputs a message starting with `cmd: <command>` or a common command like `hostname`, `ipconfig`, `dir`, `whoami`:
  1. The client sends the string across the TCP socket (Transport Layer).
  2. The **Server** intercepts the command and executes it locally using a pipe process (`_popen`).
  3. The server captures the terminal output and transmits it back across the TCP socket.
  4. The client prints the remote terminal output directly into its console.

### 2.7 Bidirectional Desktop Access (TightVNC Integration)

Both sides can request a real-time graphical desktop session while simultaneously continuing to chat in the terminal:
- **Client asks for Server's desktop:**
  - Client types `desktop` in the terminal.
  - Client automatically launches **TightVNC Viewer** targeting the Server's IP address (`192.168.0.119`).
  - Server receives an alert in its terminal notifying it of the incoming desktop session.
- **Server asks for Client's desktop (Vice Versa):**
  - Server types `desktop` in the terminal.
  - Server automatically launches **TightVNC Viewer** targeting the Client's IP address.
  - Client receives an alert in its terminal notifying it of the incoming desktop session.
- **Simultaneous Terminal Messaging:**
  - The TCP socket threads run concurrently (`std::thread`), so text messaging in the terminal is completely unaffected and continues in true full duplex!

### 2.8 Winsock API Functions Used

| Function          | Purpose                                            |
|-------------------|----------------------------------------------------|
| `WSAStartup()`    | Initialize Winsock library                         |
| `socket()`        | Create a TCP socket                                |
| `bind()`          | Bind socket to IP address and port                 |
| `listen()`        | Put server socket in listening state               |
| `accept()`        | Accept incoming client connection                  |
| `connect()`       | Client connects to server                          |
| `send()`          | Send data over the socket                          |
| `recv()`          | Receive data from the socket                       |
| `closesocket()`   | Close the socket                                   |
| `WSACleanup()`    | Clean up Winsock resources                         |

---

## 3. Cable Connection & Transmission Medium

### 3.1 Guided vs Unguided Media

> *Reference: Lecture 1 — "Medium: The physical path by which a message travels from
> sender to receiver — twisted pair, coaxial cable, fiber-optic, radio waves"*

| Property           | Guided Media                             | Unguided Media                   |
|--------------------|------------------------------------------|----------------------------------|
| **Definition**     | Signals travel through a physical cable   | Signals travel through free space |
| **Examples**       | Twisted pair, Coaxial cable, Fiber optic  | Radio waves, Microwave, Infrared |
| **Also called**    | Wired / Bounded media                    | Wireless / Unbounded media       |
| **Signal direction** | Directed along the cable               | Broadcast in all directions      |
| **Interference**   | Lower (shielded)                         | Higher (open air)                |
| **Security**       | More secure (physical access required)   | Less secure (can be intercepted) |
| **Cost**           | Higher installation cost                 | Lower installation cost          |

### 3.2 Common Guided Media (Cable Connections)

#### a) Twisted Pair Cable (UTP / STP)

- **Structure:** Two insulated copper wires twisted together
- **Types:** Unshielded (UTP) and Shielded (STP)
- **Categories:** Cat5e (100 Mbps), Cat6 (1 Gbps), Cat6a (10 Gbps)
- **Usage:** LAN connections, Ethernet, telephone lines
- **Max Distance:** ~100 meters
- **Connector:** RJ-45

#### b) Coaxial Cable

- **Structure:** Central copper conductor surrounded by insulation, metallic shield, and
  outer jacket
- **Bandwidth:** Up to 500 Mbps
- **Usage:** Cable TV, older Ethernet (10Base2, 10Base5), broadband internet
- **Max Distance:** Up to 500 meters
- **Connector:** BNC, F-type

#### c) Fiber Optic Cable

- **Structure:** Glass or plastic core that carries light signals
- **Types:** Single-mode (long distance) and Multi-mode (shorter distance)
- **Bandwidth:** Up to 100+ Gbps
- **Usage:** Backbone networks, undersea cables, high-speed internet (FTTH)
- **Max Distance:** 2 km (multi-mode) to 100+ km (single-mode)
- **Connector:** SC, LC, ST

### 3.3 Medium Used in Our Program

Our socket program runs on **localhost (127.0.0.1)** — the loopback interface. However,
if deployed on two separate machines on a LAN, the data would travel through:

- **Guided Medium: Cat5e/Cat6 Twisted Pair (UTP) Ethernet Cable**
- Connected via **RJ-45 connectors** to a **Switch or Router**
- Operating at the **Physical Layer** of the OSI model
- This is a **point-to-point, star topology** connection
- The cable carries **digital signals** (baseband transmission)

---

## 4. OSI Model

> *Reference: Lecture 2 — "The OSI Model was introduced by ISO in the late 1970s"*

The **Open Systems Interconnection (OSI)** model is a conceptual framework with
**7 layers** that describes how data moves from one application to another across a network.

### 4.1 The Seven Layers

| Layer | Name              | Function                                          | PDU      | Protocols/Devices       |
|-------|-------------------|---------------------------------------------------|----------|-------------------------|
| 7     | **Application**   | Provides services to the user                    | Data     | HTTP, FTP, SMTP, DNS    |
| 6     | **Presentation**  | Translation, compression, encryption             | Data     | SSL/TLS, JPEG, ASCII    |
| 5     | **Session**       | Dialog control and synchronization               | Data     | NetBIOS, PPTP           |
| 4     | **Transport**     | Process-to-process delivery, segmentation        | Segment  | TCP, UDP                |
| 3     | **Network**       | Source-to-destination delivery, routing           | Packet   | IP, ICMP, Routers       |
| 2     | **Data Link**     | Hop-to-hop delivery, framing, MAC addressing     | Frame    | Ethernet, Switches      |
| 1     | **Physical**      | Bit transmission over physical medium            | Bits     | Cables, Hubs, NIC       |

### 4.2 How Our Program Maps to the OSI Layers

```
┌───────────────────────────────────────────────────────────────────────┐
│  Layer 7 — Application                                                │
│  Our C++ program (server.cpp / client.cpp)                           │
│  User types a message → passed to socket API                        │
├───────────────────────────────────────────────────────────────────────┤
│  Layer 6 — Presentation                                               │
│  Data encoding (ASCII text characters → byte stream)                 │
├───────────────────────────────────────────────────────────────────────┤
│  Layer 5 — Session                                                    │
│  TCP connection establishment (3-way handshake) manages the session  │
├───────────────────────────────────────────────────────────────────────┤
│  Layer 4 — Transport                                                  │
│  TCP segments data, assigns port numbers (8080),                     │
│  provides reliable delivery, flow control, error control             │
├───────────────────────────────────────────────────────────────────────┤
│  Layer 3 — Network                                                    │
│  IP addresses packets (source: 127.0.0.1, dest: 127.0.0.1)          │
│  Handles routing (via loopback or through routers on a real network) │
├───────────────────────────────────────────────────────────────────────┤
│  Layer 2 — Data Link                                                  │
│  Frames data with MAC addresses, error detection (CRC)              │
│  Ethernet protocol on a LAN                                         │
├───────────────────────────────────────────────────────────────────────┤
│  Layer 1 — Physical                                                   │
│  Transmits raw bits over the cable (UTP Cat6 / Fiber Optic)         │
│  Defines voltage levels, data rate, connectors (RJ-45)              │
└───────────────────────────────────────────────────────────────────────┘
```

### 4.3 Key OSI Concepts from Lectures

- **Encapsulation:** As data moves DOWN through layers, each layer adds its own header
  (and sometimes trailer). At the physical layer, it is transmitted as raw bits.
- **Decapsulation:** At the receiving end, each layer removes its header and passes the
  data UP to the next layer.
- **Peer-to-Peer Communication:** Each layer on the sender communicates logically with
  the same layer on the receiver.

---

## 5. TCP/IP Protocol Suite

> *Reference: Lecture 2, Section 2-4 — "The TCP/IP protocol suite is made of five layers:
> physical, data link, network, transport, and application."*

### 5.1 TCP/IP vs OSI Model

```
        OSI Model                    TCP/IP Model
   ┌─────────────────┐          ┌─────────────────┐
   │   Application   │          │                 │
   ├─────────────────┤          │   Application   │
   │  Presentation   │          │  (HTTP, FTP,    │
   ├─────────────────┤          │   SMTP, DNS)    │
   │    Session      │          │                 │
   ├─────────────────┤          ├─────────────────┤
   │   Transport     │          │   Transport     │
   │   (TCP/UDP)     │          │   (TCP/UDP)     │
   ├─────────────────┤          ├─────────────────┤
   │    Network      │          │    Internet     │
   │     (IP)        │          │     (IP)        │
   ├─────────────────┤          ├─────────────────┤
   │   Data Link     │          │                 │
   ├─────────────────┤          │ Network Access  │
   │   Physical      │          │ (Host-to-Net)   │
   └─────────────────┘          └─────────────────┘
      7 Layers                     4/5 Layers
```

### 5.2 TCP/IP Addressing Used in Our Program

> *Reference: Lecture 2, Section 2-5 — "Four levels of addresses: physical, logical, port,
> and specific."*

| Address Type     | Value in Our Program           | OSI Layer  |
|------------------|--------------------------------|------------|
| **Physical (MAC)** | NIC's MAC address (e.g., `00:1A:2B:3C:4D:5E`) | Layer 2 |
| **Logical (IP)**   | `127.0.0.1` (localhost)                        | Layer 3 |
| **Port**           | `8080` (server), ephemeral port (client)       | Layer 4 |
| **Specific (URL)** | Not used (no HTTP in this program)             | Layer 7 |

### 5.3 TCP Three-Way Handshake

Before our client and server can exchange messages, TCP establishes a connection using
a **three-way handshake**:

```
   Client                          Server
     │                               │
     │────── SYN (seq=x) ──────────►│   1. Client sends SYN
     │                               │
     │◄───── SYN-ACK (seq=y, ack=x+1)│   2. Server replies SYN-ACK
     │                               │
     │────── ACK (ack=y+1) ────────►│   3. Client sends ACK
     │                               │
     │     CONNECTION ESTABLISHED     │
     │◄═══════ FULL DUPLEX ═════════►│   Data flows both ways
```

This maps to our code:
- **Server:** `listen()` → `accept()` (waits for SYN, sends SYN-ACK, receives ACK)
- **Client:** `connect()` (sends SYN, receives SYN-ACK, sends ACK)

### 5.4 TCP Connection Termination (Four-Way Handshake)

When either side types "exit":

```
   Initiator                       Receiver
     │────── FIN ──────────────────►│   1. "I'm done sending"
     │◄───── ACK ──────────────────│   2. "OK, acknowledged"
     │◄───── FIN ──────────────────│   3. "I'm done too"
     │────── ACK ──────────────────►│   4. "OK, connection closed"
```

This maps to our code: `closesocket()` triggers the FIN sequence.

---

## 6. How to Compile & Run (Two Different Devices)

### Prerequisites

- **Windows OS** on both devices (Winsock2 is Windows-specific)
- **g++ (MinGW)** or **MSVC (Visual Studio)** compiler on both devices
- Both devices must be on the **same network** (same Wi-Fi or same LAN switch)

### Step 1 — Find the Server's IP Address

On the **server device**, open PowerShell and run:
```powershell
ipconfig
```
Look for `IPv4 Address` under your active adapter (Wi-Fi or Ethernet). Example:
```
Wireless LAN adapter Wi-Fi:
   IPv4 Address. . . . . . . . . : 192.168.1.105
```
**Note this IP** — you'll enter it on the client device.

### Step 2 — Allow Port 8080 Through Windows Firewall (Server Device)

Run this command **as Administrator** on the server device:
```powershell
netsh advfirewall firewall add rule name="ChatServer" dir=in action=allow protocol=TCP localport=8080
```

### Step 3 — Compile on Both Devices

**With g++ (MinGW):**
```bash
g++ -o server.exe server.cpp -lws2_32
g++ -o client.exe client.cpp -lws2_32
```

**With MSVC (Visual Studio Developer Prompt):**
```bash
cl /EHsc server.cpp ws2_32.lib
cl /EHsc client.cpp ws2_32.lib
```

### Step 4 — Run the Server (Device 1)

```powershell
cd "d:\3rd Sem\Computer Networks\Assignment_1"
.\server.exe
```

Output:
```
============================================
  Full Duplex TCP Chat Server
  Assignment 1 - Imran Keerio
  Remote Access Enabled
============================================
[Server] Winsock initialized successfully.
[Server] Hostname: DESKTOP-I1BA26R
[Server] Available IP addresses on this machine:
         -> 192.168.0.119

[Server] Share one of the above IPs with the client device.
[Server] Bound to 0.0.0.0:8080 (all interfaces).
[Server] Listening... waiting for a client to connect.
```

### Step 5 — Run the Client (Device 2)

Copy `client.exe` to the second device, then run:
```powershell
.\client.exe
```

When prompted, enter the **server's IP address**:
```
============================================
  Full Duplex TCP Chat Client
  Assignment 1 - Imran Keerio
  Remote Access Enabled
============================================
[Client] Winsock initialized.

[Client] Enter server IP address
         (127.0.0.1 for same PC, or server's LAN IP): 192.168.0.119
[Client] Connecting to 192.168.0.119:8080 ...
[Client] *** Connected to server at 192.168.0.119:8080 ***
[Client] Full-duplex chat & Remote Access active.
[Client] - Type messages to chat with server in terminal
[Client] - Type 'desktop' to open TightVNC Viewer to view Server's screen
[Client] - Type 'cmd: <command>' to execute remote terminal commands on server
          (e.g., 'cmd: hostname', 'cmd: ipconfig', 'cmd: dir', 'cmd: date /t')
[Client] - Type 'exit' to quit.
```

### Supported Commands in Terminal

| Command | Typed on | Result |
|---|---|---|
| *Any text* | Either side | Instant full-duplex messaging between both terminals |
| `desktop` | Client | Launches TightVNC Viewer to view and control the Server's screen |
| `desktop` | Server | Launches TightVNC Viewer to view and control the Client's screen *(Vice Versa)* |
| `cmd: <command>` | Either side | **Bidirectional Remote Terminal:** Executes command on the remote machine (Client ➔ Server OR Server ➔ Client) and returns terminal output |
| `exit` | Either side | Closes the connection cleanly on both machines |

> 📘 **For the full exhaustive guide on setting up on two physical devices, see:**  
> [**`TWO_DEVICE_CONNECTION_GUIDE.md`**](file:///d:/3rd%20Sem/Computer%20Networks/Assignment_1/TWO_DEVICE_CONNECTION_GUIDE.md)

### Connection Scenarios

| Scenario                        | Server Runs On | Client Enters      |
|---------------------------------|----------------|---------------------|
| Same computer (testing)         | Device A       | `127.0.0.1`        |
| Two PCs on same Wi-Fi           | Device A       | `192.168.x.x`      |
| Two PCs on same LAN switch      | Device A       | `192.168.x.x` or `10.x.x.x` |

### Troubleshooting

| Problem                        | Solution                                           |
|--------------------------------|----------------------------------------------------|
| "Connection failed"            | Check that server is running first                 |
| "Connection timed out"         | Both devices must be on the same LAN/Wi-Fi         |
| Firewall blocking              | Run the `netsh` firewall command as Admin           |
| Wrong IP address               | Run `ipconfig` on server device to find correct IP |

---

## 7. Program Output

### Expected Output

**Server Terminal:**
```
============================================
  Full Duplex TCP Chat Server
  Assignment 1 - Imran Keerio
============================================
[Server] Winsock initialized successfully.
[Server] Socket created.
[Server] Bound to port 8080.
[Server] Listening for connections...
[Server] Client connected!
[Server] Full-duplex chat started. Type messages below.
[Server] Type 'exit' to quit.

Server: Hello Client, how are you?
[Client]: I am fine, thank you!
Server: Great, the full duplex is working!
```

**Client Terminal:**
```
============================================
  Full Duplex TCP Chat Client
  Assignment 1 - Imran Keerio
============================================
[Client] Winsock initialized successfully.
[Client] Socket created.
[Client] Connected to server at 127.0.0.1:8080
[Client] Full-duplex chat started. Type messages below.
[Client] Type 'exit' to quit.

[Server]: Hello Client, how are you?
Client: I am fine, thank you!
[Server]: Great, the full duplex is working!
```

---

## 8. Conclusion

In this assignment, we implemented a **full-duplex client-server chat application** in
**C++** using **Windows Winsock TCP sockets** and **multithreading**. The key takeaways are:

1. **Socket Programming:** We used the Winsock API to create TCP sockets, establish
   connections, and exchange data between two endpoints.

2. **Full-Duplex Communication:** By using **two threads** (one for sending, one for
   receiving) on each side, both the client and server can send and receive messages
   **simultaneously** — achieving true full-duplex communication.

3. **Guided Transmission Medium:** On a physical LAN, data would travel through
   **Cat5e/Cat6 UTP Ethernet cables** (guided medium) connected in a **star topology**.

4. **OSI Model Mapping:** Our program touches all 7 layers of the OSI model — from
   the Application layer (our C++ code) down to the Physical layer (cable transmission).

5. **TCP/IP Usage:** We used TCP at the Transport layer for reliable, connection-oriented
   communication, and IP at the Network layer for logical addressing (127.0.0.1:8080).

---

### References

- Data Communications & Networking, 5th Edition — Behrouz A. Forouzan
- Lecture 1: Data Communication Concepts, Network Topologies
- Lecture 2: Network Models (OSI and TCP/IP)
- Microsoft Winsock Documentation: https://learn.microsoft.com/en-us/windows/win32/winsock/
