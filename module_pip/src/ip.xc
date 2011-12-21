void pipIncomingIP(char packet[]) {

#if defined(PIP_TCP)
    if (ipType == TCP) {
        pipIncomingTCP();
        return;
    }
#endif

#if defined(PIP_UDP)
    if (ipType == UDP) {
        pipIncomingUDP();
        return;
    }
#endif

}
