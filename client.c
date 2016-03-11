/**
 * client.c
 *
 * Networking routines needed for client applications in order to submit
 * Codemon to the Codex server for execution/competition/etc. for Codemon
 * project CIS*2750F15 (implementation)
 *
 * Note specific connection process adapted from:
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
 *
 * @author   David McCaughan
 * @version  2.0, 2015/08/11
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include "common.h"
#include "arch.h"


/*
 * packet string mappings
 */
char *packet2str[NPACKETS] = { "ACK", "BYE" };


/*
 * prototypes for internal use functions
 */
FILE * _commonConnect (Request, uint32);
int32  _sendCodemon   (struct Codemon_pkg *, FILE *);
int32  _recvPacket    (Packet, FILE *);
int32  _recvReport    (FILE *, FILE *);
uint32 _recvID        (FILE *);

/**
 * runTest:
 *
 * Submit and run testing (TEST and SELF) requests on the Codex (i.e.
 * validates provided Codemon as best as it can and handles network
 * connectivity, transmission of request/Codemon and receipt of report.
 * If only * c1 has a value (and c2 == NULL), it is a TEST operation,
 * otherwise if both c1 and c2 have a value it is a SELF operation.
 *
 * @pre  for Codemon: 0 < lines <= MAXINST
 * @pre  for Codemon: 0 <= begin < lines
 * @pre  for Codemon: inst_buf contains lines instructions
 * @post Codemon have been submitted to Codex in TEST/SELF mode for limit
 *       iterations
 *
 * @param  c1 reference to raw Codemon1 data for transmission
 * @param  c2 reference to raw Codemon2 data for transmission
 * @param  limit  iteration limit for execution
 * @return report id for run on success, 0 otherwise
 * @error   0 invalid parameters (in Codemon or limit)
 * @error   0 networking error (set-up or transmission)
 *
 */
uint32 runTest(struct Codemon_pkg *c1, struct Codemon_pkg *c2, uint32 limit)
{
	FILE *fp_net = NULL;	/* stream endpoint for socket */
	uint32 reportid;		/* report ID from server */

	/*
	 * sanity checking what we've been given here...
	 */
	if (limit > MAXTURNS)
		return(0);

	if (c1)
	{
		if (c1->lines > MAXINST)
			goto FAIL;
		if (c1->begin >= c1->lines)
			goto FAIL;
	}

	if (c2)
	{
		if (c1->lines > MAXINST)
			goto FAIL;
		if (c2->begin >= c2->lines)
			goto FAIL;
	}
	
	if (c1 && c2)
	{
		if (!(fp_net = _commonConnect(SELF, limit)))
			goto FAIL;
	}
	else if (c1)
	{
		if (!(fp_net = _commonConnect(TEST, limit)))
			goto FAIL;
	}
	else
		goto FAIL;

	/*
	 * either request involves sending first Codemon
	 */
#ifdef _TRACE
	printf("    > Sending Codemon(1) data...");
	fflush(stdout);
#endif

	if (!_sendCodemon(c1,fp_net))
		goto FAIL;

	/*
	 * SELF request involve sending a second Codemon
	 */
	if (c2)
	{
#ifdef _TRACE
		printf("    > Sending Codemon(2) data...");
		fflush(stdout);
#endif

		if (!_sendCodemon(c2,fp_net))
			goto FAIL;
	}

	/*
	 * receive report id from server
	 */
	if (!(reportid = _recvID(fp_net)))
		goto FAIL;

#ifdef _TRACE
	printf("[i] Done -> REPORT ID = %010u.\n",reportid);
#endif
	fclose(fp_net);

	return(reportid);

FAIL:
	if (fp_net)
		fclose(fp_net);
	return(0);

} /* runTest */


