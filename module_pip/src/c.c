extern void pipIncomingEthernet(unsigned short[]);

void pipIncomingEthernetC(int a) {
    pipIncomingEthernet((unsigned short *) a);
}
