#include <xs1.h>
#include "uip_xtcp.h"
#include <print.h>

static int linkstate = 0;

void uip_xtcp_checklink(chanend connect_status)
{
  unsigned char ifnum;
  select 
    {
    case inuchar_byref(connect_status, ifnum):
      {
        int status;
        status = inuchar(connect_status);
        (void) inuchar(connect_status);
        (void) inct(connect_status);
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
