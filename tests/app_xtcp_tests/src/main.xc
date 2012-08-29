// Copyright (c) 2011, XMOS Ltd, All rights reserved
// This software is freely distributable under a derivative of the
// University of Illinois/NCSA Open Source License posted in
// LICENSE.txt and at <http://github.xcore.com/>

#include <platform.h>
#include <print.h>
#include "uip_server.h"
#include "getmac.h"
#include "ethernet_server.h"
#include "xtcp_client.h"
#include "stdlib.h"
#include "xtcp_blocking_client.h"

#ifndef DEVICE_ADDRESS
#define DEVICE_ADDRESS 10,0,102,200
#endif

#ifndef HOST1_ADDRESS
#define HOST1_ADDRESS 10,0,102,42
#endif

#ifndef HOST2_ADDRESS
#define HOST2_ADDRESS 10,0,102,65
#endif

#define RUNTEST(name, x) printstrln("*************************** " name " ***************************"); \
							  printstrln( (x) ? "PASSED" : "FAILED" )


#define ERROR printstr("ERROR: "__FILE__ ":"); printintln(__LINE__);

xtcp_ipaddr_t host_addrs[] = {{HOST1_ADDRESS}, {HOST2_ADDRESS}};
int host_port = 49454;
int src_port = 49468;

// Ethernet Ports
on stdcore[2]: port otp_data = XS1_PORT_32B; 		// OTP_DATA_PORT
on stdcore[2]: out port otp_addr = XS1_PORT_16C;	// OTP_ADDR_PORT
on stdcore[2]: port otp_ctrl = XS1_PORT_16D;		// OTP_CTRL_PORT

on stdcore[2]: clock clk_smi = XS1_CLKBLK_5;

on stdcore[2]: mii_interface_t mii =
  {
    XS1_CLKBLK_1,
    XS1_CLKBLK_2,

    PORT_ETH_RXCLK,
    PORT_ETH_RXER,
    PORT_ETH_RXD,
    PORT_ETH_RXDV,

    PORT_ETH_TXCLK,
    PORT_ETH_TXEN,
    PORT_ETH_TXD,
  };

#ifdef PORT_ETH_RST_N
on stdcore[2]: out port p_mii_resetn = PORT_ETH_RST_N;
on stdcore[2]: smi_interface_t smi = { PORT_ETH_MDIO, PORT_ETH_MDC, 0 };
#else
on stdcore[2]: smi_interface_t smi = { PORT_ETH_RST_N_MDIO, PORT_ETH_MDC, 1 };
#endif

// Static IP Config - change this to suit your network
#if 1
xtcp_ipconfig_t ipconfig =
{
  {DEVICE_ADDRESS}, // ip address
  {255,255,255,0},   // netmask
  {0,0,0,0}        // gateway
};
#else
xtcp_ipconfig_t ipconfig = {0};
#endif

int socket_send(chanend xtcp, xtcp_connection_t &conn, unsigned char buf[], int len) {
	return xtcp_write(xtcp, conn, buf, len);
}

int socket_receive(chanend xtcp, xtcp_connection_t &conn, unsigned char buf[], int len) {
	return xtcp_read(xtcp, conn, buf, len);
}


int socket_connect(chanend xtcp, xtcp_connection_t & conn, xtcp_ipaddr_t addr, int rport, xtcp_protocol_t protocol) {
	xtcp_connect(xtcp, rport, addr, protocol);

    slave xtcp_event(xtcp, conn);

    if (conn.event != XTCP_NEW_CONNECTION){
    	printstr("Received event ");
    	printintln(conn.event);
		return 0;
    }

    printstr("Connected to ");
    printintln(rport);

    return 1;
}

int socket_listen(chanend xtcp, xtcp_connection_t & conn, int lport, xtcp_protocol_t protocol) {
	xtcp_listen(xtcp, lport, protocol);

    slave xtcp_event(xtcp, conn);

    if (conn.event != XTCP_NEW_CONNECTION){
    	ERROR;
		return 0;
    }

    printstr("New connection on port ");
    printintln(conn.local_port);

    return 1;
}

int check_data(unsigned char data[], int len) {
	for (int i = 0; i < len; i++) {
		if (data[i] != i % 256){
			return 0;
		}
	}

	return 1;
}

