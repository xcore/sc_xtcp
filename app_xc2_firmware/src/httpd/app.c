





void httpd_sender(chanend tcp_svr, 
                  tcp_handle_t pcb,
                  int prev_output_length,
                  void *state)
{
  struct http_state *hs = state;
  int len = xtcp_max_output_len(tcp_svr, pcb);
  
  if (prev_output_length == -1) {
   
  }
  else {
    hs->dptr -= prev_output_length;
    hs->left -= prev_output_length;

    if (hs->left == 0) {
      next_cmd(hs, hs->button_info);
    }

    if (len > hs->left) {
      len = hs->left;
    }
    xtcp_write(tcp_svr, pcb, hs->dptr, len);
  }
  return;
}





void httpd_input_handler(int cmd, void *state)
{
  struct httpd_state *hs = state;
  switch (cmd) {
  case TCP_NEW_CONNECTION:
    /*    init state */
    init_httpd_state(hs);
    break;
  case TCP_INPUT:
    /* handle input */
    
    break;
}

