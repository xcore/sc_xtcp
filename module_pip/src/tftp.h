/** Function that initialises the TFTP client. To be called prior to any
 * other function. Usually called from toplevel.
 */
void pipInitTFTP();

/** Function that processes an incoming TFTP packet. May trigger an
 * outgoing packet. Normally called from IPv4 layer.
 *
 * \param packet packet data.
 *
 * \param srcIP  source IP address according to IP header.
 *
 * \param dstIP  destination IP address according to IP header.
 *
 * \param srcPort  source port according to IP header.
 *
 * \param dstPort  destination port according to IP header.
 *
 * \param offset index in packet array where the first byte of the
 *               TFTP packet resides.
 *
 * \param length length of the total packet in bytes.
 */
void pipIncomingTFTP(unsigned short packet[], unsigned srcIP, unsigned dstIP, unsigned srcPort, int offset, int length);

/** Function that is called to signal a timeout on TFTP timer. Called
 * from timer.xc, set by init and incoming.
 */
void pipTimeOutTFTP();

extern unsigned pipPortTFTP;
extern unsigned pipIpTFTP;

#define TFTP_SERVER_PORT     69
