
#include <stdio.h>
#include <string.h>

#include "constraints.h"
#include "logical_link.c"
#include "application_layer.c"
#include "file_interaction.c"

// ------------------------------------------------------------
// ---------------       GLOBAL VARIABLES       ---------------
// ------------------------------------------------------------

extern int disconnect;

// ------------------------------------------------------------
// ---------------             MAIN             ---------------
// ------------------------------------------------------------

int main(int argc, char** argv) {
  if ((argc < 2) || ((strcmp(SERIALPORT, argv[1])!=0) && (strcmp(MODEMDEVICE, argv[1])!=0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial %s\n", SERIALPORT);
    return 1;
  }
  
  //Open Logical Link Connection
  int com = (strcmp(SERIALPORT, argv[1]) == 0) ? COM1 : COM2;
  int fd = llopen(com, RECEIVER);
  if (fd < 0) {
    printf("Error when stablishing connection.\n");
    return 1;
  }

  // Receive Data from Logical Link
  struct packet p;
  struct data_packet d;
  struct control_packet c;
  strcpy(c.filename, FILENAME);
  
  while (disconnect == FALSE) {
    p.size = llread(fd, p.packet_bytes);
    if (p.size != -1) {
      if (is_data_packet(p)) {
        d = deconstruct_data_packet(p);
        if (d.size == 0) { printf("Error when obtaining information from logical link.\n"); }
        if (write_to_file(c.filename, d) == -1) { printf("Error when writing to file.\n"); }
      }
      else {
        c = deconstruct_control_packet(p);
        if (c.file_size == 0) { printf("Error when obtaining control information from logical link.\n"); }
        if (c.filename_size == 0) { strcpy(c.filename, FILENAME); }
      }
    }
  }

  // Verify Data
  if (verify() != 0) {
    printf("Error in data.\n");
    llclose(fd);
    return 1;
  }

  // End of Logical Link Connection
  if (llclose(fd) != 0) {
    printf("Error when closing the connection.\n");
    return 1;
  }
  return 0;
}
