// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

// All functions in this file are typically called from the Outgoing
// functions in TCP, UDP, IPv4, DHCP, etc etc.

/** Function to store a single integer at the given offset in the output
 * packet that is currently under construction. The offset has to be
 * specified in 16-bit entities, eg, 7 refers to an integer at bytes 14,
 * 15, 16, and 17 in the packet. Note that the integer is stored LSB first,
 * and that most values are to be converted to byte order first using a
 * byterev().
 *
 * \param offset Offset in the packet measured in 16-bit words.
 *
 * \param x      Integer to store.
 */
void txInt(int offset, int x);

/** Function to store a short 16-bit integer at the given offset in the
 * output packet that is currently under construction. The offset has to be
 * specified in 16-bit entities, eg, 6 refers to a short integer at bytes 12,
 * and 13 in the packet. Note that the integer is stored LSB first,
 * and that most values are to be converted to byte order first using a
 * byterev() and downshift.
 *
 * \param offset Offset in the packet measured in 16-bit words.
 *
 * \param x      Integer to store.
 */
void txShort(int offset, int x);

/** Function to store zeroes at the given offset in the output packet that
 * is currently under construction. The offset and length have to be
 * specified in 16-bit entities, eg, offset 6 and length 3 refers to zeroes
 * at bytes 12, 13, 14, 15, 16, and 17 in the packet.
 *
 * \param offset Offset in the packet measured in 16-bit words.
 *
 * \param len    Number of 16-bit words to clear.
 */
void txShortZeroes(int offset, int len);

/** Function to store a byte at the given offset in the
 * output packet that is currently under construction. The offset has to be
 * specified in octets
 *
 * \param offset Offset in the packet measured in octets
 *
 * \param x      Integer to store.
 */
void txByte(int offset, int x);

/** Function to store a sequence of bytes at the given offset in the output
 * packet that is currently under construction. The offset has to be
 * specified in 16-bit words. The bytes are read from an array at another
 * offset.
 *
 * \param offset Offset in the packet measured in 16-bit words
 *
 * \param data   Array from which bytes are copied
 *
 * \param dataOffset Index in the array where the first byte is copied from
 *
 * \param n      Number of bytes to copy.
 */
void txData(int offset, char data[], int dataOffset, int n);

/** Function to revert a short value.
 *
 * \param x the value to byte-reverse; should be less than 0x10000.
 *
 * \returns x with its least and one-but-least significant byte swapped.
 */
unsigned shortrev(unsigned x);

/** Function to clear the packet that is currently under construction. Only
 * called from the ARP layer as a way to replace the current IP packet with
 * an ARP request.
 */
void txClear();

/** Function to print the packet that is currently under construction.
 */
void txPrint();

extern short txbuf[];      // THis is not good - for checksum only. TODO.

/** Function that passes the current packet onto the MII layer and waits
 * for it to be transmitted.
 */
void doTx(chanend cOut);
