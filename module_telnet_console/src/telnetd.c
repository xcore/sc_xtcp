#include <string.h>
#include <print.h>
#include "xtcp_client.h"
#include "xtcp_buffered_client.h"
#include "telnetd.h"
#include "telnet_protocol.h"


// Structure to hold HTTP state
typedef struct telnetd_state_t {
  int active;
  int index;
  int conn_id;
  int parse_state;
  int inptr;
  int outptr;
  int first_recv;
  xtcp_bufinfo_t bufinfo;
  char line_buf_in[TELNET_LINE_BUFFER_LEN+1];
  char outbuf[TELNET_LINE_BUFFER_LEN+1];  
} telnetd_state_t;

telnetd_state_t connection_states[NUM_TELNETD_CONNECTIONS];

enum parse_state {
  PARSING_DATA,
  PARSING_CMD,
  PARSING_OPTIONS,
  PARSING_REQUEST,
  PARSING_NOTIFICATION,
  PARSING_EOL
};

// Initiate the HTTP state
void telnetd_init(chanend tcp_svr)
{
  int i;
  // Listen on the http port
  xtcp_listen(tcp_svr, TELNETD_PORT, XTCP_PROTOCOL_TCP);
  
  for ( i = 0; i < NUM_TELNETD_CONNECTIONS; i++ )
    {
      connection_states[i].active = 0;
      connection_states[i].index = i;
    }
}



// Parses a HTTP request for a GET
static void parse_telnet_stream(chanend tcp_svr,
                                xtcp_connection_t *conn,
                                telnetd_state_t *hs, 
                                char *data, 
                                int len)
{
  int i = 0;
  
  while (i<len) {
    switch (hs->parse_state) {
    case PARSING_CMD:
      switch (data[i]) 
        {        
        case SB:
          hs->parse_state = PARSING_OPTIONS;          
          break;
        case SE:
          hs->parse_state = PARSING_DATA;
          break;
        case WILL:
        case WONT:
          hs->parse_state = PARSING_REQUEST;
          break;
        case DO:
        case DONT:
          hs->parse_state = PARSING_NOTIFICATION;
          break;
        case IP:          
          xtcp_close(tcp_svr, conn);
          return;
        default:          
          // unsupported command - ignore
          hs->parse_state = PARSING_DATA;
          break;
        };   
      i++;
      break;
    case PARSING_NOTIFICATION:
      // just ignore everything the other side tells us for now
      hs->parse_state = PARSING_DATA;
      i++;
      break;
    case PARSING_REQUEST:
#if 0
      switch (data[i]) 
        {        
        case SUPPRESS_GA:
          hs->pending_cmd = DO;
          hs->pending_cmd_data = data[i];          
          break;
        default:
          hs->pending_cmd = DONT;
          hs->pending_cmd_data = data[i];          
          break;
        };   
      telnetd_init_send(tcp_svr, hs, conn);
#endif
      hs->parse_state = PARSING_DATA;
      i++;
      break;
    case PARSING_OPTIONS:
      if (data[i] == IAC) 
        hs->parse_state = PARSING_CMD;
      i++;
      break;
    case PARSING_DATA:
      switch (data[i]) 
        {
        case IAC:
          hs->parse_state = PARSING_CMD;
          break;
        case CR:
          hs->line_buf_in[hs->inptr] = NUL;
          hs->parse_state = PARSING_EOL;
          break;
        default:
          if (hs->inptr < TELNET_LINE_BUFFER_LEN && 
              data[i] != NUL &&
              !(data[i] & 0x80)) {
            hs->line_buf_in[hs->inptr] = data[i];
            hs->inptr++;
          }
          break;
        }
      i++;
      break;
    case PARSING_EOL:
      if (data[i] == LF) {
        telnetd_recv_line(tcp_svr, hs->index, hs->line_buf_in, hs->inptr);
        hs->inptr = 0;
        hs->parse_state = PARSING_DATA;
      }
      i++;
      break;
    }
  }

  if (hs->first_recv) {
    hs->first_recv = 0;
    telnetd_new_connection(tcp_svr,hs->index);
  }
}



// Receive a HTTP request
void telnetd_recv(chanend tcp_svr, xtcp_connection_t *conn)
{
	struct telnetd_state_t *hs = (struct telnetd_state_t *) conn->appstate;
	char data[XTCP_CLIENT_BUF_SIZE];
	int len;

	// Receive the data from the TCP stack
	len = xtcp_recv(tcp_svr, data);

	// Otherwise we have data, so parse it
        parse_telnet_stream(tcp_svr, conn, hs, &data[0], len);
}