/**
 * runPvP:
 *
 * Submit Codemon to the Codex for PvP requests (PVP2, PVP3, PVP4) against
 * other players (i.e. validates provided Codemon as best as it can and
 * handles network connectivity and transmission of request/Codemon---reports
 * are retrieved separately based on server-provided report_id)
 *
 * @pre  for Codemon: 0 < lines <= MAXINST
 * @pre  for Codemon: 0 <= begin < lines
 * @pre  for Codemon: inst_buf contains lines instructions
 * @pre  size is in {2, 3, 4}
 * @post Codemon has been submitted to Codex in PVP2/PVP3/PVP4 mode
 *
 * @param  cmon reference to raw Codemon1 data for transmission
 * @param  size desired size of match = {2, 3, 4}
 * @return report id for run on success, 0 otherwise
 * @error   0 invalid parameters (in Codemon or size)
 * @error   0 networking error (set-up or transmission)
 *
 */
uint32 runPvP(struct Codemon_pkg *cmon, uint32 size)
{
	FILE *fp_net = NULL;	/* stream endpoint for socket */
	uint32 reportid;		/* report ID from server */

	/*
	 * sanity checking what we've been given here...
	 */
	if ((size < 2) || (size > 4))
		goto FAIL;

	if (cmon)
	{
		if (cmon->lines > MAXINST)
			goto FAIL;
		if (cmon->begin >= cmon->lines)
			goto FAIL;
	}
	else
		goto FAIL;

	switch(size)
	{
		case 2:
			if (!(fp_net = _commonConnect(PVP2, MAXTURNS)))
				goto FAIL;
			break;
		case 3:
			if (!(fp_net = _commonConnect(PVP3, MAXTURNS)))
				goto FAIL;
			break;
		case 4:
			if (!(fp_net = _commonConnect(PVP4, MAXTURNS)))
				goto FAIL;
			break;
		default:
			goto FAIL;
	}

	/*
	 * all requests involve sending the Codemon
	 */
#ifdef _TRACE
	printf("    > Sending Codemon data...");
	fflush(stdout);
#endif

	if (!_sendCodemon(cmon,fp_net))
		goto FAIL;

	/*
	 * receive report id from server
	 */
	if (!(reportid = _recvID(fp_net)))
		goto FAIL;

#ifdef _TRACE
	printf("[i] Done -> REPORT ID = %010u.\n",reportid);
#endif
	fclose(fp_net);

	return(reportid);

FAIL:
	if (fp_net)
		fclose(fp_net);
	return(0);

} /* runPvP */


/**
 * getReport:
 *
 * Obtain report with specified ID from the server and insert it into
 * the provided stream.
 *
 * @pre  id is a valid report ID for the server
 * @post report from server received and inserted into fp
 *
 * @param  id  ID of report to be retreived
 * @param  fp target stream for report insertion
 * @return 1 on success, 0 if report unavailable
 * @error  -1 network error (transmission)
 *
 */
int32 getReport(uint32 id, FILE *fp)
{
	FILE *fp_net;				/* stream endpoint for socket */

	/*
	 * NOTE: The other half of the invalid report ID kludge happens here;
	 * the server is now excplitly sending an ACK for valid info, but an
	 * invalid report ID.  This requires no edits here as it turns out as
	 * _commonConnect simply expects the ACK; however, this permits the
	 * client to now distinguish an invalid report from a network
	 * error as _commonConnect will return successfully if the report id was
	 * invalid (note that a network error is still coming as the ACK for a
	 * report request for an invalid ID is still disconnected by the server,
	 * so the _recvReport is still going to return an error).  As noted
	 * server-side, on a refactoring of this code, this would be cleaned up,
	 * but should suffice for our purposes at the moment.
	 */
	if (!(fp_net = _commonConnect(RPRT, id)))
		return(-1);

	if (!_recvReport(fp_net,fp))
	{
		fclose(fp_net);
		return(0);
	}

	fclose(fp_net);

	return(1);

} /* getReport */


/**
 * _commmonConnect:
 *
 * Perform common handshaking and initial communication with server to
 * establish a valid bi-directional stream ready for the specifics of
 * the provided request.
 *
 * @pre  req is one of {TEST, SELF, PVP2, PVP3, PVP4, RPRT}
 * @pre  value valid given request type (server tested)
 * @post server connected for req on returned stream
 *
 * @param  req    request type for this connection (TEST, SELF, etc.)
 * @param  value  iteration limit for execution or report ID
 * @return new stream connection to server suitable for req
 * @error  NULL  invalid parameters
 * @error  NULL  error connecting stream to socket
 * @error  NULL  network error (set-up or transmission)
 *
 */
