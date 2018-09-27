/*
 * Copyright (c) 2017, 2018,
 * Dmitry Salychev <darkness.bsd@gmail.com>,
 * Alexander Salychev <ppsalex@rambler.ru> et al.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#ifdef MSIM_POSIX
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <netinet/in.h>
#ifndef MSIM_POSIX_CYGWIN
#include <netinet/tcp.h>
#endif

#include "mcusim/mcusim.h"

#define AVRSIM_RSP_PROTOCOL		"tcp"

#define BREAK_LOW			0x98
#define BREAK_HIGH			0x95
#define BREAK				((BREAK_HIGH<<8)|BREAK_LOW)

#define GDB_BUF_MAX			(16*1024)
#define REG_BUF_MAX			32

enum mp_type {
	BP_SOFTWARE	= 0,
	BP_HARDWARE	= 1,
	WP_WRITE	= 2,
	WP_READ		= 3,
	WP_ACCESS	= 4
};

struct rsp_state {
	char client_waiting;
	struct MSIM_AVR *mcu;		/* MCU instance */
	int proto_num;
	int fserv;			/* FD for incoming connections */
	int fcli;			/* FD for talking to GDB client */
	int sigval;			/* GDB signal for any exception */
	unsigned long start_addr;	/* Start of last run */
};

struct rsp_buf {
	char data[GDB_BUF_MAX];
	unsigned long len;
};

static struct rsp_state rsp;
static const char hexchars[] = "0123456789ABCDEF";

static void rsp_close_server(void);
static void rsp_close_client(void);
static void rsp_server_request(void);
static void rsp_client_request(void);

static struct rsp_buf *get_packet(void);
static void put_packet(struct rsp_buf *buf);
static int get_rsp_char(void);
static void put_rsp_char(char c);
static int hex(int c);
static unsigned long hex2reg(char *buf, const unsigned long dign);
static void put_str_packet(const char *str);
static void rsp_report_exception(void);
static void rsp_continue(struct rsp_buf *buf);
static void rsp_query(struct rsp_buf *buf);
static void rsp_vpkt(struct rsp_buf *buf);
static void rsp_restart(void);
static void rsp_read_all_regs(void);
static void rsp_write_all_regs(struct rsp_buf *buf);
static void rsp_read_mem(struct rsp_buf *buf);
static void rsp_write_mem(struct rsp_buf *buf);
static void rsp_write_mem_bin(struct rsp_buf *buf);
static void rsp_step(struct rsp_buf *buf);
static void rsp_insert_matchpoint(struct rsp_buf *buf);
static void rsp_remove_matchpoint(struct rsp_buf *buf);
static unsigned long rsp_unescape(char *data, unsigned long len);
static void rsp_read_reg(struct rsp_buf *buf);
static void rsp_write_reg(struct rsp_buf *buf);

static size_t read_reg(int n, char *buf);
static void write_reg(int n, char *buf);

void MSIM_RSPInit(struct MSIM_AVR *mcu, int portn)
{
	struct protoent *protocol;	/* Protocol entry */
	struct hostent *host;		/* Host entry */
	struct sockaddr_in sock_addr;	/* Socket address */
	int optval;			/* Socket options */
	int flags; 			/* Socket flags */

	/* Reset GDB RSP state */
	rsp.client_waiting = 0;		/* GDB client is not waiting */
	rsp.mcu = mcu;			/* MCU instance */
	rsp.proto_num = -1;		/* i.e. invalid */
	rsp.fserv = -1;			/* i.e. invalid */
	rsp.fcli = -1;			/* i.e. invalid */
	rsp.sigval = 0;			/* No exceptions */
	rsp.start_addr = mcu->intr->reset_pc;	/* Reset PC by default */

	protocol = getprotobyname(AVRSIM_RSP_PROTOCOL);
	if (protocol == NULL) {
		fprintf(stderr, "RSP unable to load protocol \"%s\": %s\n",
		        AVRSIM_RSP_PROTOCOL, strerror(errno));
		return;
	}

	rsp.proto_num = protocol->p_proto;

	if (portn <= IPPORT_RESERVED) {
		fprintf(stderr, "RSP could not use a reserved port: %d, "
		        "should be > %d\n", portn, IPPORT_RESERVED);
		return;
	}

	/* Create a socket using AVRSim RSP protocol */
	rsp.fserv = socket(PF_INET, SOCK_STREAM, protocol->p_proto);
	if (rsp.fserv < 0) {
		fprintf(stderr, "RSP could not create server socket: %s\n",
		        strerror(errno));
		return;
	}

	/* Set socket to reuse its address */
	optval = 1;
	if (setsockopt(rsp.fserv, SOL_SOCKET, SO_REUSEADDR, &optval,
	                sizeof optval) < 0) {
		fprintf(stderr, "RSP could not set SO_REUSEADDR option on a "
		        "server socket %d: %s\n", rsp.fserv, strerror(errno));
		rsp_close_server();
		return;
	}

	/* Server should be non-blocking */
	flags = fcntl(rsp.fserv, F_GETFL);
	if (flags < 0) {
		fprintf(stderr, "Unable to get flags for RSP server "
		        "socket %d: %s\n", rsp.fserv, strerror(errno));
		rsp_close_server();
		return;
	}
	flags |= O_NONBLOCK;
	if (fcntl(rsp.fserv, F_SETFL, flags) < 0) {
		fprintf(stderr, "Unable to set flags for RSP server socket "
		        "%d to 0x%08X: %s\n",
		        rsp.fserv, flags, strerror(errno));
		rsp_close_server();
		return;
	}

	/* Find our host entry */
	host = gethostbyname("localhost");
	if (host == NULL) {
		fprintf(stderr, "Unable to get host entry for RSP server "
		        "by 'localhost' name: %s\n", strerror(errno));
		rsp_close_server();
		return;
	}

	/* Bind socket to the appropriate address */
	memset(&sock_addr, 0, sizeof sock_addr);
	sock_addr.sin_family = (unsigned char)host->h_addrtype;
	sock_addr.sin_port = htons(portn);
	if (bind(rsp.fserv, (struct sockaddr *)&sock_addr,
	                sizeof sock_addr) < 0) {
		fprintf(stderr, "Unable to bind RSP server socket %d to "
		        "port %d: %s\n", rsp.fserv, portn, strerror(errno));
		rsp_close_server();
		return;
	}

	/*
	 * Listen to the incoming connections from GDB clients (do not allow
	 * more than 1 client to be connected simultaneously!)
	 */
	if (listen(rsp.fserv, 1) < 0) {
		fprintf(stderr, "Unable to backlog on RSP server socket %d to"
		        "%d: %s\n", rsp.fserv, 1, strerror(errno));
		rsp_close_server();
		return;
	}
}

