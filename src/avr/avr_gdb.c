/*
 * This file is part of MCUSim, an XSPICE library with microcontrollers.
 *
 * Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
 *
 * MCUSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MCUSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <netinet/in.h>

#include "mcusim/mcusim.h"
#include "mcusim/log.h"
#include "mcusim/avr/sim/private/macro.h"

#ifndef WITH_POSIX_CYGWIN
	#include <netinet/tcp.h>
#endif

#define AVRSIM_RSP_PROTOCOL		"tcp"
#define BREAK_LOW			0x98
#define BREAK_HIGH			0x95
#define BREAK				((BREAK_HIGH<<8)|BREAK_LOW)
#define GDB_BUF_MAX			(16*1024)
#define REG_BUF_MAX			32

/* Match point type */
enum mp_type {
	BP_SOFTWARE	= 0,		/* Software break point */
	BP_HARDWARE	= 1,		/* Hardware break point */
	WP_WRITE	= 2,		/* Watch point (to write memory) */
	WP_READ		= 3,		/* Watch point (to read read) */
	WP_ACCESS	= 4		/* Watch point (to access memory) */
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

typedef struct rsp_buf {
	char data[GDB_BUF_MAX];
	unsigned long len;
} rsp_buf;

static struct rsp_state rsp;
static const char hexchars[] = "0123456789ABCDEF";

static void		rsp_close_server(void);
static void		rsp_close_client(void);
static void		rsp_server_request(MSIM_AVR *mcu);
static void		rsp_client_request(MSIM_AVR *mcu);
static rsp_buf 	*get_packet(void);
static int		get_rsp_char(void);
static void		put_packet(MSIM_AVR *mcu, rsp_buf *buf);
static void		put_rsp_char(char c);
static void		put_str_packet(MSIM_AVR *mcu, const char *str);
static void		rsp_report_exception(MSIM_AVR *mcu);
static void		rsp_continue(rsp_buf *buf);
static void		rsp_query(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_vpkt(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_restart(void);
static void		rsp_read_all_regs(MSIM_AVR *mcu);
static void		rsp_write_all_regs(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_read_mem(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_write_mem(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_write_mem_bin(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_step(rsp_buf *buf);
static void		rsp_insert_matchpoint(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_remove_matchpoint(MSIM_AVR *mcu, rsp_buf *buf);
static unsigned long	rsp_unescape(char *data, unsigned long len);
static void		rsp_read_reg(MSIM_AVR *mcu, rsp_buf *buf);
static void		rsp_write_reg(MSIM_AVR *mcu, rsp_buf *buf);

static int		hex(int c);
static unsigned long	hex2reg(char *buf, const unsigned long dign);

static size_t		read_reg(int n, char *buf);
static void		write_reg(int n, char *buf);

void
MSIM_AVR_RSPInit(struct MSIM_AVR *mcu, uint16_t portn)
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
	rsp.start_addr = mcu->intr.reset_pc;	/* Reset PC by default */

	protocol = getprotobyname(AVRSIM_RSP_PROTOCOL);
	if (protocol == NULL) {
		snprintf(LOG, LOGSZ, "Unable to load protocol \"%s\": %s",
		         AVRSIM_RSP_PROTOCOL, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		return;
	}

	rsp.proto_num = protocol->p_proto;

	if (portn <= IPPORT_RESERVED) {
		snprintf(LOG, LOGSZ, "Could not use a reserved port: %d, "
		         "should be > %d", portn, IPPORT_RESERVED);
		MSIM_LOG_ERROR(LOG);

		return;
	}

	/* Create a socket using AVRSim RSP protocol */
	rsp.fserv = socket(PF_INET, SOCK_STREAM, protocol->p_proto);
	if (rsp.fserv < 0) {
		snprintf(LOG, LOGSZ, "RSP could not create server socket: %s",
		         strerror(errno));
		MSIM_LOG_ERROR(LOG);

		return;
	}

	/* Set socket to reuse its address */
	optval = 1;
	if (setsockopt(rsp.fserv, SOL_SOCKET, SO_REUSEADDR, &optval,
	                sizeof optval) < 0) {
		snprintf(LOG, LOGSZ, "Could not setup socket to reuse its "
		         "address %d: %s", rsp.fserv, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_server();
		return;
	}

	/* Server should be non-blocking */
	flags = fcntl(rsp.fserv, F_GETFL);
	if (flags < 0) {
		snprintf(LOG, LOGSZ, "Unable to get flags for RSP server "
		         "socket %d: %s", rsp.fserv, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_server();
		return;
	}
	flags |= O_NONBLOCK;
	if (fcntl(rsp.fserv, F_SETFL, flags) < 0) {
		snprintf(LOG, LOGSZ, "Unable to set flags for RSP server "
		         "socket %d to 0x%08X: %s",
		         rsp.fserv, flags, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_server();
		return;
	}

	/* Find our host entry */
	host = gethostbyname("localhost");
	if (host == NULL) {
		snprintf(LOG, LOGSZ, "Unable to get host entry for RSP server "
		         "by localhost name: %s", strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_server();
		return;
	}

	/* Bind socket to the appropriate address */
	memset(&sock_addr, 0, sizeof sock_addr);
	sock_addr.sin_family = (unsigned char)host->h_addrtype;
	sock_addr.sin_port = htons(portn);

	if (bind(rsp.fserv, (struct sockaddr *)&sock_addr,
	                sizeof sock_addr) < 0) {
		snprintf(LOG, LOGSZ, "Unable to bind RSP server socket %d to "
		         "port %d: %s", rsp.fserv, portn, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_server();
		return;
	}

	/*
	 * Listen to the incoming connections from GDB clients (do not allow
	 * more than 1 client to be connected simultaneously!)
	 */
	if (listen(rsp.fserv, 1) < 0) {
		snprintf(LOG, LOGSZ, "Unable to backlog on RSP server socket "
		         "%d to %d: %s", rsp.fserv, 1, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_server();
		return;
	}
}

void
MSIM_AVR_RSPClose(struct MSIM_AVR *mcu)
{
	rsp_close_client();
	rsp_close_server();
}

int
MSIM_AVR_RSPHandle(struct MSIM_AVR *mcu)
{
	struct pollfd fds[2];

	/* Give up if no RSP server port (this should not happen) */
	if (rsp.fserv == -1) {
		MSIM_LOG_ERROR("No open RSP server port");

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

			snprintf(LOG, LOGSZ, "Poll for RSP server failed: "
			         "closing server connection: %s",
			         strerror(errno));
			MSIM_LOG_ERROR(LOG);

			rsp_close_client();
			rsp_close_server();
			return -1;
		case 0:
			/* Timeout. This should not happen. */
			MSIM_LOG_WARN("Unexpected RSP server poll timeout");

			break;
		default:
			if (POLLIN == (fds[0].revents & POLLIN)) {
				rsp_server_request(mcu);
				rsp.client_waiting = 0;
			} else {
				snprintf(LOG, LOGSZ, "RSP server received "
				         "flags 0x%08X: closing server "
				         "connection", fds[0].revents);
				MSIM_LOG_ERROR(LOG);

				rsp_close_client();
				rsp_close_server();
				return -1;
			}
			break;
		}
	}

	/* Response with signal 5 (TRAP exception) any time */
	if (rsp.client_waiting) {
		put_str_packet(mcu, "S05");
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

		snprintf(LOG, LOGSZ, "Poll for RSP client failed: closing "
		         "server connection: %s", strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_client();
		rsp_close_server();
		return -1;
	case 0:
		/* Timeout. This should not happen. */
		MSIM_LOG_ERROR("Unexpected RSP client poll timeout");

		return -1;
	default:
		/* Is there client activity due to input available? */
		if (POLLIN == (fds[0].revents & POLLIN)) {
			rsp_client_request(mcu);
		} else {
			/*
			 * Error leads to closing the client, but not
			 * the server.
			 */
			snprintf(LOG, LOGSZ, "RSP client received flags "
			         "0x%08X: closing client connection",
			         fds[0].revents);
			MSIM_LOG_WARN(LOG);

			rsp_close_client();
		}
		break;
	}
	return 0;
}

static void
rsp_close_server(void)
{
	if (rsp.fserv != -1) {
		close(rsp.fserv);
		rsp.fserv = -1;
	}
}

static void
rsp_close_client(void)
{
	if (rsp.fcli != -1) {
		close(rsp.fcli);
		rsp.fcli = -1;
	}
}

static void
rsp_server_request(struct MSIM_AVR *mcu)
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

		snprintf(LOG, LOGSZ, "error while creating client: "
		         "closing connection: %s", strerror(errno));
		MSIM_LOG_ERROR(LOG);

		rsp_close_client();
		rsp_close_server();
		return;
	}

	/* Close a new incoming client connection if there is one already */
	if (rsp.fcli != -1) {
		snprintf(LOG, LOGSZ, "additional RSP client request refused");
		MSIM_LOG_ERROR(LOG);

		close(fd);
		return;
	}

	/* New client should be a non-blocking one */
	flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		snprintf(LOG, LOGSZ, "unable to get flags for RSP client "
		         "socket %d: %s", fd, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		close(fd);
		return;
	}

	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		snprintf(LOG, LOGSZ, "unable to set flags for RSP client "
		         "socket %d to 0x%08X: %s", fd, flags, strerror(errno));
		MSIM_LOG_ERROR(LOG);

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
		snprintf(LOG, LOGSZ, "unable to switch off Nagel's algorithm "
		         "for RSP client socket %d: %s\n", fd, strerror(errno));
		MSIM_LOG_ERROR(LOG);

		close(fd);
		return;
	}

	/* We have a new client socket */
	rsp.fcli = fd;
}

static void
rsp_client_request(struct MSIM_AVR *mcu)
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
			put_str_packet(mcu, "O6154677274656e20746f73206f7470"
			               "7064650a0d");

			snprintf(LOG, LOGSZ, "received GDB command: %c "
			         "(MCU running)", buf->data[0]);
			MSIM_LOG_WARN(LOG);
		}
		return;
	}

	switch (buf->data[0]) {
	case 0x03:
		MSIM_LOG_WARN("break command received (MCU stopped)");
		return;
	case '!':
		/* Extended mode request */
		put_str_packet(mcu, "OK");
		return;
	case '?':
		/* Report why MCU halted */
		rsp_report_exception(mcu);
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
		put_str_packet(mcu, "OK");

		rsp_close_client();
		return;
	case 'g':
		rsp_read_all_regs(mcu);
		return;
	case 'G':
		rsp_write_all_regs(mcu, buf);
		return;
	case 'H':
		/*
		 * Set a thread number for subsequent operations.
		 * Send OK to ignore silently. */
		put_str_packet(mcu, "OK");
		return;
	case 'k':
		/* Kill request. Terminate simulation. */
		rsp.mcu->state = AVR_MSIM_STOP;
		return;
	case 'm':
		/* Read memory (symbolic) */
		rsp_read_mem(mcu, buf);
		return;
	case 'M':
		/* Write memory (symbolic) */
		rsp_write_mem(mcu, buf);
		return;
	case 'p':
		rsp_read_reg(mcu, buf);
		return;
	case 'P':
		rsp_write_reg(mcu, buf);
		return;
	case 'q':
		/* One of query packets */
		rsp_query(mcu, buf);
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
		rsp_vpkt(mcu, buf);
		return;
	case 'X':
		/* Write memory (binary) */
		rsp_write_mem_bin(mcu, buf);
		return;
	case 'z':
		/* Remove a breakpoint/watchpoint */
		rsp_remove_matchpoint(mcu, buf);
		return;
	case 'Z':
		/* Insert a breakpoint/watchpoint */
		rsp_insert_matchpoint(mcu, buf);
		return;
	default:
		/* Unknown commands are ignored */
		snprintf(LOG, LOGSZ, "unknown RSP request: %s", buf->data);
		MSIM_LOG_WARN(LOG);

		return;
	}
}

static void
rsp_read_reg(MSIM_AVR *mcu, rsp_buf *buf)
{
	uint32_t regn;
	char val[REG_BUF_MAX];
	int vals;

	vals = sscanf(buf->data, "p%" SCNx32, &regn);
	if (vals != 1) {
		snprintf(LOG, LOGSZ, "Failed to recognize RSP read register "
		         "command: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
	} else {
		val[0] = 0;
		if (!read_reg((int)regn, val)) {
			snprintf(LOG, LOGSZ, "Unknown register %" PRIu32
			         ", empty response will be returned", regn);
			MSIM_LOG_ERROR(LOG);
		}

		put_str_packet(mcu, val);
	}
}

static void
rsp_write_reg(MSIM_AVR *mcu, rsp_buf *buf)
{
	uint32_t regn;
	char val[REG_BUF_MAX];
	int vals;

	vals = sscanf(buf->data, "P%" SCNx32 "=%8s", &regn, val);

	if (vals != 2) {
		snprintf(LOG, LOGSZ, "failed to recognize RSP write register "
		         "command: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
	} else {
		write_reg((int)regn, val);

		put_str_packet(mcu, "OK");
	}
}

static void
rsp_insert_matchpoint(struct MSIM_AVR *mcu, struct rsp_buf *buf)
{
	enum mp_type type;
	unsigned long addr;
	int len, vals;
	unsigned char llsb, lmsb, hlsb, hmsb;
	unsigned short inst;

	vals = sscanf(buf->data, "Z%1d,%lx,%1d", (int *)&type, &addr, &len);
	if (vals != 3) {
		snprintf(LOG, LOGSZ, "RSP matchpoint insertion request not "
		         "recognized: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
		return;
	}

	if (len != 2) {
		snprintf(LOG, LOGSZ, "RSP matchpoint length %d is not valid: "
		         "2 assumed", len);
		MSIM_LOG_WARN(LOG);

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
			snprintf(LOG, LOGSZ, "BREAK is already at 0x%8lX, "
			         "ignoring", addr);
			MSIM_LOG_WARN(LOG);

			put_str_packet(mcu, "OK");
			return;
		}

		mcu->pm[addr] = BREAK_LOW;
		mcu->pm[addr+1] = BREAK_HIGH;
		mcu->mpm[addr] = llsb;
		mcu->mpm[addr+1] = lmsb;
		if (MSIM_AVR_Is32(inst)) {
			hlsb = (unsigned char) mcu->pm[addr+2];
			hmsb = (unsigned char) mcu->pm[addr+3];
			mcu->pm[addr+2] = 0;
			mcu->pm[addr+3] = 0;
			mcu->mpm[addr+2] = hlsb;
			mcu->mpm[addr+3] = hmsb;
		}

		put_str_packet(mcu, "OK");
		break;
	default:
		snprintf(LOG, LOGSZ, "RSP matchpoint type %d is not "
		         "supported", type);
		MSIM_LOG_WARN(LOG);

		put_str_packet(mcu, "");
		break;
	}
}

static void
rsp_remove_matchpoint(MSIM_AVR *mcu, rsp_buf *buf)
{
	enum mp_type type;
	unsigned long addr;
	int len, vals;
	unsigned char llsb, lmsb, hlsb, hmsb;
	unsigned short inst;

	vals = sscanf(buf->data, "z%1d,%lx,%1d", (int *)&type, &addr, &len);
	if (vals != 3) {
		snprintf(LOG, LOGSZ, "RSP matchpoint insertion request not "
		         "recognized: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
		return;
	}

	if (len != 2) {
		snprintf(LOG, LOGSZ, "RSP matchpoint length %d is not valid: "
		         "2 assumed", len);
		MSIM_LOG_WARN(LOG);

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
			snprintf(LOG, LOGSZ, "there is no BREAK at 0x%8lX "
			         "address, ignoring", addr);
			MSIM_LOG_ERROR(LOG);

			put_str_packet(mcu, "E01");
			return;
		}

		llsb = (unsigned char) mcu->mpm[addr];
		lmsb = (unsigned char) mcu->mpm[addr+1];
		mcu->pm[addr] = llsb;
		mcu->pm[addr+1] = lmsb;

		inst = (unsigned short) (llsb | (lmsb << 8));
		if (MSIM_AVR_Is32(inst)) {
			hlsb = (unsigned char) mcu->mpm[addr+2];
			hmsb = (unsigned char) mcu->mpm[addr+3];
			mcu->pm[addr+2] = hlsb;
			mcu->pm[addr+3] = hmsb;
		}

		put_str_packet(mcu, "OK");
		break;
	default:
		snprintf(LOG, LOGSZ, "RSP matchpoint type %d is not "
		         "supported", type);
		MSIM_LOG_WARN(LOG);

		put_str_packet(mcu, "");
		break;
	}
}

static struct rsp_buf *
get_packet(void)
{
	static struct rsp_buf buf;
	uint8_t checksum;
	uint64_t count;			/* Index into the buffer */
	int8_t ch;			/* Current character */

	/* Keep getting packets, until one is found with a valid checksum */
	while (1) {
		/* Wait around for the start character ('$'). Ignore
		 * all other characters. */
		ch = (char)get_rsp_char();
		while (ch != '$') {
			if (ch == -1) {
				return  NULL;
			}

			/* 0x03 is a special case, an out-of-band break when
			 * running */
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
			/* Update checksum and add the char to the buffer. */
			checksum = (uint8_t)(checksum + (uint8_t)ch);
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
			uint8_t xmitcsum;

			ch = (char)get_rsp_char();
			if (ch == -1) {
				return NULL;
			}

			xmitcsum = (unsigned char)(hex(ch)<<4);

			ch = (char)get_rsp_char();
			if (ch == -1) {
				return  NULL;
			}

			xmitcsum = (uint8_t)(xmitcsum + (uint8_t)hex(ch));

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

static int
get_rsp_char(void)
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

static void
put_rsp_char(char c)
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

static int
hex(int c)
{
	return ((c >= 'a') && (c <= 'f')) ? c - 'a' + 10 :
	       ((c >= '0') && (c <= '9')) ? c - '0' :
	       ((c >= 'A') && (c <= 'F')) ? c - 'A' + 10 : 0;
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
static unsigned long
hex2reg(char *buf, const unsigned long dign)
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

static void
put_str_packet(MSIM_AVR *mcu, const char *str)
{
	struct rsp_buf buf;
	unsigned long len = strlen(str);

	/* Construct the packet to send, so long as string is not too big,
	 * otherwise truncate. Add EOS at the end for convenient debug
	 * printout. */
	if (len >= GDB_BUF_MAX) {
		fprintf(stderr, "Warning: String %s too large for RSP "
		        "packet: truncated\n", str);
		len = GDB_BUF_MAX - 1;
	}

	strncpy(buf.data, str, len);
	buf.data[len] = 0;
	buf.len = len;

	put_packet(mcu, &buf);
}

static void
put_packet(MSIM_AVR *mcu, rsp_buf *buf)
{
	int32_t ch;
	uint32_t count;
	int8_t c;

	/*
	 * Construct $<packet info>#<checksum>.
	 * Repeat until the GDB client acknowledges satisfactory receipt.
	 */
	do {
		uint8_t checksum = 0;

		put_rsp_char('$');		/* Start of the packet */

		/* Body of the packet */
		for (count = 0; count < buf->len; count++) {
			c = buf->data[count];
			/* Check for escaped chars */
			if (('$' == c) || ('#' == c) ||
			                ('*' == c) || ('}' == c)) {
				c ^= 0x20;
				checksum = (uint8_t)(checksum + (uint8_t)'}');
				put_rsp_char('}');
			}
			checksum = (uint8_t)(checksum + (uint8_t)c);
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

static void
rsp_report_exception(MSIM_AVR *mcu)
{
	struct rsp_buf buf;

	/* Construct a signal received packet */
	buf.data[0] = 'S';
	buf.data[1] = hexchars[rsp.sigval >> 4];
	buf.data[2] = hexchars[rsp.sigval % 16];
	buf.data[3] = 0;
	buf.len = strlen(buf.data);

	put_packet(mcu, &buf);
}

static void
rsp_continue(struct rsp_buf *buf)
{
	uint32_t addr;

	if (sscanf(buf->data, "c%" SCNx32, &addr) == 1) {
		rsp.mcu->pc = (addr >> 1);
	}
	rsp.mcu->state = AVR_RUNNING;
	rsp.client_waiting = 1;
}

static void
rsp_query(MSIM_AVR *mcu, rsp_buf *buf)
{
	if (!strcmp("qC", buf->data)) {
		/*
		 * Return the current thread ID (unsigned hex). A null
		 * response indicates to use the previously selected thread.
		 * Since we do not support a thread concept, this is the
		 * appropriate response.
		 */
		put_str_packet(mcu, "");
	} else if (!strcmp("qOffsets", buf->data)) {
		/* Report any relocation */
		put_str_packet(mcu, "Text=0;Data=0;Bss=0");
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
		put_str_packet(mcu, reply);
	} else if (!strncmp("qSymbol:", buf->data, strlen("qSymbol:"))) {
		/*
		 * Offer to look up symbols. Nothing we want (for now).
		 * TODO. This just ignores any replies to symbols we
		 * looked up, but we didn't want to do that anyway!
		 */
		put_str_packet(mcu, "OK");
	} else if (!strncmp("qTStatus", buf->data, strlen("qTStatus"))) {
		/* Trace feature is not supported */
		put_str_packet(mcu, "");
	} else if (!strncmp("qfThreadInfo", buf->data,
	                    strlen("qfThreadInfo"))) {
		/* Return info about active threads. */
		put_str_packet(mcu, "m-1");
	} else if (!strncmp("qAttached", buf->data, strlen("qAttached"))) {
		put_str_packet(mcu, "");
	} else if (!strncmp("qsThreadInfo", buf->data,
	                    strlen("qsThreadInfo"))) {
		/* Return info about more active threads */
		put_str_packet(mcu, "l");
	} else {
		MSIM_LOG_WARN("unrecognized RSP query");
	}
}

static void
rsp_vpkt(MSIM_AVR *mcu, rsp_buf *buf)
{
	if (!strncmp("vAttach;", buf->data, strlen("vAttach;"))) {
		/*
		 * Attaching is a null action, since we have no other
		 * process. We just return a stop packet (using TRAP)
		 * to indicate we are stopped.
		 */
		put_str_packet(mcu, "S05");
		return;
	} else if (!strcmp("vCont?", buf->data)) {
		/* There're no vCont actions supported at the moment. */
		put_str_packet(mcu, "");
		return;
	} else if (!strncmp("vCont", buf->data, strlen("vCont"))) {
		/*
		 * This shouldn't happen, because we've reported non-support
		 * via vCont? above */
		MSIM_LOG_WARN("RSP vCont is not supported: ignored");

		return;
	} else if (!strncmp("vRun;", buf->data, strlen("vRun;"))) {
		/* We shouldn't be given any args, but check for this */
		if (buf->len > strlen("vRun;")) {
			MSIM_LOG_WARN("Unexpected arguments to RSP vRun "
			              "command: ignored");
		}

		/*
		 * Restart the current program. However unlike a "R" packet,
		 * "vRun" should behave as though it has just stopped.
		 * We use signal 5 (TRAP).
		 */
		rsp_restart();
		put_str_packet(mcu, "S05");
	} else if (!strncmp("vKill;", buf->data, strlen("vKill;"))) {
		/* Restart MCU in stopped state on kill request */
		rsp_restart();
		put_str_packet(mcu, "OK");
	} else {
		fprintf(stderr, "Unknown RSP 'v' packet type %s: ignored\n",
		        buf->data);
		put_str_packet(mcu, "E01");
		return;
	}
}

static void
rsp_restart(void)
{
	rsp.mcu->pc = rsp.mcu->intr.reset_pc;
	rsp.mcu->state = AVR_STOPPED;
}

static size_t
read_reg(int n, char *buf)
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
		        (unsigned char)((rsp.mcu->pc << 1) & 0xFF),
		        (unsigned char)(((rsp.mcu->pc << 1) >> 8) & 0xFF),
		        (unsigned char)(((rsp.mcu->pc << 1) >> 16) & 0xFF));
		break;
	}
	return strlen(buf);
}

static void
write_reg(int n, char *buf)
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
		rsp.mcu->pc = (uint32_t) (hex2reg(buf, 8) >> 1);
		break;
	}
	return;
}

static void
rsp_read_all_regs(MSIM_AVR *mcu)
{
	char reply[GDB_BUF_MAX];
	char *rep;
	int i;

	rep = reply;
	for (i = 0; i < 35; i++) {
		rep += read_reg(i, rep);
	}
	*rep = 0;

	put_str_packet(mcu, reply);
}

static void
rsp_write_all_regs(MSIM_AVR *mcu, rsp_buf *buf)
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
			rsp.mcu->pc = (uint32_t)(hex2reg(buf->data+off, 8) >> 1);
			off += 8;
			break;
		}
	}

	put_str_packet(mcu, "OK");
}

static void
rsp_read_mem(MSIM_AVR *mcu, rsp_buf *buf)
{
	unsigned int addr, len, i;
	unsigned char c;
	unsigned char *src = NULL;
	uint16_t *pm = NULL;
	unsigned char tmp_buf[(GDB_BUF_MAX - 1) / 2];

	if (sscanf(buf->data, "m%x,%x:", &addr, &len) != 2) {
		snprintf(LOG, LOGSZ, "failed to recognize RSP read memory "
		         "command: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
		return;
	}
	addr &= 0x00FFFFFF;

	/* Make sure we won't overflow the buffer (2 chars per byte) */
	if ((len*2) >= GDB_BUF_MAX) {
		snprintf(LOG, LOGSZ, "memory read %s too large for RSP packet: "
		         "requested chunk will be truncated", buf->data);
		MSIM_LOG_WARN(LOG);

		len = (GDB_BUF_MAX-1)/2;
	}

	/* Find a memory to read from */
	if (addr < rsp.mcu->flashend) {
		pm = rsp.mcu->pm + (addr >> 1);

		/* Prepare bytes of the progmem */
		for (i = 0; i < len; i += 2) {
			tmp_buf[i] =
			        (unsigned char)((pm[i >> 1] & 0xFF));
			tmp_buf[i + 1] =
			        (unsigned char)((pm[i >> 1] >> 8) & 0xFF);
		}
		src = &tmp_buf[0];
	} else if ((addr >= 0x800000) &&
	                ((addr-0x800000) <= rsp.mcu->ramend)) {
		src = rsp.mcu->dm + addr - 0x800000;
	} else if (addr == (0x800000 + rsp.mcu->ramend+1) && len == 2) {
		put_str_packet(mcu, "0000");
		return;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		//src = rsp.mcu->ee + addr - 0x810000;
		put_str_packet(mcu, "E01");
		return;
	} else {
		snprintf(LOG, LOGSZ, "Unable to read memory %08X, %08X",
		         addr, len);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
		return;
	}

	/* Do the memory read into a temporary buffer */
	if (src != NULL) {
		for (i = 0; i < len; i++) {
			c = src[i];
			buf->data[i*2] = hexchars[c>>4];
			buf->data[i*2+1] = hexchars[c&0xF];
		}
		buf->data[i*2] = 0;
		buf->len = strlen(buf->data);
	}

	put_packet(mcu, buf);
}

static void
rsp_write_mem(MSIM_AVR *mcu, rsp_buf *buf)
{
	static uint8_t tmpbuf[GDB_BUF_MAX];
	uint64_t addr, datlen;
	uint32_t len;
	char *symdat;
	uint32_t off;
	uint8_t mnyb, lnyb;
	uint8_t *dest = NULL;
	uint16_t *pm = NULL;

	if (sscanf(buf->data, "M%lx,%x:", &addr, &len) != 2) {
		snprintf(LOG, LOGSZ, "failed to recognize RSP write memory "
		         "command: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
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
		snprintf(LOG, LOGSZ, "write of %d digits requested, but %lu "
		         "digits supplied: request ignored", len*2, datlen);
		MSIM_LOG_WARN(LOG);

		put_str_packet(mcu, "E01");
		return;
	}

	/* Write bytes to memory */
	for (off = 0; off < len; off++) {
		mnyb = (uint8_t) hex(symdat[off*2]);
		lnyb = (uint8_t) hex(symdat[off*2+1]);
		tmpbuf[off] = (uint8_t)(((mnyb<<4)&0xF0)|(lnyb&0x0F));
	}

	/* Find a memory to write to */
	if (addr < rsp.mcu->flashend) {
		pm = rsp.mcu->pm + (addr >> 1);

		for (uint32_t i = 0; i < (len >> 1); i++) {
			pm[i] = (uint16_t)(
			                ((tmpbuf[(i << 1) + 1] << 8) & 0xFF00) |
			                (tmpbuf[(i << 1)] &0xFF));
		}
	} else if ((addr >= 0x800000) &&
	                ((addr-0x800000) <= rsp.mcu->ramend)) {
		dest = rsp.mcu->dm + addr - 0x800000;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		/*dest = rsp.mcu->ee + addr - 0x810000;*/
		put_str_packet(mcu, "E01");
		return;
	} else {
		snprintf(LOG, LOGSZ, "unable to write memory %08lX, %08X",
		         addr, len);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
		return;
	}

	if (dest != NULL) {
		memcpy(dest, tmpbuf, len);
	}

	put_str_packet(mcu, "OK");
}

static void
rsp_write_mem_bin(MSIM_AVR *mcu, rsp_buf *buf)
{
	unsigned long addr, datlen, len;
	char *bindat;
	unsigned int off;
	unsigned char *dest = NULL;
	uint16_t *pm = NULL;

	if (sscanf(buf->data, "X%lx,%lx:", &addr, &len) != 2) {
		snprintf(LOG, LOGSZ, "failed to recognize RSP write memory "
		         "command: %s", buf->data);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
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

		snprintf(LOG, LOGSZ, "write of %lu bytes requested, but %lu "
		         "bytes supplied. %lu will be written",
		         len, datlen, minlen);
		MSIM_LOG_WARN(LOG);

		len = minlen;
	}

	/* Find a memory to write to */
	if (addr < rsp.mcu->flashend) {
		pm = rsp.mcu->pm + (addr >> 1);

		for (uint32_t i = 0; i < (len >> 1); i++) {
			pm[i] = (uint16_t)(
			                ((bindat[(i << 1) + 1] << 8) & 0xFF00) |
			                (bindat[(i << 1)] &0xFF));
		}
	} else if ((addr >= 0x800000) &&
	                ((addr-0x800000) <= rsp.mcu->ramend)) {
		dest = rsp.mcu->dm + addr - 0x800000;
	} else if (addr >= 0x810000 && (addr-0x810000) <= rsp.mcu->e2end) {
		/* There should be a pointer to EEPROM */
		/*dest = rsp.mcu->ee + addr - 0x810000;*/
		put_str_packet(mcu, "E01");
		return;
	} else {
		snprintf(LOG, LOGSZ, "unable to write memory %08lX, %08lX",
		         addr, len);
		MSIM_LOG_ERROR(LOG);

		put_str_packet(mcu, "E01");
		return;
	}

	if (dest != NULL) {
		memcpy(dest, bindat, len);
	}

	put_str_packet(mcu, "OK");
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
static unsigned long
rsp_unescape(char *data, unsigned long len)
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

static void
rsp_step(struct rsp_buf *buf)
{
	rsp.mcu->state = AVR_MSIM_STEP;
	rsp.client_waiting = 1;
}
