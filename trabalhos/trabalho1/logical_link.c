/*Non-Canonical Input Processing*/

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "constraints.h"

// ------------------------------------------------------------
// ---------------       GLOBAL VARIABLES       ---------------
// ------------------------------------------------------------

int last_sequence_number = -1;
static struct termios oldtio;
int side = UNDEFINED;

int time_limit = FALSE;   // TRANSMITTER

int disconnect = FALSE;   // RECEIVER

// ------------------------------------------------------------
// ---------------       HELPER FUNCTIONS       ---------------
// ------------------------------------------------------------

/**
 * Send a Message (C)
 * 
 * Return Values :
 *        success   -> TRUE
 *        unsuccess -> FALSE
**/
int send(int fd, unsigned char a, unsigned char c) {
  unsigned char message[5];
  message[0] = FLAG;
  message[1] = a;
  message[2] = c;
  message[3] = a ^ c;
  message[4] = FLAG;

  int res = write(fd, message, 5);

  if (side == TRANSMITTER) { printf("Emitter"); }
  else                     { printf("Receiver"); }
  printf(" write %d bytes, %x %x %x %x %x\n", res, message[0], message[1], message[2], message[3], message[4]);
  
  return (res == 5);
}

/**
 * Attempts to retrieve C Byte from a frame.
 * 
 * Return Values:
 *        success   -> C
 *        unsuccess -> error (0xFF)
**/
unsigned char retrieve(int fd, unsigned char a) {
  const int MAX_CHARS_PER_READ = 1;
  unsigned char c = C_ERROR;
  unsigned char frame[6] = {0, 0, 0, 0, 0, 0}; //TODO : maybe remove
  unsigned char buf[MAX_CHARS_PER_READ];
    
  int index = 0;
  int res;

  int timeout = FALSE;
  time_t now, start = time(NULL);
  double t;

  int stop = FALSE;
  while (stop == FALSE) {               
    res = read(fd, buf, MAX_CHARS_PER_READ);
    if (res == 1) {
      switch(index) {
        case S_START_FLAG_INDEX:
          if (buf[0] == FLAG) { frame[index] = buf[0]; ++ index; }
          break;
        
        case S_A_INDEX:
          if (buf[0] == a) { frame[index] = buf[0]; ++ index; }
          else if (buf[0] != FLAG) { index = S_START_FLAG_INDEX; }
          break;
        
        case S_C_INDEX:
          if (buf[0] == FLAG) { index = S_START_FLAG_INDEX; }
          else                { frame[index] = buf[0]; c = buf[0]; ++ index; }
          break;
        
        case S_BCC1_INDEX:
          if (buf[0] == (a ^ c)) { frame[index] = buf[0]; ++ index; }
          else { return C_ERROR; }
          break;
        
        case S_FINISH_FLAG_INDEX:
          frame[index] =  buf[0];
          index ++;
          break;
        
        default:
          break;
      }
    }
    if (time_limit && (TIME != 0)) {
      now = time(NULL);
      t = difftime(now, start);
      timeout = (t > TIME * 1.1);
    }
    if (timeout && index == S_START_FLAG_INDEX) stop = TRUE;
    if (index == S_END_INDEX) stop = TRUE;
  }

  if (side == TRANSMITTER) { printf("Emitter"); }
  else                     { printf("Receiver"); }
  printf(" read  %d bytes, %x %x %x %x %x\n", index, frame[0], frame[1], frame[2], frame[3], frame[4]);
  
  return timeout ? C_ERROR : c;
}

/**
 * Send a Command Message (C)
 * 
 * Return Values :
 *        success   -> TRUE
 *        unsuccess -> FALSE
**/
int send_command(int fd, unsigned char c) {
  unsigned char a;
  switch (side) {
    case TRANSMITTER:
      a = A_CMD_SND;
      break;
    case RECEIVER:
      a = A_CMD_RCV;
      break;
    default:
      // ERROR
      return -1;
  }
  return send(fd, a, c);
}