void init_data(unsigned char data[], int len) {
	for (int i = 0; i < len; i++) {
		data[i] = i % 256;
	}
}

void zero_data(unsigned char data[], int len) {
	for (int i = 0; i < len; i++) {
		data[i] = 0;
	}
}

void wait(int ticks){
	timer tmr;
	int t;
	tmr :> t;
	tmr when timerafter(t + ticks) :> t;
}

int echo_client(chanend xtcp, xtcp_protocol_t protocol){
	unsigned char buf[1024];
	xtcp_connection_t conn;
	int len = 1024;
	int i = 0;

	wait(10000000);

	do{
		int rport = host_port + ++i;
		init_data(buf, len);
		if (!socket_connect(xtcp, conn, host_addrs[0], rport, protocol)){
			ERROR;
			return 0;
		}
		if (!socket_send(xtcp, conn, buf, len)){
			ERROR;
			return 0;
		}
		if (!socket_receive(xtcp, conn, buf, len)){
			ERROR;
			return 0;
		}
		if (!check_data(buf, len)){
			ERROR;
			return 0;
		}

		xtcp_close(xtcp, conn);

	    slave xtcp_event(xtcp, conn);
	    if (conn.event != XTCP_CLOSED){
	    	ERROR;
			return 0;
	    }
	}while ((len /= 2));

	return 1;
}

int echo_client_multihost(chanend xtcp [], xtcp_protocol_t protocol, int hosts){
	unsigned char buf[2][1024];
	xtcp_connection_t conn[4];
	int len = 1024;

	for (int i=0;i<hosts;i++){
		init_data(buf[i], len);
	}
	wait(100000000);

	for (int i=0;i<hosts;i++){
		if (!socket_connect(xtcp[i], conn[i], host_addrs[i], host_port + (100 + i), protocol)){
			ERROR;
			return 0;
		}
	}
	for (int i=0;i<hosts;i++){
		if (!socket_send(xtcp[i], conn[i], buf[i], len)){
			ERROR;
			return 0;
		}
	}

	par{
		{
			socket_receive(xtcp[0], conn[0], buf[0], len);
			if (!check_data(buf[0], len)){
				ERROR;
			}
		}
		{
			socket_receive(xtcp[1], conn[1], buf[1], len);
			if (!check_data(buf[1], len)){
				ERROR;
			}
		}
	}

//	for (int i=0;i<hosts;i++){

//		select{
//			case xtcp_event(xtcp[0], conn[0]):
//				select_xtcp_read(xtcp[0], conn[0], buf, len);
//			break;
//			case xtcp_event(xtcp[1], conn[1]):
//				select_xtcp_read(xtcp[1], conn[1], buf, len);
//			break;
//		}

//		case (int i=0; i < hosts; i++) slave xtcp [i] :> conn[i]:
//				printstr("");
//			break;
//		}

//		socket_receive(xtcp[i], conn[i], buf, len);

//	}
	for (int i=0;i<hosts;i++){
		xtcp_close(xtcp[i], conn[i]);
	    slave xtcp_event(xtcp[i], conn[i]);
	    if (conn[i].event != XTCP_CLOSED){
	    	ERROR;
			return 0;
	    }
	}

	return 1;

}

int echo_server(chanend xtcp, xtcp_protocol_t protocol){
	unsigned char buf[1024];
	unsigned int rport;
	unsigned char raddr[4];
	xtcp_connection_t conn;
	int len = 1024;
	int i = 0;
	
	do{
		int lport = src_port + ++i;
		zero_data(buf, 1024);
		if (!socket_listen(xtcp, conn, lport, protocol)){
			return 0;
		}
		if (!socket_receive(xtcp, conn, buf, len)){
			ERROR;
			return 0;
		}
		rport = conn.remote_port;
		for (int j=0;j<4;j++){
			raddr[j] = conn.remote_addr[j];
		}

		xtcp_bind_remote(xtcp, conn, raddr, rport);

		if (!socket_send(xtcp, conn, buf, len)){
			ERROR;
			return 0;
		}

		xtcp_unlisten(xtcp, lport);
		xtcp_close(xtcp, conn);

	    slave xtcp_event(xtcp, conn);
	    if (conn.event != XTCP_CLOSED){
	    	ERROR;
			return 0;
	    }
	}while ((len /= 2));
	
	return 1;
}