// Setup a new connection
void telnetd_init_state(chanend tcp_svr, xtcp_connection_t *conn)
{
  int i;
  
  // Try and find an empty connection slot
  for (i=0;i<NUM_TELNETD_CONNECTIONS;i++)
    {
      if (!connection_states[i].active)
        break;
    }
  
  // If no free connection slots were found, abort the connection
  if ( i == NUM_TELNETD_CONNECTIONS )
    {
      xtcp_abort(tcp_svr, conn);
    }
  else
    {
      // Otherwise, assign the connection to a slot
      connection_states[i].active = 1;
      connection_states[i].conn_id = conn->id;
      connection_states[i].parse_state = PARSING_DATA;
      connection_states[i].inptr = 0;
      connection_states[i].first_recv = 1;
      
      xtcp_buffered_set_tx_buffer(tcp_svr, 
                                  conn, 
                                  &connection_states[i].bufinfo,
                                  connection_states[i].outbuf,
                                  TELNET_LINE_BUFFER_LEN,
                                  (TELNET_LINE_BUFFER_LEN*1/4));                
      
      xtcp_set_connection_appstate(tcp_svr, conn, (xtcp_appstate_t) &connection_states[i]);

    }
}


// Free a connection slot, for a finished connection
void telnetd_free_state(xtcp_connection_t *conn)
{
	int i;

	for ( i = 0; i < NUM_TELNETD_CONNECTIONS; i++ )
	{
		if (connection_states[i].conn_id == conn->id)
		{
			connection_states[i].active = 0;
	  	}
	}
}


// HTTP event handler
void telnetd_handle_event(chanend tcp_svr, xtcp_connection_t *conn)
{
  // We have received an event from the TCP stack, so respond appropriately

  // Ignore events that are not directly relevant to http
  switch (conn->event) 
    {
    case XTCP_IFUP:
    case XTCP_IFDOWN:
    case XTCP_ALREADY_HANDLED:
      return;
    default:
      break;
    }

  if (conn->local_port == TELNETD_PORT) {
    switch (conn->event)
      {
      case XTCP_NEW_CONNECTION:
        telnetd_init_state(tcp_svr, conn);
        break;          

      case XTCP_RECV_DATA:
        telnetd_recv(tcp_svr, conn);
        break;        

      case XTCP_SENT_DATA:        
      case XTCP_REQUEST_DATA:
      case XTCP_RESEND_DATA:
        {
          struct telnetd_state_t *st = 
            (struct telnetd_state_t *) conn->appstate;
          xtcp_buffered_send_handler(tcp_svr, conn, &st->bufinfo);
          break;
        }        
      case XTCP_TIMED_OUT:
      case XTCP_ABORTED:
      case XTCP_CLOSED: {
        struct telnetd_state_t *st = (struct telnetd_state_t *) conn->appstate;
        telnetd_connection_closed(tcp_svr, st->index);
        telnetd_free_state(conn);
        break;
      }
      default:
        break;
      }
    conn->event = XTCP_ALREADY_HANDLED;
  }
  return;
}

int telnetd_send_line(chanend tcp_svr,
                      int i,
                      char line[])
{
  struct telnetd_state_t *st = &connection_states[i];
  xtcp_connection_t conn;
  int success=1;
  int len;
  char eol[2] = {CR,LF};
  int mss;

  conn.id = st->conn_id;

  len = strlen(line);

  mss = xtcp_buffered_send_buffer_remaining(&st->bufinfo) - 2;

  if (mss < 0) 
    mss = 0;

  if (len > mss) {
    success = 0;
    len = mss;
  }

  
  success |= xtcp_buffered_send(tcp_svr, &conn, &st->bufinfo, line, len);  
  success |= xtcp_buffered_send(tcp_svr, &conn, &st->bufinfo, eol, 2);  

  return success;
}

int telnetd_send(chanend tcp_svr,
                 int i,
                 char line[])
{
  struct telnetd_state_t *st = &connection_states[i];
  xtcp_connection_t conn;
  int success=1;
  int len;
  int mss;

  conn.id = st->conn_id;

  len = strlen(line);

  if (!len)
    return 0;

  mss = xtcp_buffered_send_buffer_remaining(&st->bufinfo);

  if (mss < 0) 
    mss = 0;

  if (len > mss) {
    success = 0;
    len = mss;
  }

  
  success |= xtcp_buffered_send(tcp_svr, &conn, &st->bufinfo, line, len);  

  return success;
}


