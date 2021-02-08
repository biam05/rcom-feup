#include <string.h>

#include "constraints.h"

// ------------------------------------------------------------
// ---------------       GLOBAL VARIABLES       ---------------
// ------------------------------------------------------------

int last_packet_number = -1;
int started = FALSE;
struct control_packet start_packet; // RECEIVER
int size = 0;                       // RECEIVER
int control_packet_count = 0;       // RECEIVER

// ------------------------------------------------------------
// ---------------      PACKET CONSTRUCTION     ---------------
// ------------------------------------------------------------

/**
 * Turns the received array into a data packet
 * 
 * Return Values :
 *        success   -> data_packet with size > 0
 *        unsuccess -> data_packet with size <= 0
**/
struct data_packet deconstruct_data_packet(struct packet p) {
  // C  |  N  |  L2  |  L1  |  P
  struct data_packet d;
  bzero(d.data, DATA_SIZE);
  d.size = 0;
  int index = 0;

  if (p.packet_bytes[index ++] != C_DATA_PACKET) {
    printf("This packet is not a data packet.\n");
    d.size = -1;
    return d;
  }

  int packet_number = (last_packet_number + 1) % SIZE_PER_BYTE;
  if (packet_number != p.packet_bytes[index ++]) {
    printf("This data packet is out of sequence.\n");
    return d;
  }

  d.size = p.packet_bytes[index ++] * SIZE_PER_BYTE;
  d.size = d.size + p.packet_bytes[index ++];

  for (int i = 0; i < d.size; ++ i) {
    d.data[i] = p.packet_bytes[index ++];
  }

  last_packet_number = packet_number;
  size = size + d.size;
  return d;
}

/**
 * Turns the received array into a control packet
 * 
 * Return Values :
 *        success   -> control_packet with file_size > 0
 *        unsuccess -> control_packet with file_size <= 0
**/
struct control_packet deconstruct_control_packet(struct packet p) {
  // C  |  T1  |  L1  |  V1  |  T2  |  L2  |  V2
  struct control_packet c;
  bzero(c.filename, MAX_FILENAME_SIZE);
  c.file_size = 0;
  c.filename_size = 0;
  int index = 0;

  ++ control_packet_count;
  if (control_packet_count > 2) {
    printf("Too much control packets.\n");
    return c;
  }

  unsigned char c_byte = p.packet_bytes[index ++];

  if (c_byte == C_DATA_PACKET) {
    printf("This packet is not a control packet.\n");
    c.file_size = -1;
    return c;
  }
  else if ((!started) && (c_byte != C_START_PACKET)) {
    printf("This control packet is out of order.\n");
    return c;
  }
  else if ((started) && (c_byte != C_END_PACKET)) {
    printf("This control packet is out of order.\n");
    return c;
  }

  unsigned char t;
  int l;
  while (index < p.size) {
    t = p.packet_bytes[index ++];
    l = p.packet_bytes[index ++];

    switch(t) {
      case T_FILENAME:
        c.filename_size = l;
        for (int i = 0; i < l; ++ i) {
          c.filename[i] = p.packet_bytes[index ++];
        }
        break;
      case T_LENGTH:
        c.file_size = 0;
        for (int i = 0; i < l; ++ i) {
          c.file_size = c.file_size * SIZE_PER_BYTE + p.packet_bytes[index ++];
        }
        break;
      default:
        printf("This type is not defined.\n");
        break;
    }
  }

  if (!started) { started = TRUE; start_packet = c; }
  else if ((started) && ((strcmp(start_packet.filename, c.filename) != 0) || (start_packet.file_size != c.file_size))) {
    printf("Start Packet did not match Finish Packet.\n");
    c.file_size = 0;
  }
  else if ((started) && (size != start_packet.file_size)) {
    printf("Received size doesn't match control size.\n");
    c.file_size = -2;
  }
  return c;
}

/**
 * Turns the data packet into the outgoing array
 * 
 * Return Values :
 *        success   -> packet with size > 0
 *        unsuccess -> packet with size == 0
**/
struct packet construct_data_packet(struct data_packet d) {
  // C  |  N  |  L2  |  L1  |  P
  int packet_number = (last_packet_number + 1) % SIZE_PER_BYTE;
  struct packet p;
  p.size = 0;
  
  if ((d.size == 0) || (d.size > DATA_SIZE)) {
    printf("Error with data packet size.\n");
    return p;
  }

  p.packet_bytes[p.size ++] = C_DATA_PACKET;
  p.packet_bytes[p.size ++] = packet_number;
  p.packet_bytes[p.size ++] = d.size / SIZE_PER_BYTE;
  p.packet_bytes[p.size ++] = d.size % SIZE_PER_BYTE;

  for (int index = 0; index < d.size; ++ index) {
    p.packet_bytes[p.size ++] = d.data[index];
  }

  last_packet_number = packet_number;
  return p;
}

/**
 * Turns the control packet into the outgoing array
 * 
 * Return Values :
 *        success   -> packet with size > 0
 *        unsuccess -> packet with size == 0
**/
struct packet construct_control_packet(struct control_packet c) {
  // C  |  T1  |  L1  |  V1  |  T2  |  L2  |  V2

  struct packet p;
  p.size = 0;
  unsigned char array[SIZE_PER_BYTE];
  int size_aux = c.file_size;
  int m = SIZE_PER_BYTE, divisions = 0;

  if (!started) { p.packet_bytes[p.size ++] = C_START_PACKET; started = TRUE; }
  else          { p.packet_bytes[p.size ++] = C_END_PACKET; }
  p.packet_bytes[p.size ++] = T_LENGTH;

  while (size_aux > 0) {
    array[-- m] = size_aux % SIZE_PER_BYTE;
    size_aux    = size_aux / SIZE_PER_BYTE;
    ++ divisions;
  }

  p.packet_bytes[p.size ++] = divisions;

  for (int i = 0; i < divisions; ++ i) {
    p.packet_bytes[p.size ++] = array[m ++];
  }

  p.packet_bytes[p.size ++] = T_FILENAME;
  p.packet_bytes[p.size ++] = c.filename_size;
  for (int i = 0; i < c.filename_size; ++ i) {
    p.packet_bytes[p.size ++] = c.filename[i];
  }

  return p;
}

/**
 * Determines if the packet is a data packet or not
 * 
 * Return Values :
 *        success   -> TRUE
 *        unsuccess -> FALSE
**/
int is_data_packet(struct packet p) {
  return (p.packet_bytes[0] == C_DATA_PACKET);
}

/**
 * Verifies data coherence
 * 
 * Return Values :
 *        success   ->  0
 *        unsuccess -> -1
**/
int verify() {
  if (control_packet_count != 2) {
    printf("Wrong Number of Control Packets.\n");
    return -1;
  }
  if (size != start_packet.file_size) {
    printf("Wrong File Size.\n");
    return -1;
  }
  return 0;
}
