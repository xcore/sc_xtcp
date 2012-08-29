// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <xs1.h>
#include "xtcp_client.h"
#include "xtcp_buffered_client.h"
#include "demo_protocol.h"
#include "print.h"

#define NUM_DEMO_PROTOCOL_CONNECTIONS 3

#define DEMO_PROTOCOL_CONN_TIMEOUT_TIMER_TICKS  (DEMO_PROTOCOL_CONN_TIMEOUT_MS * XS1_TIMER_KHZ)

// array of protocol state buffers
demo_protocol_state_t connection_states[NUM_DEMO_PROTOCOL_CONNECTIONS];

char echo_header[] = "Echo: ";

/*
 * Initialize the protocol:
 *
 * - initialise the protocol state array with 'inactive' flags
 * - 'listen' on the protocol server ports
 */
void demo_protocol_init(chanend tcp_svr) {
	for (int i = 0; i < NUM_DEMO_PROTOCOL_CONNECTIONS; i++) {
		connection_states[i].active = 0;
	}
	xtcp_listen(tcp_svr, DEMO_PROTOCOL_PORT, XTCP_PROTOCOL_TCP);
}

/*
 * The xtcp stack has sent a data received event:
 *
 * - use buffered receive until an entire packet has been read in
 *   and processed.
 *
 */
void demo_protocol_recv(chanend tcp_svr, xtcp_connection_t *conn) {
	struct demo_protocol_state_t *st =
			(struct demo_protocol_state_t *) conn->appstate;

	unsigned len=0;
	int overflow=0;
	char *buf;

	if (!st) return;

	do
	{
		len = xtcp_buffered_recv_upto(tcp_svr, conn, &st->bufinfo, &buf, '\n', &overflow);

		if (len>0 && overflow==0)
		{
			xtcp_buffered_send(tcp_svr, conn, &st->bufinfo, echo_header, sizeof(echo_header));
			xtcp_buffered_send(tcp_svr, conn, &st->bufinfo, buf, len);
		}
	} while (len != 0 && overflow==0);

	return;
}

void demo_protocol_send(chanend tcp_svr, xtcp_connection_t *conn) {
	struct demo_protocol_state_t *st =
			(struct demo_protocol_state_t *) conn->appstate;
	xtcp_buffered_send_handler(tcp_svr, conn, &st->bufinfo);
	return;
}

/*
 * This associates some buffer state with a conenction.  It uses three
 * API calls.
 *
 * xtcp_set_connection_appstate - for attaching a piece of user state
 * to a xtcp connection.
 *
 * xtcp_buffered_set_rx_buffer - to associate the rx memory buffer with a
 * buffered connection.
 *
 * xtcp_buffered_set_tx_buffer - to associate the tx memory buffer with a
 * buffered connection.
 */
static void demo_protocol_init_state(chanend tcp_svr, xtcp_connection_t *conn,
		int timestamp) {
	int i;

	for (i = 0; i < NUM_DEMO_PROTOCOL_CONNECTIONS; i++) {
		if (!connection_states[i].active)
			break;
	}

	if (i == NUM_DEMO_PROTOCOL_CONNECTIONS)
	{
		xtcp_abort(tcp_svr, conn);
	} else {
		connection_states[i].active = 1;
		connection_states[i].last_used = timestamp;
		connection_states[i].conn_id = conn->id;
		xtcp_set_connection_appstate(tcp_svr, conn,
				(xtcp_appstate_t) &connection_states[i]);
		xtcp_buffered_set_rx_buffer(tcp_svr, conn,
				&connection_states[i].bufinfo, connection_states[i].inbuf,
				DEMO_PROTOCOL_RXBUF_LEN);
		xtcp_buffered_set_tx_buffer(tcp_svr, conn,
				&connection_states[i].bufinfo, connection_states[i].outbuf,
				DEMO_PROTOCOL_TXBUF_LEN, DEMO_PROTOCOL_MAX_MSG_SIZE);
	}
	return;
}

/*
 * This is called when we no longer want a buffer associated with a connection
 */
static void demo_protocol_free_state(xtcp_connection_t *conn) {
	struct demo_protocol_state_t *st =
			(struct demo_protocol_state_t *) conn->appstate;

	if (st) {
		st->active = 0;
	}
}

/*
 * This processes the events from the xtcp stack.
 *
 * new connections cause us call demo_protocol_init_state to find a new
 * connection management buffer and fill it in.
 *
 * the received data event causes us to call demo_protocol_recv which
 * will receive a message from the stack, process it and ultimately
 * send a response
 *
 * the various send data events cause us to call demo_protocol_send which
 * forwards the send request to the buffered protocol send handler.
 *
 * the various close/timeout events cause us to call demo_protocol_free_state
 * to free up the handler buffer
 */
void demo_protocol_handle_event(chanend tcp_svr, xtcp_connection_t *conn,
		int timestamp) {
	// We have received an event from the TCP stack, so respond
	// appropriately

	// Ignore events that are not directly relevant to us
	switch (conn->event) {
	case XTCP_IFUP:
	case XTCP_IFDOWN:
	case XTCP_ALREADY_HANDLED:
		return;
	default:
		break;
	}

	if (conn->local_port == DEMO_PROTOCOL_PORT) {
		struct demo_protocol_state_t *st =
				(struct demo_protocol_state_t *) conn->appstate;
		switch (conn->event) {
		case XTCP_NEW_CONNECTION:
			demo_protocol_init_state(tcp_svr, conn, timestamp);
			break;
		case XTCP_RECV_DATA:
			if (st) {
				st->last_used = timestamp;
				demo_protocol_recv(tcp_svr, conn);
			} else
				xtcp_ignore_recv(tcp_svr);
			break;
		case XTCP_SENT_DATA:
		case XTCP_REQUEST_DATA:
		case XTCP_RESEND_DATA:
			if (st) {
				st->last_used = timestamp;
				demo_protocol_send(tcp_svr, conn);
			} else
				xtcp_ignore_send(tcp_svr);
			break;
		case XTCP_TIMED_OUT:
		case XTCP_ABORTED:
		case XTCP_CLOSED:
			demo_protocol_free_state(conn);
			break;
		default:
			// Ignore anything else
			break;
		}
		conn->event = XTCP_ALREADY_HANDLED;
	}

	return;
}

void demo_protocol_periodic(chanend tcp_svr, int t) {
	// timeout any inactive connections
	for (int i = 0; i < NUM_DEMO_PROTOCOL_CONNECTIONS; i++) {
		struct demo_protocol_state_t *st = &connection_states[i];
		xtcp_connection_t conn;
		if (st->active) {
			if ((t - st->last_used) > DEMO_PROTOCOL_CONN_TIMEOUT_TIMER_TICKS) {
				conn.id = st->conn_id;
				printstr("timed out\n");
				xtcp_close(tcp_svr, &conn);
			}
		}
	}
}
