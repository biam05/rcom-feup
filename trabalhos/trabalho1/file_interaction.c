#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>

#include "constraints.h"

// ------------------------------------------------------------
// ---------------       GLOBAL VARIABLES       ---------------
// ------------------------------------------------------------

FILE * aux_f = NULL;  // TRANSMITTER
int finished = FALSE; // TRANSMITTER

// ------------------------------------------------------------
// ---------------         FILE RELATED         ---------------
// ------------------------------------------------------------

/**
 * Read from the file
 * 
 * Reading information from the file
 * to a data struct
 * 
 * Return Values :
 *        success   -> d.size  >  0
 *        unsuccess -> d.size ==  0
 *        finished  -> d.size == -1
**/
struct data_packet read_from_file(char * filename) {
  struct data_packet d;
  d.size = 0;

  if (finished) {
    d.size = -1;
    return d;
  }

  // Open the file in read only mode
  FILE *f;
  if (aux_f == NULL) {
    f = fopen(filename, "rb");
    if (f == NULL) { printf("Couldn't read from file.\n"); finished = TRUE; return d; }
  }
  else f = aux_f;  

  // Read from file to a data_packet
  d.size = fread(d.data, 1, MAX_READ_CHARS, f);
  if (ferror(f)) { printf("Error when reading from file.\n"); return d; }

  if (d.size < MAX_READ_CHARS) {
    aux_f = NULL;
    finished = TRUE;
    if (fclose(f) != 0) { printf("Error when closing the file.\n"); }
  }
  else aux_f = f;

  return d;
}

/**
 * Write to the file
 * 
 * Takes the information from the data packet
 * Appends that information in the file
 * 
 * Return Values :
 *        success   ->  0
 *        unsuccess -> -1
**/
int write_to_file(char * filename, struct data_packet d) {
  // Open the file in append mode (Creating it if it doesn't exit)
  FILE * f = fopen(filename, "a");
  if (f == NULL) { printf("Couldnt't open file.\n"); }

  // Write the data to the file (appending it)
  int res = fwrite(d.data, 1, d.size, f);
  if (res != d.size) { printf("Error when writing to file.\n"); }

  // Close the file
  res = fclose(f);
  if (res != 0) { printf("Error when closing the file.\n"); }
  return 0;
}

/**
 * Get Size from a File
 * 
 * Return Values :
 *        success   -> size from file
 *        unsuccess -> 0
**/
off_t get_size_from_file(char * filename){
  struct stat info;
  stat(filename, &info);
  return info.st_size;
}

/**
 * Verify if there is a file with name filename
 * 
 * Return Values :
 *        success   ->  0
 *        unsuccess -> -1
**/
int verify_file(char * filename) {
  FILE * f = fopen(filename, "r");
  if (f == NULL) { return -1; }
  fclose(f);
  return 0;
}
