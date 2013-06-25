// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

/** Function that computes a one's completement checksum over two 32-bit
 * values. Both values are assumed to be in host order (not network order).
 *
 * \param x first 32-bit value
 *
 * \param y second 32-bit value
 *
 * \returns a 16-bit value which is the ones complement checksum, suitable
 *          for passing to onesChecksum(). This is a value in host-order.
 */
unsigned int onesAdd(unsigned int x, unsigned int y);

/** Function that computes a one's complement checksum over a section of data.
 *
 * \param sum   initial checksum, typically zero or the checksum value of a
 *              pseudo header (TCP & UDP). The sum may be a 32-bit value,
 *              but must be in host-order.
 *
 * \param data  array of data over which to compute the checksum. The data array
 *              is assumed to be in network order.
 *
 * \param begin index of first entry of array.
 *
 * \param lengthInBytes   Number of bytes over which to compute checksum.
 *
 * \returns the checksum in network order, xored with 0xffff. Possible
 *          values are in the range 0x0001-0xffff (0x0000 is reserved for no checksum) 
 */
unsigned int onesChecksum(unsigned int sum, unsigned short data[], int begin, int lengthInBytes);