/**
 * Send a Confirmation Response (C)
 * 
 * Return Values :
 *        success   -> TRUE
 *        unsuccess -> FALSE
**/
int send_confirmation(int fd, unsigned char c) {
  unsigned char a;
  switch (side) {
    case TRANSMITTER:
      a = A_ANS_SND;
      break;
    case RECEIVER:
      a = A_ANS_RCV;
      break;
    default:
      // ERROR
      return -1;
  }
  return send(fd, a, c);
}

/**
 * Attempts to retrieve C Command.
 * 
 * Return Values:
 *        success   -> C command
 *        unsuccess -> error (0xFF)
**/
unsigned char retrieve_command(int fd) {
  unsigned char a;
  switch (side) {
    case TRANSMITTER:
      a = A_CMD_RCV;
      break;
    case RECEIVER:
      a = A_CMD_SND;
      break;
    default:
      // ERROR
      return C_ERROR;
  }
  return retrieve(fd, a);
}

/**
 * Attempts to retrieve C Response.
 * 
 * Return Values:
 *        success   -> C response
 *        unsuccess -> error (0xFF)
**/
unsigned char retrieve_confirmation(int fd) {
  unsigned char a;
  switch (side) {
    case TRANSMITTER:
      a = A_ANS_RCV;
      break;
    case RECEIVER:
      a = A_ANS_SND;
      break;
    default:
      // ERROR
      return C_ERROR;
  }
  return retrieve(fd, a);
}

// ------------------------------------------------------------
// ---------------    Logical Link Connection   ---------------
// ------------------------------------------------------------

