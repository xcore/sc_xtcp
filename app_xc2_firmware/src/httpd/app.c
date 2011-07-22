/**
 * Module:  app_xc2_firmware
 * Version: 1v3
 * Build:   ceb87a043f18842a34b85935baf3f2a402246dbd
 * File:    app.c
 *
 * The copyrights, all other intellectual and industrial 
 * property rights are retained by XMOS and/or its licensors. 
 * Terms and conditions covering the use of this code can
 * be found in the Xmos End User License Agreement.
 *
 * Copyright XMOS Ltd 2009
 *
 * In the case where this code is a modification of existing code
 * under a separate license, the separate license terms are shown
 * below. The modifications to the code are still covered by the 
 * copyright notice above.
 *
 **/                                   






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

