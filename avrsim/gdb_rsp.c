/*
 * AVRSim - Simulator for AVR microcontrollers.
 * This software is a part of MCUSim, interactive simulator for
 * microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mcusim/avr/sim/sim.h"
#include "mcusim/avr/sim/gdb_rsp.h"

#ifdef MSIM_POSIX
#	include <netdb.h>
#	include <sys/socket.h>
#	include <string.h>
#	include <fcntl.h>
#	include <unistd.h>
#	include <errno.h>
#	include <poll.h>
#	include <netinet/tcp.h>

#define AVRSIM_RSP_PROTOCOL		"tcp"

#define GDB_BUF_MAX			(16*1024)

struct rsp_state {
	char client_waiting;
	struct MSIM_AVR *mcu;		/* MCU instance */
	int proto_num;
	int fserv;			/* FD for incoming connections */
	int fcli;			/* FD for talking to GDB client */
	int sigval;			/* GDB signal for any exception */
	MSIM_AVRFlashAddr_t start_addr;	/* Start of last run */
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
static void put_str_packet(const char *str);
static void rsp_report_exception(void);
static void rsp_continue(struct rsp_buf *buf);
static void rsp_query(struct rsp_buf *buf);
static void rsp_vpkt(struct rsp_buf *buf);
static void rsp_restart(void);
static void rsp_read_all_regs(void);
static void rsp_write_all_regs(struct rsp_buf *buf);
static void rsp_read_mem(struct rsp_buf *buf);
static void rsp_step(struct rsp_buf *buf);

static size_t read_reg(int n, char *buf);
static void write_reg(int n, unsigned char *val);

void MSIM_RSPInit(struct MSIM_AVR *mcu, int portn)
{
	struct protoent *protocol;	/* Protocol entry */
	struct hostent *host;		/* Host entry */
	struct sockaddr_in sock_addr;	/* Socket address */
	int optval;			/* Socket options */
	int flags; 			/* Socket flags */
	char name[256];			/* Hostname */

	/* Reset GDB RSP state */
	rsp.client_waiting = 0;		/* GDB client is not waiting */
	rsp.mcu = mcu;			/* MCU instance */
	rsp.proto_num = -1;		/* i.e. invalid */
	rsp.fserv = -1;			/* i.e. invalid */
	rsp.fcli = -1;			/* i.e. invalid */
	rsp.sigval = 0;			/* No exceptions */
	rsp.start_addr = mcu->reset_pc;	/* Reset PC by default */

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
				"server socket %d: %s\n",
				rsp.fserv, strerror(errno));
		rsp_close_server();
		return;
	}

	/* Server should be non-blocking */
	flags = fcntl(rsp.fserv, F_GETFL);
	if (flags < 0) {
		fprintf(stderr, "Unable to get flags for RSP server "
				"socket %d: %s\n",
				rsp.fserv, strerror(errno));
		rsp_close_server();
		return;
	}
	flags |= O_NONBLOCK;
	if (fcntl(rsp.fserv, F_SETFL, flags) < 0) {
		fprintf(stderr, "Unable to set flags for RSP server "
				"socket %d to 0x%08X: %s\n",
				rsp.fserv, flags, strerror(errno));
		rsp_close_server();
		return;
	}

	/* Find our hostname */
	/*
	if (gethostname(name, sizeof name) < 0) {
		fprintf(stderr, "Unable to get hostname for RSP server: %s\n",
				strerror(errno));
		rsp_close_server();
		return;
	}
	 */
	strncpy(name, "localhost", sizeof name);

	/* Find our host entry */
	host = gethostbyname(name);
	if (host == NULL) {
		fprintf(stderr, "Unable to get host entry for RSP server "
				"by '%s' name: %s\n", name, strerror(errno));
		rsp_close_server();
		return;
	}

	/* Bind socket to the appropriate address */
	memset(&sock_addr, 0, sizeof sock_addr);
	sock_addr.sin_family = host->h_addrtype;
	sock_addr.sin_port = htons(portn);
	if (bind(rsp.fserv, (struct sockaddr *)&sock_addr,
		 sizeof sock_addr) < 0) {
		fprintf(stderr, "Unable to bind RSP server socket %d to "
				"port %d: %s\n", rsp.fserv, portn,
				strerror(errno));
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
			if (errno == EINTR)
				/* Ignore received signal while polling */
				break;
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
						"0x%08X: closing server "
						"connection\n",
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
		if (errno == EINTR)
			break;
		fprintf(stderr, "Poll for RSP client failed: "
				"closing server connection: %s\n",
				strerror(errno));
		rsp_close_client();
		rsp_close_server();
		return -1;
	case 0:
		/* Timeout. This should not happen. */
		fprintf(stderr, "Unexpected RSP client poll "
				"timeout\n");
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
					"0x%08X: closing client "
					"connection\n",
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
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return;

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
				"%d to 0x%08X: %s\n",
				fd, flags, strerror(errno));
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
				"RSP client socket %d: %s\n",
				fd, strerror(errno));
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

	printf("GDB client request: %s\n", buf->data);

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
		/*rsp_write_mem(buf);*/
		return;
	case 'p':
		/*rsp_read_reg(buf);*/
		return;
	case 'P':
		/*rsp_write_reg(buf);*/
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
		/*rsp_write_mem_bin(buf);*/
		return;
	case 'z':
		/* Remove a breakpoint/watchpoint */
		/*rsp_remove_matchpoint(buf);*/
		return;
	case 'Z':
		/* Insert a breakpoint/watchpoint */
		/*rsp_insert_matchpoint(buf);*/
		return;
	default:
		/* Unknown commands are ignored */
		fprintf(stderr, "Unknown RSP request: %s\n", buf->data);
		return;
	}
}