void MSIM_RSPClose(void)
{
	rsp_close_client();
	rsp_close_server();
}

int MSIM_RSPHandle(void)
{
	struct pollfd fds[2];

	/* Give up if no RSP server port (this should not happen) */
	if (rsp.fserv == -1) {
		fprintf(stderr, "No RSP server port open\n");
		return -1;
	}

	/*
	 * If there is no RSP client, poll the server socket
	 * until we get one.
	 */
	while (rsp.fcli == -1) {
		/* Poll for a client of RSP server socket */
		fds[0].fd = rsp.fserv;
		fds[0].events = POLLIN;

		switch (poll(fds, 1, -1)) {
		case -1:
			if (errno == EINTR) {
				/* Ignore received signal while polling */
				break;
			}
			fprintf(stderr, "Poll for RSP server failed: "
			        "closing server connection: %s\n",
			        strerror(errno));
			rsp_close_client();
			rsp_close_server();
			return -1;
		case 0:
			/* Timeout. This should not happen. */
			fprintf(stderr, "Unexpected RSP server poll "
			        "timeout\n");
			break;
		default:
			if (POLLIN == (fds[0].revents & POLLIN)) {
				rsp_server_request();
				rsp.client_waiting = 0;
			} else {
				fprintf(stderr, "RSP server received flags "
				        "0x%08X: closing server connection\n",
				        fds[0].revents);
				rsp_close_client();
				rsp_close_server();
				return -1;
			}
			break;
		}
	}

	/* Response with signal 5 (TRAP exception) any time */
	if (rsp.client_waiting) {
		put_str_packet("S05");
		rsp.client_waiting = 0;
	}

	/* Poll the RSP client socket for a message from GDB */
	fds[0].fd = rsp.fcli;
	fds[0].events = POLLIN;

	/* Poll is always blocking. We have to wait. */
	switch (poll(fds, 1, -1)) {
	case -1:
		if (errno == EINTR) {
			break;
		}
		fprintf(stderr, "Poll for RSP client failed: closing server "
		        "connection: %s\n", strerror(errno));
		rsp_close_client();
		rsp_close_server();
		return -1;
	case 0:
		/* Timeout. This should not happen. */
		fprintf(stderr, "Unexpected RSP client poll timeout\n");
		return -1;
	default:
		/* Is there client activity due to input available? */
		if (POLLIN == (fds[0].revents & POLLIN)) {
			rsp_client_request();
		} else {
			/*
			 * Error leads to closing the client, but not
			 * the server.
			 */
			fprintf(stderr, "RSP client received flags "
			        "0x%08X: closing client connection\n",
			        fds[0].revents);
			rsp_close_client();
		}
		break;
	}
	return 0;
}

static void rsp_close_server(void)
{
	if (rsp.fserv != -1) {
		close(rsp.fserv);
		rsp.fserv = -1;
	}
}

static void rsp_close_client(void)
{
	if (rsp.fcli != -1) {
		close(rsp.fcli);
		rsp.fcli = -1;
	}
}

