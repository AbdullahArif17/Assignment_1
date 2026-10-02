# Complete Step-by-Step Guide: Connecting Two Devices Over a Network
## Full-Duplex Terminal Chat + Bidirectional Desktop Access (TightVNC)

**Course:** CSC211 — Computer Communication & Networks  
**Assignment:** Assignment No. 1 — Socket Programming & Remote Access  
**Student:** Imran Keerio  

---

## 📋 Table of Contents
1. [Network Architecture & Prerequisites](#1-network-architecture--prerequisites)
2. [How to Test Everything on THIS PC Right Now](#2-how-to-test-everything-on-this-pc-right-now)
3. [Full Setup on Two Different Devices (Step-by-Step)](#3-full-setup-on-two-different-devices-step-by-step)
   - [Part A: Finding the IP Addresses](#part-a-finding-the-ip-addresses)
   - [Part B: Windows Firewall Rules (Ports 8080 & 5900)](#part-b-windows-firewall-rules-ports-8080--5900)
   - [Part C: Installing & Configuring TightVNC on Both PCs](#part-c-installing--configuring-tightvnc-on-both-pcs)
   - [Part D: Copying Client Files to Device 2](#part-d-copying-client-files-to-device-2)
4. [Running the System (Two Devices)](#4-running-the-system-two-devices)
5. [Feature Guide & Commands](#5-feature-guide--commands)
   - [Feature 1: True Full-Duplex Chat in Terminal](#feature-1-true-full-duplex-chat-in-terminal)
   - [Feature 2: Bidirectional Desktop Access (Vice Versa)](#feature-2-bidirectional-desktop-access-vice-versa)
   - [Feature 3: Remote Terminal Execution (Telnet Mode)](#feature-3-remote-terminal-execution-telnet-mode)
6. [Troubleshooting Guide](#6-troubleshooting-guide)
7. [Theoretical Mapping to Course Lectures (For Viva / Exam)](#7-theoretical-mapping-to-course-lectures-for-viva--exam)

---

## 1. Network Architecture & Prerequisites

### Architecture Diagram

```
                 ┌────────────────────────────────────────────────────────┐
                 │                   Wi-Fi Router / Switch                │
                 │                 Local Area Network (LAN)               │
                 └───────────────┬────────────────────────┬───────────────┘
                                 │                        │
                                 ▼                        ▼
                       ┌───────────────────┐    ┌───────────────────┐
                       │     DEVICE 1      │    │     DEVICE 2      │
                       │     (SERVER)      │    │     (CLIENT)      │
                       │                   │    │                   │
                       │ IP: 192.168.0.119 │    │ IP: 192.168.0.xxx │
                       │ Port 8080 (Chat)  │    │ Port 8080 (Chat)  │
                       │ Port 5900 (VNC)   │    │ Port 5900 (VNC)   │
                       └─────────┬─────────┘    └─────────┬─────────┘
                                 │                        │
       Full-Duplex Terminal Chat │◄══════════════════════►│ Full-Duplex Terminal Chat
                                 │      TCP Port 8080     │
                                 │                        │
      TightVNC Viewer (Dev 1) ───┼───────────────────────►│ TightVNC Server (Dev 2)
       [Access Client Desktop]   │      TCP Port 5900     │
                                 │                        │
      TightVNC Server (Dev 1) ◄──┼────────────────────────┼─── TightVNC Viewer (Dev 2)
                                 │      TCP Port 5900     │    [Access Server Desktop]
```

### System Capabilities:
1. **Full-Duplex Terminal Messaging:** Both sides can type and receive messages simultaneously without waiting for turns.
2. **Bidirectional Desktop Access ("Vice Versa"):**
   - **Client ➔ Server:** Client types `desktop` in terminal to view and control Server's desktop screen.
   - **Server ➔ Client:** Server types `desktop` in terminal to view and control Client's desktop screen.
3. **Remote Terminal Commands:** Client can run commands (`hostname`, `ipconfig`, `dir`, `whoami`) directly on the Server.

---

## 2. How to Test Everything on THIS PC Right Now

Before connecting two devices, you can verify all features on your current machine in **60 seconds**:

### Step 1: Open Window 1 (Server)
Open PowerShell and run:
```powershell
cd "d:\3rd Sem\Computer Networks\Assignment_1"
.\server.exe
```
*(Server prints local IP addresses and starts listening on port 8080).*

### Step 2: Open Window 2 (Client)
Open a **second** PowerShell window side-by-side and run:
```powershell
cd "d:\3rd Sem\Computer Networks\Assignment_1"
.\client.exe
```
* When prompted: `Enter server IP address (127.0.0.1 for same PC, or server's LAN IP):`  
  Simply press **`Enter`** (it defaults to `127.0.0.1`).

### Step 3: Test Full-Duplex Chat
* Type in the Server terminal: `Hello Client!` ➔ Appears on Client screen.
* Type in the Client terminal: `Hello Server!` ➔ Appears on Server screen.
* Both can type at the exact same moment!

### Step 4: Test Bidirectional Desktop Access (TightVNC)
* In the **Client window**, type:
  ```text
  desktop
  ```
* TightVNC Viewer will immediately pop up on your screen targeting `127.0.0.1`!
* Enter your TightVNC password (e.g. `123456`) and click **OK**.
* You have live graphical desktop access right on your screen, while the terminal chat continues in full duplex!

### Step 5: Test Remote Terminal Commands
In the Client window, type:
```text
cmd: hostname
```
or:
```text
cmd: ipconfig
```
The server executes the command in the background and sends the terminal output back to the client!

---

## 3. Full Setup on Two Different Devices (Step-by-Step)

Follow these steps to deploy on two physical PCs / laptops on your Wi-Fi or LAN.

### Part A: Finding the IP Addresses

1. **On Device 1 (Server):**
   - Run `.\server.exe`. It automatically displays your real LAN IP under `Available IP addresses on this machine:`
   - Your primary Wi-Fi/LAN IP is: **`192.168.0.119`**
   - *(Alternatively, open PowerShell and type `ipconfig`)*.

2. **On Device 2 (Client):**
   - Open PowerShell and type:
     ```cmd
     ipconfig
     ```
   - Look for `IPv4 Address` under your active Wi-Fi or Ethernet adapter (e.g., `192.168.0.150`).
   - Note this IP down (Device 1 will use it if Device 1 wants to view Device 2's desktop).

---

### Part B: Windows Firewall Rules (Ports 8080 & 5900)

By default, Windows Firewall blocks incoming connections from other computers. You must allow:
- **Port 8080 (TCP):** For our C++ Full-Duplex Socket Chat
- **Port 5900 (TCP):** For TightVNC Remote Desktop

#### On Device 1 (Server PC):
Open **PowerShell as Administrator** and paste:
```powershell
netsh advfirewall firewall add rule name="ChatServer" dir=in action=allow protocol=TCP localport=8080
netsh advfirewall firewall add rule name="TightVNC" dir=in action=allow protocol=TCP localport=5900
```
It will display `Ok.` for both.

#### On Device 2 (Client PC):
To allow Device 1 to also view Device 2's desktop (the **"vice versa"** access):
Open **PowerShell as Administrator** on Device 2 and paste:
```powershell
netsh advfirewall firewall add rule name="TightVNC" dir=in action=allow protocol=TCP localport=5900
```

---

### Part C: Installing & Configuring TightVNC on Both PCs

To enable bidirectional screen sharing, both devices should have TightVNC installed:

1. Download **[TightVNC for Windows](https://www.tightvnc.com/download.php)** on both PCs.
2. Run the installer and choose **Typical** or **Complete**.
3. When prompted to set passwords:
   * **Primary password:** Enter a simple password (e.g., `123456`). This is what the remote device will type to connect.
   * **Administrative password:** You can leave it blank or set the same.
4. Finish installation.
5. Confirm that the **TightVNC Server icon (`V`)** is visible in your Windows system tray (bottom-right near the clock) on both devices.

---

### Part D: Copying Client Files to Device 2

On Device 2, you only need `client.exe`:
1. Copy [`client.exe`](file:///d:/3rd Sem/Computer Networks/Assignment_1/client.exe) from Device 1 to Device 2 using a USB drive, Google Drive, WhatsApp Web, or local network share.
2. Place it in any folder on Device 2 (e.g. `C:\Users\YourName\Desktop\Assignment_1\client.exe`).

*(Optional: If copying the C++ source file `client.cpp`, compile it on Device 2 using: `g++ -o client.exe client.cpp -lws2_32` or via Dev-C++ with `-lws2_32` linker flag).*

---

## 4. Running the System (Two Devices)

### Step 1: Start Server on Device 1
Open PowerShell on Device 1:
```powershell
cd "d:\3rd Sem\Computer Networks\Assignment_1"
.\server.exe
```
Expected output:
```text
============================================
  Full Duplex TCP Chat Server
  Assignment 1 - Imran Keerio
  Remote Access Enabled
============================================
[Server] Winsock initialized successfully.
[Server] Available IP addresses on this machine:
         -> 192.168.0.119
[Server] Bound to 0.0.0.0:8080 (all interfaces).
[Server] Listening... waiting for a client to connect.
```

### Step 2: Start Client on Device 2
Open PowerShell where `client.exe` is located on Device 2:
```powershell
.\client.exe
```

When prompted:
```text
[Client] Enter server IP address
         (127.0.0.1 for same PC, or server's LAN IP): 
```
Type Device 1's IP:
```text
192.168.0.119
```
and press **Enter**.

### Step 3: Success!
Both terminals now display:
* `*** Connected ***`
* `Full-duplex chat & Remote Access active.`

---

## 5. Feature Guide & Commands

### Feature 1: True Full-Duplex Chat in Terminal
- **How to use:** Just type any message and press Enter in either window.
- **Why it is Full-Duplex:** The program uses two threads (`receiveThread` and `sendThread`). Device 1 and Device 2 can type at the exact same moment without locking each other out.

### Feature 2: Bidirectional Desktop Access (Vice Versa)

#### Case A: Client accesses Server's Desktop
1. On **Device 2 (Client)**, type:
   ```text
   desktop
   ```
2. The program automatically launches **TightVNC Viewer** targeting `192.168.0.119`.
3. Device 2 enters the password (`123456`).
4. **Device 2 now has full graphical control over Device 1's desktop!**
5. Device 1 receives an alert in its terminal:
   `[Notice] Client has launched a Remote Desktop session to view your screen.`

#### Case B: Server accesses Client's Desktop (Vice Versa)
1. On **Device 1 (Server)**, type:
   ```text
   desktop
   ```
2. The program automatically launches **TightVNC Viewer** targeting Device 2's IP.
3. Device 1 enters the password.
4. **Device 1 now has full graphical control over Device 2's desktop!**
5. Device 2 receives an alert in its terminal:
   `[Notice] Server has launched a Remote Desktop session to view your screen.`

#### Live Chat Continues!
While viewing the desktop, you can switch back to the PowerShell terminal and keep chatting back and forth!

### Feature 3: Bidirectional Remote Terminal Execution (Both Ways!)
You can execute terminal commands across the network in **both directions**:

#### Direction 1: Client executes commands on Server
Type in the **Client** terminal:
```text
cmd: hostname
cmd: ipconfig
cmd: dir
cmd: whoami
```
The **Server** executes the command on its machine and streams the output back to the **Client**.

#### Direction 2: Server executes commands on Client (Vice Versa!)
Type in the **Server** terminal:
```text
cmd: hostname
cmd: ipconfig
cmd: dir
cmd: whoami
```
The **Client** executes the command on its machine and streams the output back to the **Server**.

*(Note: Common commands like `hostname`, `ipconfig`, `dir`, and `whoami` are also auto-detected even if you forget to type the `cmd:` prefix!)*

---

## 6. Troubleshooting Guide

| Issue / Error | Cause | Fix |
|---|---|---|
| **`Connection failed. Error: 10061`** | Server program is not running yet. | Start `.\server.exe` on Device 1 **first**, then start `.\client.exe` on Device 2. |
| **`Connection failed. Error: 10060`** | Windows Firewall is blocking Port 8080. | Run the `netsh advfirewall...` command as Administrator on Device 1. Confirm both PCs are on the same Wi-Fi. |
| **TightVNC says "Failed to connect to server"** | TightVNC Server is not running or Port 5900 is blocked. | 1. Ensure the `V` icon is running in the taskbar.<br>2. Run the Port 5900 firewall rule command as Administrator on target PC. |
| **TightVNC says "Authentication failed"** | Incorrect VNC password. | Open TightVNC Service Configuration from Start Menu and reset the Primary Password to `123456`. |
| **`Bind failed. Error: 10048`** | Port 8080 is already in use by a previous instance. | Close existing PowerShell windows running `server.exe` or kill `server.exe` via Task Manager. |

---

## 7. Theoretical Mapping to Course Lectures (For Viva / Exam)

### 1. Transmission Media (Lecture 1)
- **Guided Media:** If connecting via an **Ethernet Cable (Cat5e / Cat6 UTP)** with **RJ-45 connectors**, bits travel as guided electrical signals through twisted copper wire pairs.
- **Unguided Media:** If connecting via **Wi-Fi (IEEE 802.11)**, bits travel through free space as unguided radio waves.

### 2. Transmission Modes (Lecture 1, Slide 25)
- **Simplex:** One direction only (e.g. keyboard to PC).
- **Half-Duplex:** Two directions, but one at a time (e.g. walkie-talkie).
- **Full-Duplex:** Simultaneous two-way transmission. Our program achieves this using C++ `std::thread` to run separate `send` and `recv` loops concurrently.

### 3. OSI 7-Layer Model (Lecture 2)
- **Layer 7 (Application):** Our C++ terminal chat interface, `cmd:` remote execution, and TightVNC RFB (Remote Framebuffer) protocol.
- **Layer 6 (Presentation):** ASCII string encoding and VNC pixel/compression translation.
- **Layer 5 (Session):** Established via TCP 3-way handshake session management.
- **Layer 4 (Transport):** **TCP** providing reliable, ordered delivery, error control, and port addressing (**Port 8080** for chat, **Port 5900** for VNC).
- **Layer 3 (Network):** **IP** protocol delivering packets to host IP addresses (`192.168.0.119`).
- **Layer 2 (Data Link):** Ethernet / Wi-Fi MAC framing, physical addressing, and CRC error checking.
- **Layer 1 (Physical):** Bit transmission over copper cable or radio frequencies.

### 4. TCP 3-Way Handshake (Lecture 2)
When the Client connects:
1. `Client` sends `SYN` (synchronize sequence number).
2. `Server` replies `SYN + ACK` (acknowledgment).
3. `Client` sends `ACK`.
4. TCP connection reaches `ESTABLISHED` state, and full-duplex socket streaming begins.