FILE *_commonConnect(Request req, uint32 value)
{
	int32 sockfd;							/* connection socket */
	int32 retval;							/* tracking return value(s) */
	struct addrinfo hints, *servinfo, *p;	/* network address structures */
	FILE *fp_net = NULL;					/* stream endpoint for socket */
	struct info_pkg info;					/* information package */
	uint32 code;							/* host byte order code */

#ifdef _TRACE
	fprintf(stdout, "[i] Connecting to server...\n");
	fflush(stdout);
#endif

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
#ifdef _TRACE
		perror("[!] socket\n");
#endif
		return(NULL);
	}

	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((retval = getaddrinfo(SERVER, PORT, &hints, &servinfo)) < 0)
	{
#ifdef _TRACE
		perror("[!] getaddrinfo\n");
#endif
		return(NULL);
	}

	for (p = servinfo; p!= NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
		{
#ifdef _TRACE
			perror("[!] socket\n");
#endif
			continue;
		}
		if (connect(sockfd,p->ai_addr, p->ai_addrlen) < 0)
		{
#ifdef _TRACE
			perror("[!] connect");
#endif
			close(sockfd);
			return(NULL);
		}
		break;
	}

	if (p == NULL)
	{
#ifdef _TRACE
		fprintf(stderr,"[!] client unable to connect to server\n");
#endif
		return(NULL);
	}
	
	freeaddrinfo(servinfo);

	if (!(fp_net = fdopen(sockfd,"r+")))
	{
#ifdef _TRACE
		perror("[!] fdopen");
#endif
		close(sockfd);
		return(NULL);
	}

#ifdef _TRACE
	printf("connected\n");
	printf("    > sending code...");
	fflush(stdout);
#endif

	code = htonl(CODEWORD);
	if (fwrite(&code,sizeof(uint32), 1, fp_net) < 1)
		goto FAIL;

	if (!_recvPacket(ACK,fp_net))
		goto FAIL;

#ifdef _TRACE
	printf("    > construct and send info package...");
	fflush(stdout);
#endif
	info.req = htonl(req);
	strncpy(info.id,PLAYERID,MAXNAME+1);
	info.id[MAXNAME]='\0';
	info.val = htonl(value);

	if (fwrite(info.id, sizeof(char), MAXNAME+1, fp_net) < (MAXNAME+1))
		goto FAIL;
	if (fwrite(&info.req, sizeof(uint32), 1, fp_net) < 1)
		goto FAIL;;
	if (fwrite(&info.val, sizeof(uint32), 1, fp_net) < 1)
		goto FAIL;

	if (!_recvPacket(ACK,fp_net))
		goto FAIL;

	return(fp_net);

FAIL:
	if (fp_net)
		fclose(fp_net);
	return(NULL);

} /* _commonConnect */


/**
 * _sendCodemon:
 *
 * Internal routine to perform common transmission of a Codemon_pkg to
 * the server connected on the provided stream.  Expected order for
 * serialization is: name (MAXNAME+1 chars), lines and begin (as uint32),
 * followed by the program (contiguous sequence of lines uint64).  Note
 * that all integral values are sent in network byte order except the program
 * itself which has explicitly specified binary formatting.  Expects an ACK
 * from the server to confirm receipt.
 *
 * @pre  Codemon has been validated for content
 * @post cmon has been serialized and sent to Codex
 *
 * @param  cmon  reference to raw Codemon data for transmission
 * @param  fp    destination stream for Codemon transmission
 * @return 1 on success
 * @error  0  network error (transmission)
 *
 */
