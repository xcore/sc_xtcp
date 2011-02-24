#ifndef _mac_cusom_filter_h_
#define _mac_cusom_filter_h_

#define MAC_FILTER_ARPIP 0x1

inline int mac_custom_filter(unsigned int buf[])
{
  int result = 0;
  unsigned short etype = (unsigned short) buf[3];
  int qhdr = (etype == 0x0081);
 
  if (qhdr) {
    // has a 802.1q tag - read etype from next word
    etype = (unsigned short) buf[4];
  }

  switch (etype) {
    case 0x0608:
    case 0x0008:
      result = MAC_FILTER_ARPIP;
      break;
    default:
      break;
  }

  return result;
}

#endif