static struct rsp_buf *get_packet(void)
{
	static struct rsp_buf buf;

	/* Keep getting packets, until one is found with a valid checksum */
	while (1) {
		unsigned char checksum;
		int count;		/* Index into the buffer */
		int ch;			/* Current character */

		/*
		 * Wait around for the start character ('$'). Ignore
		 * all other characters
		 */
		ch = get_rsp_char();
		while (ch != '$') {
			if (ch == -1)
				return  NULL;

	  		/*
			 * 0x03 is a special case, an out-of-band break
			 * when running
			 */
			if (ch == 0x03) {
				buf.data[0] = ch;
				buf.len = 1;
				return &buf;
			}
			ch = get_rsp_char();
		}

		/* Read until a '#' or end of buffer is found */
		checksum = 0;
		count = 0;
		while (count < GDB_BUF_MAX-1) {
			ch = get_rsp_char();

			/* Check for connection failure */
			if (ch == -1)
				return  NULL;
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
			if (ch == '#')
				break;
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

			ch = get_rsp_char();
			if (ch == -1)
				return  NULL;

			xmitcsum = hex(ch) << 4;

			ch = get_rsp_char();
			if (ch == -1)
				return  NULL;

			xmitcsum += hex(ch);

			/*
			 * If the checksums don't match print a warning,
			 * and put the negative ack back to the client.
			 * Otherwise put a positive ack.
			 */
			if (checksum != xmitcsum) {
				fprintf(stderr, "Warning: Bad RSP "
						"checksum: Computed "
						"0x%02X, received 0x%02X\n",
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
		switch(read(rsp.fcli, &c, sizeof c)) {
		case -1:
			/* Error: only allow interrupts or would block */
			if (errno == EAGAIN || errno == EINTR)
				break;

			fprintf(stderr, "Failed to read from RSP client: "
					"Closing client connection: %s\n",
					strerror(errno));
			rsp_close_client();
			return -1;
		case 0: /* EOF */
			rsp_close_client();
			return -1;
		default:
			return c&0xff;
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
			if (errno == EAGAIN || errno == EINTR)
				break;

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

static void put_str_packet(const char *str)
{
	struct rsp_buf buf;
	int len = strlen(str);

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

	printf("Put packet: %s\n", buf.data);
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
			char ch = buf->data[count];

			/* Check for escaped chars */
			if (('$' == ch) || ('#' == ch) ||
			    ('*' == ch) || ('}' == ch)) {
				ch ^= 0x20;
				checksum += (unsigned char)'}';
				put_rsp_char('}');
			}

			checksum += ch;
			put_rsp_char(ch);
		}

		put_rsp_char ('#');		/* End char */
		/* Computed checksum */
		put_rsp_char(hexchars[checksum >> 4]);
		put_rsp_char(hexchars[checksum % 16]);

		/* Check for ack of connection failure */
		ch = get_rsp_char();
		if (ch == -1)
			return;			/* Fail the put silently. */
	} while ('+' != ch);
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

	if (strncmp(buf->data, "c", 2)) {
		if (sscanf(buf->data, "c%lx", &addr) != 1) {
			fprintf(stderr, "RSP continue address %s "
					"not recognized: ignored\n",
					buf->data);
		} else {
			rsp.mcu->pc = addr;
			rsp.mcu->state = AVR_RUNNING;
			rsp.client_waiting = 1;
		}
	}
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
		if (buf->len > strlen("vRun;"))
			fprintf(stderr, "Unexpected arguments to RSP vRun "
					"command: ignored\n");

		/*
		 * Restart the current program. However unlike a "R" packet,
		 * "vRun" should behave as though it has just stopped.
		 * We use signal 5 (TRAP).
		 */
		rsp_restart();
		put_str_packet("S05");
	} else {
		fprintf(stderr, "Unknown RSP 'v' packet type %s: ignored\n",
				buf->data);
		put_str_packet("E01");
		return;
	}
}

static void rsp_restart(void)
{
	rsp.mcu->pc = rsp.mcu->reset_pc;
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

static void write_reg(int n, unsigned char *val)
{
	/*
	 * This function writes registers in order required to reply to
	 * GDB client. Remember that N is not an index in this case!
	 */
	if (n >= 0 && n <= 31) {	/* GPR0..31 */
		rsp.mcu->dm[n] = val[0];
		return;
	}

	switch (n) {
	case 32:			/* SREG */
		*rsp.mcu->sreg = val[0];
		break;
	case 33:			/* SPH and SPL */
		*rsp.mcu->spl = val[0];
		*rsp.mcu->sph = val[1];
		break;
	case 34:			/* PC */
		rsp.mcu->pc = val[0] | (val[1]<<8) | (val[2]<<16) | (0<<24);
		break;
	}
}

static void rsp_read_all_regs(void)
{
	char reply[GDB_BUF_MAX];
	char *rep;
	int i;

	rep = reply;
	for (i = 0; i < 35; i++)
		rep += read_reg(i, rep);
	*rep = 0;
	put_str_packet(reply);
}

static void rsp_write_all_regs(struct rsp_buf *buf)
{
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
				"requested chunk will be truncated\n",
				buf->data);
		len = (GDB_BUF_MAX-1)/2;
	}

	/* Find a memory to read from */
	if (addr < rsp.mcu->flashend) {
		src = rsp.mcu->pm + addr;
	} else if ((addr >= 0x800000) &&
		   (rsp.mcu->ramend >= (addr-0x800000))) {
		src = rsp.mcu->dm + addr - 0x800000;
	} else if (addr == (0x800000 + rsp.mcu->ramend+1) && len == 2) {
		put_str_packet("0000");
		return;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		/*src = rsp.mcu->ee;*/
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

static void rsp_step(struct rsp_buf *buf)
{
	rsp.mcu->state = AVR_MSIM_STEP;
	rsp.client_waiting = 1;
}

#else /* MSIM_POSIX is not defined */

void MSIM_RSPInit(int portn)
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