int32 _sendCodemon(struct Codemon_pkg *cmon, FILE *fp)
{
	uint32 netlines, netbegin;	/* values for network byte order conversion */

	netlines = htonl(cmon->lines);
	netbegin = htonl(cmon->begin);

	if (fwrite(cmon->name, sizeof(char), MAXNAME+1, fp) < (MAXNAME+1))
		return(0);
    if (fwrite(&netlines, sizeof(uint32), 1, fp) < 1)
        return(0);
    if (fwrite(&netbegin, sizeof(uint32), 1, fp) < 1)
        return(0);
    if (fwrite(cmon->program, sizeof(uint64), cmon->lines, fp) < cmon->lines)
        return(0);

	if (!_recvPacket(ACK,fp))
		return(0);

	return(1);

} /* _sendCodemon */


/**
 * _recvPacket:
 *
 * Receipt of packet from server routine.  Attempts to receive and validate
 * the specified packet from the server connected on the provided stream.
 * Packet is assumed to be in network byte order upon receipt.
 *
 * @pre  packet is a valid packet type
 * @post packet received from server has been validated for expected type
 *
 * @param  packet packet expected from stream
 * @param  fp     source stream for connected server
 * @return 1 if packet was received as expected, 0 otherwise
 * @error  0  network error (transmission)
 *
 */
int32 _recvPacket(Packet packet, FILE *fp)
{
	uint32 code;			/* host byte order code */

	if (fread(&code,sizeof(uint32), 1, fp) < 1)
		return(0);

	if (ntohl(code) == packet)
	{
#ifdef _TRACE
		printf("received %s\n",packet2str[packet]);
#endif
		return(1);
	}
	else
	{
#ifdef _TRACE
		printf("received garbage instead of %s\n",packet2str[packet]);
#endif
		return(0);
	}

} /* _recvPacket */


/**
 * _recvID:
 *
 * Internal common receipt of report ID from server routine.  ID is assumed
 * to be in network byte order upon receipt.
 *
 * @pre  void
 * @post ID received from server
 *
 * @param  packet packet expected from stream
 * @param  fp     source stream for connected server
 * @return 1 if ID was received as expected, 0 otherwise
 * @error  0  network error (transmission)
 *
 */
uint32 _recvID(FILE *fp)
{
	uint32 id;			/* received ID */

	if (fread(&id,sizeof(uint32), 1, fp) < 1)
	{
#ifdef _TRACE
		printf("error receiving report id\n");
#endif
		return(0);
	}
	else
	{
		id = ntohl(id);
#ifdef _TRACE
	printf("received id %u\n",id);
#endif
		return(id);
	}

} /* _recvID */


/**
 * _recvReport:
 *
 * Internal routine to obtain reports from the server and insert it into
 * another (local) stream.
 *
 * @pre  fp_in contains a valid report as sent by Codemon server
 * @post report from network stream fp_in received and inserted into fp_out
 *
 * @param  fp_in  source network stream for receipt of report
 * @param  fp_out target stream for report insertion
 * @return 1 on success
 * @error  0  network error (transmission)
 *
 */
int32 _recvReport(FILE *fp_in, FILE *fp_out)
{
	uint32 tot, ctot;			/* tracking size of incoming report */
	char reportbuf[FBUFSIZE];	/* staging buffer for report receipt */

#ifdef _TRACE
	printf("    > waiting for report...");
	fflush(stdout);
#endif

	/*
	 * receiving report in FBUFSIZE-sized chunks; note need to account for
	 * any leftover bytes not a multiple of FBUFSIZE (due to blocking calls,
	 * we need to handle this carefully)
	 */
	if (fread(&tot, sizeof(uint32), 1, fp_in) < 1)
		return(0);
	tot = ctot = ntohl(tot);

	while(ctot > FBUFSIZE)
	{
		if (fread(reportbuf, sizeof(char), FBUFSIZE, fp_in) < FBUFSIZE)
			return(0);
		else if (fwrite(reportbuf, sizeof(char), FBUFSIZE, fp_out) < FBUFSIZE)
			return(0);
		ctot -= FBUFSIZE;
	}
	if (ctot > 0)
	{
		if (fread(reportbuf, sizeof(char), ctot, fp_in) < ctot)
			return(0);
		else if (fwrite(reportbuf, sizeof(char), ctot, fp_out) < ctot)
			return(0);
	}

#ifdef _TRACE
	printf("received.\n");
#endif

	return(1);

} /* _recvReport */