#define SPEED_TEST_DATA_SIZE 8192 * 10

int speed_test(chanend xtcp, xtcp_protocol_t protocol){
	unsigned char buf[4096];
	timer tmr;
	int t1, t2;
	xtcp_connection_t conn;
	int block_size;
	int bytes_to_send = SPEED_TEST_DATA_SIZE;

	if (protocol == XTCP_PROTOCOL_TCP){
		block_size = 4096;
	}else{
		block_size = XTCP_CLIENT_BUF_SIZE - 28; // - IP & UDP header
	}

	wait(100000000);

	if (!socket_connect(xtcp, conn, host_addrs[0], host_port, protocol)){
		ERROR;
		return 0;
	}

	init_data(buf, block_size);

	tmr :> t1;
	while (bytes_to_send > 0){
		socket_send(xtcp, conn, buf, block_size);
		bytes_to_send -= block_size;
	}
	tmr :> t2;

    t2 -= t1;
    t2 /= 100000;

    printstr("Sent ");
    printint(SPEED_TEST_DATA_SIZE);
    printstr(" bytes in ");

    if (t2 > 0){
    	printint(t2);
    	printstrln(" milliseconds");
    	t2 = SPEED_TEST_DATA_SIZE / t2;
    	t2 *= 1000 * 8;
    	printint(t2);
    	printstr(" bits per second\n");
    }else{
    	printstrln("< 1 millisecond");
    }

	xtcp_close(xtcp, conn);

    slave xtcp_event(xtcp, conn);
    if (conn.event != XTCP_CLOSED){
    	ERROR;
		return 0;
    }

	return 1;

}

//int dhcp(chanend xtcp){
//	xtcp_ipconfig_t ipc;
//	xtcp_get_ipconfig(xtcp, ipc);
//
//	for (int i = 0; i < 4; i++){
//		if (ipc.ipaddr[i] != ipconfig.ipaddr[i]){
//			ERROR;
//			return 0;
//		}
//	}
//
//	return 1;
//}

int init(chanend xtcp[], int links){
	xtcp_connection_t conn;

	slave xtcp_event(xtcp[0], conn);
	if (conn.event != XTCP_IFDOWN){
		ERROR;
		return 0;
	}

	for (int i=0;i<links;i++){
		slave xtcp_event(xtcp[i], conn);
		if (conn.event != XTCP_IFUP){
			ERROR;
			return 0;
		}
	}

	return 1;
}

void runtests(chanend xtcp[], int links){
	RUNTEST("init", init(xtcp, links));
	RUNTEST("udp_server_test", echo_server(xtcp[0], XTCP_PROTOCOL_UDP));
	RUNTEST("udp_client_test", echo_client(xtcp[0], XTCP_PROTOCOL_UDP));
	RUNTEST("tcp_server_test", echo_server(xtcp[0], XTCP_PROTOCOL_TCP));
	RUNTEST("tcp_client_test", echo_client(xtcp[0], XTCP_PROTOCOL_TCP));
	RUNTEST("tcp_speed_test",  speed_test(xtcp[0], XTCP_PROTOCOL_TCP));
	RUNTEST("udp_speed_test",  speed_test(xtcp[0], XTCP_PROTOCOL_UDP));
	RUNTEST("multi host",      echo_client_multihost(xtcp, XTCP_PROTOCOL_TCP, 2));
	exit(0);
}


int main(void)
{
  chan mac_rx[1], mac_tx[1], xtcp[2], connect_status;

  par
    {
      // XCore 0
      on stdcore[0]: runtests(xtcp, 2);

      // XCore 2
      on stdcore[2]: uip_server(mac_rx[0], mac_tx[0], xtcp, 2, 
                                ipconfig, connect_status);
      on stdcore[2]:
      {
        int mac_address[2];

        ethernet_getmac_otp(otp_data, otp_addr, otp_ctrl,
                            (mac_address, char[]));

        phy_init(clk_smi, 
#ifdef PORT_ETH_RST_N
                 p_mii_resetn,
#else
                 null,
#endif
                 smi, mii);

        ethernet_server(mii, mac_address,
                        mac_rx, 1, mac_tx, 1, smi, connect_status);
      }

    }
  return 0;
}
