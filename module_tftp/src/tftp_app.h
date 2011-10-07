// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#ifndef TFTP_APP_H_
#define TFTP_APP_H_

#include <xccompat.h>

int tftp_app_process_data_block(REFERENCE_PARAM(unsigned char, data), int num_bytes);

void tftp_app_transfer_complete(void);

int tftp_app_transfer_begin(void);

void tftp_app_transfer_error(void);


#endif /* TFTP_APP_H_ */
