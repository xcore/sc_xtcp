void pipDhcpIncoming(unsigned short packet[], unsigned srcIP, unsigned dstIP, int offset, int length);

void pipDhcpInit();
void pipDhcpCreate(int firstMessage,
                   unsigned proposedIP,
                   unsigned serverIP);

void pipDhcpTimeOutT1();
void pipDhcpTimeOutT2();
