# TCP-Client-Server

This C program establishes a simple file transfer mechanism between a client and a server using TCP sockets. The client reads a file, sends it to the server, and the server saves it to disk.

## Usage

### Client

- **Compile:** `gcc client.c -o client`
- **Run:** `./client <server_ip> <server_port>`

### Server

- **Compile:** `gcc server.c -o server`
- **Run:** `./server <server_port>`

## Program Flow

1. **Client Side:**
   - The client establishes a TCP socket connection to the server.
   - It prompts the user to enter a file name. To exit, the user can input "DONE."
   - The client sends the file name's size, the file name itself, and the file size to the server.
   - It reads the file in chunks, sends each chunk to the server, and repeats until the entire file is sent.
   - The client then receives acknowledgment from the server about the bytes received.

2. **Server Side:**
   - The server creates a socket, binds it to an address, and listens for incoming connections.
   - Upon connection, it receives the file name's size, the file name, and the file size from the client.
   - It reads the file name in chunks, creates a file with that name, and reads the file content from the client.
   - The server writes the received content to the file and sends an acknowledgment about the total bytes received back to the client.
   - The server loops to handle multiple file transfers until the client inputs "DONE."

## Important Points

- The client and server communicate using socket programming and the TCP protocol.
- The file size and file name's size are sent in network byte order (using `ntohs` and `htons`).
- Error handling is implemented for various stages, such as file opening, socket creation, and data transmission.
- The server listens for multiple client connections and handles each file transfer independently.
