#ifndef TFTP_CONF_H_
#define TFTP_CONF_H_

#ifndef TFTP_DEBUG_PRINT
#define TFTP_DEBUG_PRINT      0
#endif

#ifndef TFTP_DEFAULT_PORT
#define TFTP_DEFAULT_PORT     69
#endif

#ifndef TFTP_BLOCK_SIZE
#define TFTP_BLOCK_SIZE       512       /* 512 bytes */
#endif
#ifndef TFTP_MAX_FILE_SIZE
#define TFTP_MAX_FILE_SIZE      (128 * 1024)  /* 128 KB */
#endif

// The number of seconds after which the connection will close if no new data is received
#ifndef TFTP_TIMEOUT_SECONDS
#define TFTP_TIMEOUT_SECONDS    3
#endif

// The maximum length in bytes of the custom messages in error packets.
// Should always be >= the length of the longest error message.
#ifndef TFTP_ERROR_MSG_MAX_LENGTH
#define TFTP_ERROR_MSG_MAX_LENGTH 25
#endif

// By default, the TFTP client only accepts an image with the filename TFTP_IMAGE_FILENAME.
// Setting TFTP_ACCEPT_ANY_FILENAME to non-zero allows the client to accept any filename
#ifndef TFTP_ACCEPT_ANY_FILENAME
#define TFTP_ACCEPT_ANY_FILENAME  0
#endif

#if !TFTP_ACCEPT_ANY_FILENAME
#ifndef TFTP_IMAGE_FILENAME
#define TFTP_IMAGE_FILENAME   "upgrade.bin" /* Don't forget quotes around the string */
#endif
#endif

#endif /* TFTP_CONF_H_ */
