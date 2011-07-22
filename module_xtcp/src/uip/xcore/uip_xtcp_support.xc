// Copyright (c) 2011, XMOS Ltd., All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include "uip_xtcp.h"
#include <print.h>

static int linkstate = 0;

void uip_xtcp_checklink(chanend connect_status)
{
  select 
    {
    case connect_status :> int ifnum:
      {
        int status;
        connect_status :> status;
        if (!status && linkstate) {
          linkstate = 0;
          uip_linkdown();
        }
        if (status && !linkstate) {
          linkstate = 1;
          uip_linkup();  
        }
        break;
      }
    default:
      break;
    }
  return;
}