/**
 * Establishment of the Logical Link
 * 
 * TRANSMITTER:
 * Sends the SET message.
 * Attempts to receive UA message.
 * 
 * RECEIVER:
 * Receives the SET message.
 * When successful, sends the UA message.
 * 
 * Return Values:
 *        success   -> fd
 *        unsuccess -> -1
**/
int llopen(int com, int machine_side) {
  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  char * arg = (com == COM1) ? SERIALPORT: MODEMDEVICE;
  static struct termios newtio;
  int fd = open(arg, O_RDWR | O_NOCTTY );
  if (fd < 0) { perror(arg); return -1; }

  if (tcgetattr(fd,&oldtio) == -1) {
    /* save current port settings */
    perror("tcgetattr");
    return -1;
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME]    = TIME;   /* inter-character timer */
  newtio.c_cc[VMIN]     = MIN;    /* blocking read until MIN chars received */

  /* 
    VTIME e VMIN should be altered in order to protect
    the reading of the next chars with a timer
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    return -1;
  }

  printf("New termios structure set\n");

  side = machine_side;
  if (side == TRANSMITTER) {
    int retransmission_count = 1;
    time_limit = TRUE;
    while (retransmission_count <= MAX_RETRANSMISSIONS) {
      send_command(fd, C_SET);
      if (retrieve_confirmation(fd) == C_UA) { break; }
      ++ retransmission_count;
    }
    time_limit = FALSE;
    if (retransmission_count > MAX_RETRANSMISSIONS) {
      printf("UA not Received!\n");
      return -1;
    }
    printf("UA Received.\n");
    return fd;
  }
  else { // side == RECEIVER
    unsigned char set = retrieve_command(fd);
    if (set == C_SET) { send_confirmation(fd, C_UA); return fd; }
  }
  return -1;
}

/**
 * Transmitting of Information Through the Logical Link
 * 
 * Reads a Message.
 * Detects if the Emitter wants to Disconnect.
 * Send a Adequate Response.
 * 
 * Return Values:
 *        size of the message received
 *        -1 if no message is received
**/
int llread(int fd, unsigned char * buffer) {
  const int MAX_CHARS_PER_READ = 1;
  int size = MAX_INFORMATION_SIZE;
  bzero(buffer, size);

  unsigned char buf[MAX_CHARS_PER_READ];
  int res;
  int loop_index = 0, index = -1;
  int sequence_number = -1;
  unsigned char lastChar = 0x00;
  int escapedChar = FALSE;
  int bcc1error = FALSE, bcc2error = FALSE;
  unsigned char bcc2 = 0x00;

  int stop = FALSE;
  while (stop == FALSE) {
    res = read(fd, buf, MAX_CHARS_PER_READ);
    if (res == 1) {
      switch (loop_index) {
        case I_START_FLAG_INDEX:
          if (buf[0] == FLAG)              { ++ loop_index; }
          break;
        
        case I_A_INDEX:
          if      (buf[0] == A_CMD_SND)    { ++ loop_index; }
          else if (buf[0] != FLAG)         { loop_index = I_START_FLAG_INDEX; }
          break;
        
        case I_C_INDEX:
          if      (buf[0] == C_SEQUENCE_0) { sequence_number = 0; ++ loop_index; }
          else if (buf[0] == C_SEQUENCE_1) { sequence_number = 1; ++ loop_index; }
          else if (buf[0] == FLAG)         { loop_index = I_A_INDEX; }
          else if (buf[0] == C_DISC)       { disconnect = TRUE; ++ loop_index; }
          else                             { loop_index = I_START_FLAG_INDEX; }
          break;
        
        case I_BCC1_INDEX:
          if      ((sequence_number == 0) && (buf[0] == (A_CMD_SND ^ C_SEQUENCE_0))) { ++ loop_index; }
          else if ((sequence_number == 1) && (buf[0] == (A_CMD_SND ^ C_SEQUENCE_1))) { ++ loop_index; }
          else if ((     disconnect     ) && (buf[0] == (A_CMD_SND ^ C_DISC)))       { ++ loop_index; }
          else { bcc1error = TRUE; /* BCC1 error */ }
          break;
        
        case I_INFORMATION_INDEX:
          if (buf[0] == FLAG) {
            stop = TRUE;
            if (disconnect) { break; }
            // BCC2 is the lastChar
            if (index > size) { bcc2error = TRUE; break; /* BCC2 error */ }
            for (int i = 0; i < index; ++ i) { bcc2 = bcc2 ^ buffer[i]; }
            if (bcc2 != lastChar) { bcc2error = TRUE; /* BCC2 error */ }
          }
          else if ((lastChar == ESCAPE) && (!escapedChar)) { lastChar = buf[0] ^ ESCAPE_XOR; escapedChar = TRUE;}
          else {
            if ((index >= 0) && (index < size)) { buffer[index] = lastChar; }
            ++ index;
            lastChar = buf[0];
            escapedChar = FALSE;
          }
          break;
        default:
          break;
      }
    }
  }

  switch (sequence_number) {
    case 0:
      if (bcc1error) { printf("Error with BCC1\n"); return -1; }
      else if (bcc2error) {
        if (last_sequence_number == sequence_number) { send_confirmation(fd, C_RR1 ); } /* Duplicated Data */
        else                                         { send_confirmation(fd, C_REJ1); } /* New Data */
        printf("Error with BCC2 %x %x\n", bcc2, lastChar); return -1;
      }
      else { send_confirmation(fd, C_RR1); last_sequence_number = sequence_number; return index; }
      break;
    case 1:
      if (bcc1error) { printf("Error with BCC1\n"); return -1; }
      else if (bcc2error) {
        if (last_sequence_number == sequence_number) { send_confirmation(fd, C_RR0 ); } /* Duplicated Data */
        else                                         { send_confirmation(fd, C_REJ0); } /* New Data */
        printf("Error with BCC2\n"); return -1;
      }
      else { send_confirmation(fd, C_RR0); last_sequence_number = sequence_number; return index; }
      break;
    default:
      // Means it received DISC and is now preparing to disconnect
      break;
  }

  return index;
}

/**
 * Transmitting of Information Through the Logical Link
 * 
 * Sends a message.
 * Receives the UA message.
 * 
 * Return Values:
 *        success   ->  0
 *        unsuccess -> -1
**/
int llwrite(int fd, unsigned char * buf, int size) {
  // Message : F | A | C | Bcc1 | D | Bcc2 | F

  if (size > MAX_INFORMATION_SIZE) {
    printf("Too many chars read.\n");
    return -1;
  }

  int sequence_number = ((last_sequence_number == 0) ? 1 : 0);

  int MAX_CHARS = MAX_INFORMATION_WRITE_SIZE + I_FINISH_FLAG_INDEX;
  int i = 0, j = 0;

  unsigned char msg[MAX_CHARS];   // Frame to be sent

  msg[i++] = FLAG;                                                                // Flag
  msg[i++] = A_CMD_SND;                                                           // A    
  msg[i++] = (sequence_number == 0) ? C_SEQUENCE_0 : C_SEQUENCE_1;                // C
  msg[i++] = A_CMD_SND ^ ((sequence_number == 0) ? C_SEQUENCE_0 : C_SEQUENCE_1);  // BCC1

  unsigned char info_frame[MAX_INFORMATION_WRITE_SIZE];
  unsigned char bcc2 = 0x00;

  /*
  *       Byte Stuffing and buffer size adjusting - Information
  * 
  * If inside the information occurs the FLAG pattern,
  *   it should be replaced by the sequence ESCAPE ESCAPE_FLAG.
  * 
  * If inside the information occurs the ESCAPE pattern,
  *   it should be replaced by the sequence ESCAPE ESCAPE_ESCAPE.
  */
  for (int k = 0; k < size; ++ k) {
    if (buf[k] == FLAG) {
      info_frame[j++] = ESCAPE;
      info_frame[j++] = ESCAPE_FLAG;
    }
    else if (buf[k] == ESCAPE) {
      info_frame[j++] = ESCAPE;
      info_frame[j++] = ESCAPE_ESCAPE;
    }    
    else {
      info_frame[j++] = buf[k];
    }

    bcc2 = buf[k] ^ bcc2;
  }

  // D
  int p = 0;
  for (p = 0; p < j; ++ p) {
    msg[i+p] = info_frame[p];
  }
  i = i + p;

  // Byte Stuffing - BCC2
  if (bcc2 == FLAG) {
    msg[i++] = ESCAPE;
    msg[i++] = ESCAPE_FLAG;
  }
  else if (bcc2 == ESCAPE) {
    msg[i++] = ESCAPE;
    msg[i++] = ESCAPE_ESCAPE;
  }
  else {
    msg[i++] = bcc2;
  }
  
  msg[i++] = FLAG;                  // Flag
  
  unsigned char confirmation;
  int confirmed = FALSE;
  int retransmission_count = 1;
  while ((retransmission_count <= MAX_RETRANSMISSIONS) && (confirmed == FALSE)) {
    write(fd, msg, i);
    ++ retransmission_count;
    
    time_limit = TRUE;
    confirmation = retrieve_confirmation(fd);
    time_limit = FALSE;

    switch (confirmation) {
      case C_RR0:
        if (sequence_number == 1) { confirmed = TRUE; }
        break;
      case C_RR1:
        if (sequence_number == 0) { confirmed = TRUE; }
        break;
      default:
        /*
          Goes for C_REJ0, C_REJ1 and error
          Also C_RR0 or C_RR1 out of sequence
        */
        break;
    }
  }

  if (confirmed == FALSE) {
    printf("Confirmation not Received!\n");
    return -1;
  }

  last_sequence_number = sequence_number;
  printf("Confirmation Received.\n");
  return 0;
}

/**
 * Closing of Logical Link
 * 
 * TRANSMITTER:
 * Sends command DISC.
 * Receives DISC command.
 * Sends UA message.
 * 
 * RECEIVER:
 * Sends command DISC.
 * Receives UA message.
 * 
 * Return Values:
 *        success   ->  0
 *        unsuccess -> -1
**/
int llclose(int fd) {
  if (side == TRANSMITTER) {
    send_command(fd, C_DISC);
    time_limit = TRUE;
    unsigned char disc = retrieve_command(fd);
    time_limit = FALSE;
    if (disc != C_DISC) { printf("Error on DISC.\n"); return -1; }
    send_confirmation(fd, C_UA);
  }
  else { // side == RECEIVER
    send_command(fd, C_DISC);
    unsigned char ua = retrieve_confirmation(fd);
    if (ua != C_UA) { printf("Error on UA.\n"); }
	if (tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      printf("Error on tcsetattr\n");
      perror("tcsetattr");
      return -1;
    }
  }

  
  close(fd);
  return 0;
}
