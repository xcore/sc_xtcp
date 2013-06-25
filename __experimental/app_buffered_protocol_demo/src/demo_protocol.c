// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "xtcp_client.h"
#include "xtcp_buffered_client.h"
#include "demo_protocol.h"
#include "print.h"

// This method is called to process the header.  It returns the size, which is the
// first byte of the header
int decode_demo_protocol_header(char *hdr) {
	return *hdr;
}

// This method is called by the protocol RX function when a full message is received
void demo_protocol_process_message(chanend tcp_svr, xtcp_connection_t *conn,
		demo_protocol_state_t *st, char *msg) {
	char reply[7] = { 0, 1, 2, 3, 4, 5, 6 };
	int success;

	// Print the received message to the console
	printstrln(&msg[1]);

	// Send a reply, which is a 7 byte message
	success = xtcp_buffered_send(tcp_svr, conn, &st->bufinfo, reply, 7);

	if (!success)
		printstr("send buffer overflow\n");

	return;
}