static void rsp_server_request(void)
{
	struct sockaddr_in sock_addr;	/* The socket address */
	socklen_t len;			/* Size of the socket address */
	int fd, flags, optval;

	len = sizeof sock_addr;
	fd = accept(rsp.fserv, (struct sockaddr *)&sock_addr, &len);

	if (fd < 0) {
		if (errno == EWOULDBLOCK || errno == EAGAIN) {
			return;
		}

		fprintf(stderr, "RSP server error creating client: "
		        "closing connection: %s\n", strerror(errno));
		rsp_close_client();
		rsp_close_server();
		return;
	}

	/* Close a new incoming client connection if there is one already */
	if (rsp.fcli != -1) {
		fprintf(stderr, "Additional RSP client request refused\n");
		close(fd);
		return;
	}

	/* New client should be non-blocking */
	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		fprintf(stderr, "Unable to get flags for RSP client socket "
		        "%d: %s\n", fd, strerror(errno));
		close(fd);
		return;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		fprintf(stderr, "Unable to set flags for RSP client socket "
		        "%d to 0x%08X: %s\n", fd, flags, strerror(errno));
		close(fd);
		return;
	}

	/*
	 * Turn off Nagel's algorithm for the client socket (do not wait
	 * to fill a packet before sending).
	 */
	optval = 0;
	len = sizeof optval;
	if (setsockopt(fd, rsp.proto_num, TCP_NODELAY, &optval, len) < 0) {
		fprintf(stderr, "Unable to switch off Nagel's algorithm for "
		        "RSP client socket %d: %s\n", fd, strerror(errno));
		close(fd);
		return;
	}

	/* We have a new client socket */
	rsp.fcli = fd;
}

static void rsp_client_request(void)
{
	struct rsp_buf *buf;

	buf = get_packet();

	/* NULL means we hit EOF or link closed for some other reason */
	if (buf == NULL) {
		rsp_close_client();
		return;
	}

	/* Process a limited GDB commands while MCU running */
	if (rsp.mcu->state == AVR_RUNNING) {
		if (buf->data[0] == 0x03) {
			rsp.mcu->state = AVR_STOPPED;
		} else {
			put_str_packet(
			        "O6154677274656e20746f73206f74707064650a0d");
			fprintf(stderr, "Received GDB command 0x%X (%c) "
			        "while MCU running!\n",
			        buf->data[0], buf->data[0]);
		}
		return;
	}

	switch (buf->data[0]) {
	case 0x03:
		fprintf(stderr, "Break command received while MCU stopped\n");
		return;
	case '!':
		/* Extended mode request */
		put_str_packet("OK");
		return;
	case '?':
		/* Report why MCU halted */
		rsp_report_exception();
		return;
	case 'c':
		/* Continue */
		rsp_continue(buf);
		return;
	case 'C':
		/*
		 * Continue with signal.
		 * Ignore signal at the moment and continue as usual.
		 */
		rsp_continue(buf);
		return;
	case 'D':
		/* Detach GDB */
		put_str_packet("OK");
		rsp_close_client();
		return;
	case 'g':
		rsp_read_all_regs();
		return;
	case 'G':
		rsp_write_all_regs(buf);
		return;
	case 'H':
		/*
		 * Set a thread number for subsequent operations.
		 * Send OK to ignore silently. */
		put_str_packet("OK");
		return;
	case 'k':
		/* Kill request. Terminate simulation. */
		rsp.mcu->state = AVR_MSIM_STOP;
		return;
	case 'm':
		/* Read memory (symbolic) */
		rsp_read_mem(buf);
		return;
	case 'M':
		/* Write memory (symbolic) */
		rsp_write_mem(buf);
		return;
	case 'p':
		rsp_read_reg(buf);
		return;
	case 'P':
		rsp_write_reg(buf);
		return;
	case 'q':
		/* One of query packets */
		rsp_query(buf);
		return;
	case 'R':
		/* Restart the MCU program */
		rsp_restart();
		return;
	case 's':
		rsp_step(buf);
		return;
	case 'S':
		/* Ignore signal and perform a step as usual */
		rsp_step(buf);
		return;
	case 'v':
		/* One of execution control packets */
		rsp_vpkt(buf);
		return;
	case 'X':
		/* Write memory (binary) */
		rsp_write_mem_bin(buf);
		return;
	case 'z':
		/* Remove a breakpoint/watchpoint */
		rsp_remove_matchpoint(buf);
		return;
	case 'Z':
		/* Insert a breakpoint/watchpoint */
		rsp_insert_matchpoint(buf);
		return;
	default:
		/* Unknown commands are ignored */
		fprintf(stderr, "Unknown RSP request: %s\n", buf->data);
		return;
	}
}

