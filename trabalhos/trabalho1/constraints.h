#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

// ------------------------------------------------------------
// ---------------         Main Related         ---------------
// ------------------------------------------------------------

#define SERIALPORT                  "/dev/ttyS0"
#define MODEMDEVICE                 "/dev/ttyS1"
#define COM1                        1
#define COM2                        2
#define _POSIX_SOURCE               1         // POSIX compliant source
#define FALSE                       0
#define TRUE                        1

// ------------------------------------------------------------
// ---------------       Application Layer      ---------------
// ------------------------------------------------------------

// ----- Packets : Formatting and Types -----
#define SIZE_PER_BYTE               256       // Max Size Per Byte

#define C_DATA_PACKET               1
#define C_START_PACKET              2
#define C_END_PACKET                3

// ---- Data Packet ----
//  C  |  N  |  L2  |  L1  |  P 
#define NON_INFO_BYTES_PER_PACKET   4         // Bytes in Data Packet not used for Information
#define MAX_DATA_SIZE               (SIZE_PER_BYTE-1) * SIZE_PER_BYTE + (SIZE_PER_BYTE-1)      // Max Information Size Per Packet
#define DATA_SIZE                   4096      // Bytes of Information Per Packet

struct data_packet {
  unsigned char data[DATA_SIZE];
  int size;
};

// ---- Control Packet ----
//  C  |  T1  |  L1  |  V1  |  T2  |  L2  |  V2
#define T_LENGTH                    0         // Set File Length
#define T_FILENAME                  1         // Set File Name
#define MAX_FILENAME_SIZE           SIZE_PER_BYTE

struct control_packet {
  char filename[MAX_FILENAME_SIZE];
  int filename_size;
  int file_size;
};

// ------------------------------------------------------------
// ---------------        File Interaction      ---------------
// ------------------------------------------------------------
#define FILENAME                    "pinguim.gif"
#define MAX_READ_CHARS              DATA_SIZE         // Max Chars Read From a File At a Time

// ------------------------------------------------------------
// ---------------    Logical Link Connection   ---------------
// ------------------------------------------------------------

// ----- Connection Device -----
#define UNDEFINED                  -1
#define TRANSMITTER                 0
#define RECEIVER                    1

// ----- Connection Parameters -----
#define BAUDRATE                B38400
#define TIME                        3         // Inter-character timer - TIME seconds
#define MIN                         0         // Blocking read until MIN chars read
#define MAX_RETRANSMISSIONS         3         // Number of Alarm Interruptions
#define PROPAGATION_DELAY           2         // Propagation Time

// ----- Frames : Formatting and Types -----       
#define MAX_INFORMATION_SIZE        DATA_SIZE + NON_INFO_BYTES_PER_PACKET         // Max Bytes Transfered Per Frame
#define MAX_INFORMATION_WRITE_SIZE  2 * MAX_INFORMATION_SIZE

struct packet {
  unsigned char packet_bytes[MAX_INFORMATION_WRITE_SIZE];
  int size;
};

#define FLAG                        0x7E      // Framing Delimiter

// ----- A Byte -----
#define A_CMD_SND                   0x03      // Command sent by Transmitter
#define A_CMD_RCV                   0x01      // Command sent by Receiver
#define A_ANS_SND                   0x01      // Answer  sent by Transmitter
#define A_ANS_RCV                   0x03      // Answer  sent by Receiver

// ----- C Byte -----
#define C_SET                       0x03      // SET (set up)
#define C_DISC                      0X0B      // DISC (disconnect)
#define C_UA                        0x07      // UA (unnumbered acknowledgment)
#define C_RR0                       0x05      // RR (receiver ready/positive ACK) with next sequence number 0
#define C_RR1                       0x85      // RR (receiver ready/positive ACK) with next sequence number 1
#define C_REJ0                      0x01      // REJ (reject/negative ACK) with next sequence number 0
#define C_REJ1                      0x81      // REJ (reject/negative ACK) with next sequence number 1
#define C_SEQUENCE_0                0x00      // Information Frame with sequence number 0
#define C_SEQUENCE_1                0x40      // Information Frame with sequence number 1
#define C_ERROR                     0xFF      // Error

// ----- Information Frame -----
//  FLAG  |  A  |  C  |  BCC1  |  D  |  BCC2  |  FLAG

// ----- Index -----
#define I_START_FLAG_INDEX          0
#define I_A_INDEX                   1
#define I_C_INDEX                   2
#define I_BCC1_INDEX                3
#define I_INFORMATION_INDEX         4
#define I_BCC2_INDEX                5
#define I_FINISH_FLAG_INDEX         6
#define I_END_INDEX                 7

// ----- Byte Stuffing -----
#define ESCAPE                      0x7D      // Escape Character
#define ESCAPE_XOR                  0x20      // Used in a XOR to obtain the character that follows an escape character
#define ESCAPE_ESCAPE               ESCAPE_XOR ^ ESCAPE
#define ESCAPE_FLAG                 ESCAPE_XOR ^ FLAG

// ----- Supervision Frame -----
//  FLAG  |  A  |  C  |  BCC1  |  FLAG

// ----- Index -----
#define S_START_FLAG_INDEX          0
#define S_A_INDEX                   1
#define S_C_INDEX                   2
#define S_BCC1_INDEX                3
#define S_FINISH_FLAG_INDEX         4
#define S_END_INDEX                 5

#endif // CONSTRAINTS_H
