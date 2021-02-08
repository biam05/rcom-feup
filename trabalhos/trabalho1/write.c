
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constraints.h"
#include "logical_link.c"
#include "application_layer.c"
#include "file_interaction.c"

// ------------------------------------------------------------
// ---------------       GLOBAL VARIABLES       ---------------
// ------------------------------------------------------------

extern int finished;

// ------------------------------------------------------------
// ---------------             MAIN             ---------------
// ------------------------------------------------------------

void description(int filesize, double time) {
  printf("-----------------------------------\n");
  printf(" * Filename: %s\n", FILENAME);
  printf(" * FileSize: %d\n", filesize);
  printf(" * Baudrate: %d\n", BAUDRATE);
  printf(" * Retransmission attempts: %d\n", MAX_RETRANSMISSIONS);
  printf(" * Propagation Delay: %d\n", PROPAGATION_DELAY);
  printf(" * Timeout time: %d\n", TIME);
  printf(" * Chars read per time: %d\n", MIN);
  printf(" * Data Per Packet: %d\n", DATA_SIZE);
  printf("-----------------------------------\n");
  printf(" * Time: %f\n", time);
  printf("-----------------------------------\n");
}

int main(int argc, char** argv) {
  if ((argc < 2) || ((strcmp(SERIALPORT, argv[1])!=0) && (strcmp(MODEMDEVICE, argv[1])!=0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial %s\n", MODEMDEVICE);
    return 1;
  }

  if (DATA_SIZE > MAX_DATA_SIZE) {
    printf("Invalid number of bytes per packet.\n");
    return 1;
  }

  // Time
  time_t start_time = time(NULL);
  
  char * filename = FILENAME;
  if (verify_file(filename) != 0) {
    printf("Couldn't find file.\n");
    return 1;
  }
  
  //Open Logical Link Connection
  int com = (strcmp(SERIALPORT, argv[1]) == 0) ? COM1 : COM2;
  int fd = llopen(com, TRANSMITTER);
  if (fd < 0) {
    printf("Error when stablishing connection.\n");
    return 1;
  }

  struct packet p;

  // Send Starting Control Packet
  struct control_packet c;
  c.file_size = get_size_from_file(filename);
  int filesize = c.file_size;
  if (c.file_size == 0) {
    printf("Error when obtaining the file size.\n");
    llclose(fd);
    return 1;
  }
  c.filename_size = sizeof(FILENAME);
  strcpy(c.filename, filename);
  
  int error = FALSE;
  
  p = construct_control_packet(c);
  if (p.size <= 0) {
    printf("Error when constructing the control packet.\n");
    error = TRUE;
  }
  
  if (llwrite(fd, p.packet_bytes, p.size) < 0) {
    printf("Error when sending the starting control packet.\n");
    error = TRUE;
  }

  // Send Data Through Logical Link
  struct data_packet d; d.size = 0;  

  while (finished == FALSE) {
    d = read_from_file(filename);
    if (d.size > 0) {
      p = construct_data_packet(d);
      if (p.size <= 0) {
        printf("Error when constructing the data packet.\n");
        error = TRUE;
      }
      
      // Simulate Delay
      sleep(PROPAGATION_DELAY);
      if (llwrite(fd, p.packet_bytes, p.size) < 0) {
        printf("Error when sending message.\n");
        error = TRUE;
      }
    }
  }

  // Send Ending Control Packet
  p = construct_control_packet(c);
  if (p.size <= 0) {
    printf("Error when constructing the control packet.\n");
    error = TRUE;
  }
  if (llwrite(fd, p.packet_bytes, p.size) < 0) {
    printf("Error when sending the ending control packet.\n");
    error = TRUE;
  }

  // End of Logical Link Connection
  if (llclose(fd) != 0) {
    printf("Error when closing the connection.\n");
    error = TRUE;
  }
  
  // Time
  time_t stop_time = time(NULL);
  double time = difftime(stop_time, start_time);
  description(filesize, time);
  
  if (error) { return 1; }
  return 0;
}