static void rsp_read_reg(struct rsp_buf *buf)
{
	int regn;
	char val[REG_BUF_MAX];

	if (sscanf(buf->data, "p%x", &regn) != 1) {
		fprintf(stderr, "Failed to recognize RSP read register "
		        "command: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}

	val[0] = 0;
	if (!read_reg(regn, val)) {
		fprintf(stderr, "Unknown register %u, empty response "
		        "will be returned\n", regn);
	}
	put_str_packet(val);
}

static void rsp_write_reg(struct rsp_buf *buf)
{
	int regn;
	char val[REG_BUF_MAX];

	if (sscanf(buf->data, "P%x=%8s", &regn, val) != 2) {
		fprintf(stderr, "Failed to recognize RSP write register "
		        "command: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}
	write_reg(regn, val);
	put_str_packet("OK");
}

static void rsp_insert_matchpoint(struct rsp_buf *buf)
{
	enum mp_type type;
	unsigned long addr;
	int len;
	unsigned char llsb, lmsb, hlsb, hmsb;
	unsigned short inst;
	struct MSIM_AVR *mcu;

	mcu = rsp.mcu;
	if (sscanf(buf->data, "Z%1d,%lx,%1d", &type, &addr, &len) != 3) {
		fprintf(stderr, "RSP matchpoint insertion request not "
		        "recognized: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}

	if (len != 2) {
		fprintf(stderr, "RSP matchpoint length %d is not valid: "
		        "2 assumed\n", len);
		len = 2;
	}

	switch (type) {
	case BP_SOFTWARE:
		/*
		 * This is the only type of matchpoints to be supported in
		 * this minimal implementation. Insertion of a breakpoint at
		 * the same location twice won't make any change.
		 */
		llsb = (unsigned char) mcu->pm[addr];
		lmsb = (unsigned char) mcu->pm[addr+1];
		inst = (unsigned short) (llsb | (lmsb << 8));
		if (inst == BREAK) {
			fprintf(stderr, "BREAK is already at 0x%8lX, "
			        "ignoring\n", addr);
			put_str_packet("OK");
			return;
		}

		mcu->pm[addr] = BREAK_LOW;
		mcu->pm[addr+1] = BREAK_HIGH;
		mcu->mpm[addr] = llsb;
		mcu->mpm[addr+1] = lmsb;
		if (MSIM_Is32(inst)) {
			hlsb = (unsigned char) mcu->pm[addr+2];
			hmsb = (unsigned char) mcu->pm[addr+3];
			mcu->pm[addr+2] = 0;
			mcu->pm[addr+3] = 0;
			mcu->mpm[addr+2] = hlsb;
			mcu->mpm[addr+3] = hmsb;
		}
		put_str_packet("OK");
		break;
	default:
		fprintf(stderr, "RSP matchpoint type %d is not "
		        "supported\n", type);
		put_str_packet("");
		break;
	}
}

static void rsp_remove_matchpoint(struct rsp_buf *buf)
{
	enum mp_type type;
	unsigned long addr;
	int len;
	unsigned char llsb, lmsb, hlsb, hmsb;
	unsigned short inst;
	struct MSIM_AVR *mcu;

	mcu = rsp.mcu;
	if (sscanf(buf->data, "z%1d,%lx,%1d", &type, &addr, &len) != 3) {
		fprintf(stderr, "RSP matchpoint insertion request not "
		        "recognized: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}

	if (len != 2) {
		fprintf(stderr, "RSP matchpoint length %d is not valid: "
		        "2 assumed\n", len);
		len = 2;
	}

	switch (type) {
	case BP_SOFTWARE:
		/*
		 * This is the only type of matchpoints to be supported in
		 * this minimal implementation. Double check if breakpoint
		 * exists at the given address.
		 */
		llsb = (unsigned char) mcu->pm[addr];
		lmsb = (unsigned char) mcu->pm[addr+1];
		inst = (unsigned short) (llsb | (lmsb << 8));
		if (inst != BREAK) {
			fprintf(stderr, "There is no BREAK at 0x%8lX "
			        "address, ignoring\n", addr);
			put_str_packet("E01");
			return;
		}

		llsb = (unsigned char) mcu->mpm[addr];
		lmsb = (unsigned char) mcu->mpm[addr+1];
		mcu->pm[addr] = llsb;
		mcu->pm[addr+1] = lmsb;

		inst = (unsigned short) (llsb | (lmsb << 8));
		if (MSIM_Is32(inst)) {
			hlsb = (unsigned char) mcu->mpm[addr+2];
			hmsb = (unsigned char) mcu->mpm[addr+3];
			mcu->pm[addr+2] = hlsb;
			mcu->pm[addr+3] = hmsb;
		}
		put_str_packet("OK");
		break;
	default:
		fprintf(stderr, "RSP matchpoint type %d is not "
		        "supported\n", type);
		put_str_packet("");
		break;
	}
}

static struct rsp_buf *get_packet(void)
{
	static struct rsp_buf buf;

	/* Keep getting packets, until one is found with a valid checksum */
	while (1) {
		unsigned char checksum;
		unsigned long count;		/* Index into the buffer */
		char ch;			/* Current character */

		/*
		 * Wait around for the start character ('$'). Ignore
		 * all other characters
		 */
		ch = (char)get_rsp_char();
		while (ch != '$') {
			if (ch == -1) {
				return  NULL;
			}

			/*
			 * 0x03 is a special case, an out-of-band break
			 * when running
			 */
			if (ch == 0x03) {
				buf.data[0] = ch;
				buf.len = 1;
				return &buf;
			}
			ch = (char)get_rsp_char();
		}

		/* Read until a '#' or end of buffer is found */
		checksum = 0;
		count = 0;
		while (count < GDB_BUF_MAX-1) {
			ch = (char)get_rsp_char();

			/* Check for connection failure */
			if (ch == -1) {
				return  NULL;
			}
			/*
			 * If we hit a start of line char begin
			 * all over again
			 */
			if (ch == '$') {
				checksum = 0;
				count = 0;
				continue;
			}

			/* Break out if we get the end of line char */
			if (ch == '#') {
				break;
			}
			/*
			 * Update the checksum and add the char to
			 * the buffer
			 */
			checksum = checksum + (unsigned char)ch;
			buf.data[count] = (char)ch;
			count++;
		}

		/*
		 * Mark the end of the buffer with EOS - it's convenient
		 * for non-binary data to be valid strings.
		 */
		buf.data[count] = 0;
		buf.len = count;

		/*
		 * If we have a valid end of packet char, validate
		 * the checksum
		 */
		if (ch == '#') {
			unsigned char xmitcsum;

			ch = (char)get_rsp_char();
			if (ch == -1) {
				return NULL;
			}

			xmitcsum = (unsigned char)(hex(ch)<<4);

			ch = (char)get_rsp_char();
			if (ch == -1) {
				return  NULL;
			}

			xmitcsum += hex(ch);

			/*
			 * If the checksums don't match print a warning,
			 * and put the negative ack back to the client.
			 * Otherwise put a positive ack.
			 */
			if (checksum != xmitcsum) {
				fprintf(stderr, "Warning: Bad RSP "
				        "checksum: Computed 0x%02X, "
				        "received 0x%02X\n",
				        checksum, xmitcsum);
				put_rsp_char('-');
			} else {
				put_rsp_char('+');
				break;
			}
		} else {
			fprintf(stderr, "Warning: RSP packet overran "
			        "buffer\n");
		}
	}

	return &buf;
}

static int get_rsp_char(void)
{
	unsigned char c;
	ssize_t bytes;

	if (rsp.fcli == -1) {
		fprintf(stderr, "Attempt to read from unopened RSP "
		        "client: Ignored\n");
		return  -1;
	}

	/*
	 * Read until successful (we retry after interrupts) or
	 * catastrophic failure.
	 */
	while (1) {
		bytes = read(rsp.fcli, &c, sizeof c);

		if (bytes == sizeof c) {
			return c&0xFF;
		}

		if (bytes == -1) {
			/* Error: only allow interrupts or would block */
			if (errno == EAGAIN || errno == EINTR) {
				continue;
			}

			fprintf(stderr, "Failed to read from RSP client: "
			        "Closing client connection: %s\n",
			        strerror(errno));
			rsp_close_client();
			return -1;
		} else {
			rsp_close_client();
			return -1;
		}
	}
}

static void put_rsp_char(char c)
{
	if (rsp.fcli == -1) {
		fprintf(stderr, "Attempt to write '%c' to unopened RSP "
		        "client: Ignored\n", c);
		return;
	}

	/*
	 * Write until successful (we retry after interrupts) or
	 * catastrophic failure.
	 */
	while (1) {
		switch (write(rsp.fcli, &c, sizeof c)) {
		case -1:
			/* Error: only allow interrupts or would block */
			if (errno == EAGAIN || errno == EINTR) {
				break;
			}

			fprintf(stderr, "Failed to write to RSP client: "
			        "Closing client connection: %s\n",
			        strerror(errno));
			rsp_close_client();
			return;
		case 0:
			break;		/* Nothing written! Try again */
		default:
			return;		/* Success, we can return */
		}
	}
}

static int hex(int c)
{
	return ((c >= 'a') && (c <= 'f')) ? c - 'a' + 10 :
	       ((c >= '0') && (c <= '9')) ? c - '0' :
	       ((c >= 'A') && (c <= 'F')) ? c - 'A' + 10 : -1;
}

/*
 * Convert a hex digit string to a register value.
 * The supplied 8 digit hex string is converted to a 32-bit value according
 * the target endianness.
 *
 * buf		The buffer with the hex string
 * dign		Number of digits in buffer to convert
 * return	The value to convert
 */
static unsigned long hex2reg(char *buf, const unsigned long dign)
{
	unsigned int n;			/* Counter for digits */
	unsigned long val;		/* The result */
	int nyb_shift;

	val = 0;
	for (n = 0; n < dign; n++) {
		nyb_shift = (int)(((dign-1)*4)-(n*4));
		val |= (unsigned long)(hex(buf[n]) << nyb_shift);
	}
	return val;
}

static void put_str_packet(const char *str)
{
	struct rsp_buf buf;
	unsigned long len = strlen(str);

	/*
	 * Construct the packet to send, so long as string is not too big,
	 * otherwise truncate. Add EOS at the end for convenient debug
	 * printout
	 */
	if (len >= GDB_BUF_MAX) {
		fprintf(stderr, "Warning: String %s too large for RSP "
		        "packet: truncated\n", str);
		len = GDB_BUF_MAX - 1;
	}

	strncpy(buf.data, str, len);
	buf.data[len] = 0;
	buf.len = len;

	put_packet(&buf);
}

static void put_packet(struct rsp_buf *buf)
{
	int ch;

	/*
	 * Construct $<packet info>#<checksum>. Repeat until the GDB client
	 * acknowledges satisfactory receipt.
	 */
	do {
		unsigned char checksum = 0;
		unsigned int count = 0;

		put_rsp_char('$');		/* Start of the packet */

		/* Body of the packet */
		for (count = 0; count < buf->len; count++) {
			char c = buf->data[count];

			/* Check for escaped chars */
			if (('$' == c) || ('#' == c) ||
			                ('*' == c) || ('}' == c)) {
				c ^= 0x20;
				checksum += (unsigned char)'}';
				put_rsp_char('}');
			}
			checksum += c;
			put_rsp_char(c);
		}

		put_rsp_char ('#');		/* End char */
		/* Computed checksum */
		put_rsp_char(hexchars[checksum >> 4]);
		put_rsp_char(hexchars[checksum % 16]);

		/* Check for ack of connection failure */
		ch = get_rsp_char();
		if (ch == -1) {
			return;			/* Fail the put silently. */
		}
	} while (ch != '+');
}

static void rsp_report_exception(void)
{
	struct rsp_buf buf;

	/* Construct a signal received packet */
	buf.data[0] = 'S';
	buf.data[1] = hexchars[rsp.sigval >> 4];
	buf.data[2] = hexchars[rsp.sigval % 16];
	buf.data[3] = 0;
	buf.len = strlen(buf.data);

	put_packet (&buf);
}

static void rsp_continue(struct rsp_buf *buf)
{
	unsigned long addr;

	if (sscanf(buf->data, "c%lx", &addr) == 1) {
		rsp.mcu->pc = addr;
	}
	rsp.mcu->state = AVR_RUNNING;
	rsp.client_waiting = 1;
}

static void rsp_query(struct rsp_buf *buf)
{
	if (!strcmp("qC", buf->data)) {
		/*
		 * Return the current thread ID (unsigned hex). A null
		 * response indicates to use the previously selected thread.
		 * Since we do not support a thread concept, this is the
		 * appropriate response.
		 */
		put_str_packet("");
	} else if (!strcmp("qOffsets", buf->data)) {
		/* Report any relocation */
		put_str_packet("Text=0;Data=0;Bss=0");
	} else if (!strncmp("qSupported", buf->data, strlen ("qSupported"))) {
		/*
		 * Report a list of the features we support. For now we just
		 * ignore any supplied specific feature queries, but in the
		 * future these may be supported as well. Note that the
		 * packet size allows for 'G' + all the registers sent to us,
		 * or a reply to 'g' with all the registers and an EOS so
		 * the buffer is a well formed string.
		 */
		char reply[GDB_BUF_MAX];

		sprintf(reply, "PacketSize=%X", GDB_BUF_MAX);
		put_str_packet(reply);
	} else if (!strncmp("qSymbol:", buf->data, strlen("qSymbol:"))) {
		/*
		 * Offer to look up symbols. Nothing we want (for now).
		 * TODO. This just ignores any replies to symbols we
		 * looked up, but we didn't want to do that anyway!
		 */
		put_str_packet("OK");
	} else if (!strncmp("qTStatus", buf->data, strlen("qTStatus"))) {
		/* Trace feature is not supported */
		put_str_packet("");
	} else if (!strncmp("qfThreadInfo", buf->data,
	                    strlen("qfThreadInfo"))) {
		/* Return info about active threads. */
		put_str_packet("m-1");
	} else if (!strncmp("qAttached", buf->data, strlen("qAttached"))) {
		put_str_packet("");
	} else if (!strncmp("qsThreadInfo", buf->data,
	                    strlen("qsThreadInfo"))) {
		/* Return info about more active threads */
		put_str_packet("l");
	} else {
		fprintf(stderr, "Unrecognized RSP query\n");
	}
}

static void rsp_vpkt(struct rsp_buf *buf)
{
	if (!strncmp("vAttach;", buf->data, strlen("vAttach;"))) {
		/*
		 * Attaching is a null action, since we have no other
		 * process. We just return a stop packet (using TRAP)
		 * to indicate we are stopped.
		 */
		put_str_packet("S05");
		return;
	} else if (!strcmp("vCont?", buf->data)) {
		/* For now we don't support this. */
		put_str_packet("");
		return;
	} else if (!strncmp("vCont", buf->data, strlen("vCont"))) {
		/*
		 * This shouldn't happen, because we've reported non-support
		 * via vCont? above */
		fprintf(stderr, "RSP vCont is not supported: ignored\n");
		return;
	} else if (!strncmp("vRun;", buf->data, strlen("vRun;"))) {
		/* We shouldn't be given any args, but check for this */
		if (buf->len > strlen("vRun;")) {
			fprintf(stderr, "Unexpected arguments to RSP vRun "
			        "command: ignored\n");
		}

		/*
		 * Restart the current program. However unlike a "R" packet,
		 * "vRun" should behave as though it has just stopped.
		 * We use signal 5 (TRAP).
		 */
		rsp_restart();
		put_str_packet("S05");
	} else if (!strncmp("vKill;", buf->data, strlen("vKill;"))) {
		/* Restart MCU in stopped state on kill request */
		rsp_restart();
		put_str_packet("OK");
	} else {
		fprintf(stderr, "Unknown RSP 'v' packet type %s: ignored\n",
		        buf->data);
		put_str_packet("E01");
		return;
	}
}

static void rsp_restart(void)
{
	rsp.mcu->pc = rsp.mcu->intr->reset_pc;
	rsp.mcu->state = AVR_STOPPED;
}

static size_t read_reg(int n, char *buf)
{
	/*
	 * This function reads registers in order required to reply to
	 * GDB client. Remember that N is not an index in this case!
	 */
	if (n >= 0 && n <= 31) {	/* GPR0..31 */
		sprintf(buf, "%02X", rsp.mcu->dm[n]);
		return strlen(buf);
	}

	switch (n) {
	case 32:			/* SREG */
		sprintf(buf, "%02X", *rsp.mcu->sreg);
		break;
	case 33:			/* SPH and SPL */
		sprintf(buf, "%02X%02X", *rsp.mcu->spl, *rsp.mcu->sph);
		break;
	case 34:			/* PC */
		sprintf(buf, "%02X%02X%02X00",
		        (unsigned char)(rsp.mcu->pc&0xFF),
		        (unsigned char)((rsp.mcu->pc>>8)&0xFF),
		        (unsigned char)((rsp.mcu->pc>>16)&0xFF));
		break;
	}
	return strlen(buf);
}

static void write_reg(int n, char *buf)
{
	unsigned long v;

	if (n >= 0 && n <= 31) {	/* GPR0..31 */
		rsp.mcu->dm[n] = (unsigned char)hex2reg(buf, 2);
		return;
	}

	switch (n) {
	case 32:			/* SREG */
		*rsp.mcu->sreg = (unsigned char)hex2reg(buf, 2);
		break;
	case 33:			/* SPH and SPL */
		v = hex2reg(buf, 4);
		*rsp.mcu->sph = (unsigned char)(v&0xFF);
		*rsp.mcu->spl = (unsigned char)((v>>8)&0xFF);
		break;
	case 34:			/* PC */
		rsp.mcu->pc = hex2reg(buf, 8);
		break;
	}
	return;
}

static void rsp_read_all_regs(void)
{
	char reply[GDB_BUF_MAX];
	char *rep;
	int i;

	rep = reply;
	for (i = 0; i < 35; i++) {
		rep += read_reg(i, rep);
	}
	*rep = 0;
	put_str_packet(reply);
}

static void rsp_write_all_regs(struct rsp_buf *buf)
{
	unsigned int n, off;
	unsigned long v;

	off = 0;
	for (n = 0; n < 35; n++) {
		if (n <= 31) { /* General purpose regs 0..31 */
			rsp.mcu->dm[n] = (unsigned char)
			                 hex2reg(buf->data+off, 2);
			off += 2;
			continue;
		}

		switch (n) {
		case 32: /* SREG */
			*rsp.mcu->sreg = (unsigned char)
			                 hex2reg(buf->data+off, 2);
			off += 2;
			break;
		case 33: /* SPH and SPL */
			v = hex2reg(buf->data+off, 4);
			off += 4;
			*rsp.mcu->sph = (unsigned char)(v&0xFF);
			*rsp.mcu->spl = (unsigned char)((v>>8)&0xFF);
			break;
		case 34: /* PC */
			rsp.mcu->pc = hex2reg(buf->data+off, 8);
			off += 8;
			break;
		}
	}
	put_str_packet("OK");
}

static void rsp_read_mem(struct rsp_buf *buf)
{
	unsigned int addr, len, i;
	unsigned char c;
	unsigned char *src;

	if (sscanf(buf->data, "m%x,%x:", &addr, &len) != 2) {
		fprintf(stderr, "Failed to recognize RSP read memory "
		        "command: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}
	addr &= 0xFFFFFF;

	/* Make sure we won't overflow the buffer (2 chars per byte) */
	if ((len*2) >= GDB_BUF_MAX) {
		fprintf(stderr, "Memory read %s too large for RSP packet: "
		        "requested chunk will be truncated\n", buf->data);
		len = (GDB_BUF_MAX-1)/2;
	}

	/* Find a memory to read from */
	if (addr < rsp.mcu->flashend) {
		src = rsp.mcu->pm + addr;
	} else if ((addr >= 0x800000) &&
	                ((addr-0x800000) <= rsp.mcu->ramend)) {
		src = rsp.mcu->dm + addr - 0x800000;
	} else if (addr == (0x800000 + rsp.mcu->ramend+1) && len == 2) {
		put_str_packet("0000");
		return;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		/*src = rsp.mcu->ee + addr - 0x810000;*/
		put_str_packet("E01");
		return;
	} else {
		fprintf(stderr, "Unable to read memory %08X, %08X\n",
		        addr, len);
		put_str_packet("E01");
		return;
	}

	/* Do the memory read into a temporary buffer */
	for (i = 0; i < len; i++) {
		c = src[i];
		buf->data[i*2] = hexchars[c>>4];
		buf->data[i*2+1] = hexchars[c&0xF];
	}
	buf->data[i*2] = 0;
	buf->len = strlen(buf->data);
	put_packet(buf);
}

static void rsp_write_mem(struct rsp_buf *buf)
{
	static unsigned char tmpbuf[GDB_BUF_MAX];

	unsigned long addr, datlen;
	unsigned int len;
	char *symdat;
	unsigned int off;
	unsigned char mnyb, lnyb;
	unsigned char *dest;

	if (sscanf(buf->data, "M%lx,%x:", &addr, &len) != 2) {
		fprintf(stderr, "Failed to recognize RSP write memory "
		        "command: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}

	/*
	 * Find the start of the data and check there is the amount
	 * we expect.
	 */
	symdat = memchr((const void *)buf->data, ':', GDB_BUF_MAX);
	symdat++;
	datlen = buf->len - (unsigned long)(symdat - buf->data);

	/* Sanity check */
	if (datlen != len*2) {
		fprintf(stderr, "Write of %d digits requested, but %lu "
		        "digits supplied: request ignored\n", len*2, datlen);
		put_str_packet("E01");
		return;
	}

	/* Write bytes to memory */
	for (off = 0; off < len; off++) {
		mnyb = (unsigned char)hex(symdat[off*2]);
		lnyb = (unsigned char)hex(symdat[off*2+1]);
		tmpbuf[off] = ((mnyb<<4)&0xF0)|(lnyb&0x0F);
	}
	/* Find a memory to write to */
	if (addr < rsp.mcu->flashend) {
		dest = rsp.mcu->pm + addr;
	} else if ((addr >= 0x800000) &&
	                ((addr-0x800000) <= rsp.mcu->ramend)) {
		dest = rsp.mcu->dm + addr - 0x800000;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		/*dest = rsp.mcu->ee + addr - 0x810000;*/
		put_str_packet("E01");
		return;
	} else {
		fprintf(stderr, "Unable to write memory %08lX, %08X\n",
		        addr, len);
		put_str_packet("E01");
		return;
	}
	memcpy(dest, tmpbuf, len);
	put_str_packet("OK");
}

static void rsp_write_mem_bin(struct rsp_buf *buf)
{
	unsigned long addr, datlen, len;
	char *bindat;
	unsigned int off;
	unsigned char *dest;

	if (sscanf(buf->data, "X%lx,%lx:", &addr, &len) != 2) {
		fprintf(stderr, "Failed to recognize RSP write memory "
		        "command: %s\n", buf->data);
		put_str_packet("E01");
		return;
	}

	/* Find the start of the data and "unescape" it. */
	bindat = memchr((const void *)buf->data, ':', GDB_BUF_MAX);
	bindat++;
	off = (unsigned int)(bindat - buf->data);
	datlen = rsp_unescape(bindat, buf->len - off);

	/* Sanity check */
	if (datlen != len) {
		unsigned long minlen = len < datlen ? len : datlen;

		fprintf(stderr, "Write of %lu bytes requested, but %lu bytes "
		        "supplied. %lu will be written\n",
		        len, datlen, minlen);
		len = minlen;
	}
	/* Find a memory to write to */
	if (addr < rsp.mcu->flashend) {
		dest = rsp.mcu->pm + addr;
	} else if ((addr >= 0x800000) &&
	                ((addr-0x800000) <= rsp.mcu->ramend)) {
		dest = rsp.mcu->dm + addr - 0x800000;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		/*dest = rsp.mcu->ee + addr - 0x810000;*/
		put_str_packet("E01");
		return;
	} else {
		fprintf(stderr, "Unable to write memory %08lX, %08lX\n",
		        addr, len);
		put_str_packet("E01");
		return;
	}
	memcpy(dest, bindat, len);
	put_str_packet("OK");
}

/*
 * "Unescape" RSP binary data.
 * '#', '$' and '}' are escaped by preceding them by '}' and oring with 0x20.
 * This function reverses that, modifying the data in place.
 *
 * @param[in] data The array of bytes to convert
 * @para[in] len The number of bytes to be converted
 * @return The number of bytes AFTER conversion
 */
static unsigned long rsp_unescape(char *data, unsigned long len)
{
	unsigned long from_off = 0;		/* Offset to source char */
	unsigned long to_off = 0;		/* Offset to dest char */

	while (from_off < len) {
		/* Is it escaped? */
		if (data[from_off] == '}') {
			from_off++;
			data[to_off] = data[from_off] ^ 0x20;
		} else {
			data[to_off] = data[from_off];
		}
		from_off++;
		to_off++;
	}
	return  to_off;
}

static void rsp_step(struct rsp_buf *buf)
{
	rsp.mcu->state = AVR_MSIM_STEP;
	rsp.client_waiting = 1;
}

#else /* MSIM_POSIX is not defined */

void MSIM_RSPInit(struct MSIM_AVR *mcu, int portn)
{
}

int MSIM_RSPHandle(void)
{
	return 0;
}

void MSIM_RSPClose(void)
{
}
#endif /* MSIM_POSIX */
