# TFTP Client and Server

This README provides an overview of a Trivial File Transfer Protocol (TFTP) client and server implementation. The TFTP protocol, defined in RFC 1350, facilitates simple file transfers between devices over a network.

## Trivial File Transfer Protocol

### Default TFTP

TFTP is a lightweight protocol designed for basic file transfers. It lacks advanced features like authentication and supports only reading and writing files on a remote server.

#### Packets

File transfers occur through TFTP packets exchanged between clients and servers using UDP communication. There are five types of TFTP packets: WRQ (write request), RRQ (read request), DATA, ACK (acknowledgment), and ERROR.

#### Communication

Communication begins with a client sending a WRQ or RRQ packet. The server responds accordingly, and the data transfer proceeds. An ACK packet acknowledges successful receipt of each DATA packet. Communication concludes when a DATA packet with less than 512 bytes signals the end of the file.

For detailed packet formats, refer to RFC 1350.

### Options Extension

As per RFC 2347, options modify TFTP behavior. The client and server support Blocksize, Timeout, and Transfer Size options.

#### Changes to Default Communication

Option negotiation introduces the OACK (option acknowledgment) packet and an error (error number 8) for terminating transfers due to option negotiation.

- Blocksize Option: Optimizes data transfer size in each DATA packet.
- Timeout Option: Specifies the time both client and server wait before retransmitting responses.
- Transfer Size Option: Shares information about the size of data to be transported.

Refer to RFC 2347, RFC 2348, and RFC 2349 for more details.

## Implementation

Both the client and server are implemented in C++ to run on Linux-based operating systems, following object-oriented principles.

### Common Parts

Code common to both implementations includes TFTP packets, options, I/O handlers, UDP communication, SIGINT handling, and logging.

### TFTP Client

The client handles command-line arguments, initiates communication, and manages I/O.

#### How to Use

```bash
tftp-client -h hostname [-p port] [-f filepath] -t dest_filepath
```

- -h: Remote server's IP address or domain name.
- -p: Remote server port (default assumed if not specified).
- -f: Path to the file to download (stdin used for upload).
- -t: Destination path on the remote server or locally.



### TFTP Server

The server waits for incoming packets, processes them in separate threads, and handles client communication.

#### How to Use

```bash
tftp-server [-p port] root_dirpath
```

- -p: Local port for incoming connections.
- root_dirpath: Path to the root directory for storing incoming files.



### Known Limitations

- Maximum file size: Limited by the two-byte block number in TFTP packets.
- Data loss: Possible due to UDP's lack of reliability.
- Directory hierarchy: TFTP does not create directories, and writing to non-existent directories fails.
- File operations: No support for renaming or deleting files on the server.
- For more information, consult the relevant RFCs (1350, 2347, 2348, 2349), and the source code for implementation details.

### Implementation graphs

#### Client

![Graph of include dependencies of TFTP Client](./graphs/client_graph.png)

#### Server

![Graph of include dependencies of TFTP Server](./graphs/client_graph.png)
