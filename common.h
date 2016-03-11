/**
 * common.h:
 *
 * Common client/server definitions and types for Codemon project
 * CIS*2750F15 (prototypes/data definitions).
 *
 * @author   David McCaughan
 * @version  2.0, 2015/08/11
 *
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include "arch.h"

/*
 * Server information (name + port + your player ID as strings)
 */
#define SERVER   "eon.socs.uoguelph.ca"
#define PORT     "4848"
#define PLAYERID "Doughnuts"

/*
 * Version number
 */
#define VERSION    "2.0"

/*
 * Basic Codemon parameters (be aware that not all of these are necessarily
 * relevant to the client-side)
 * - maximum length of names and ids is 19 characters (20 including '\0')
 * - instruction limit in a Codemon is 50 (not including "begin" address)
 * - memory has 8192 locations (all values are integers between 0 and 8191)
 * - maximum number of turns in a Codemon game is 10000 (for now, this will
 *   change in future, particular for tournament purposes)
 */
#define MAXNAME    19
#define MAXINST    50
#define MEMSIZE    8192
#define MAXTURNS   10000

/*
 * File extension for codemon programs
 */
#define CODEMONEXT ".codemon"

/*
 * connection codeword for authentication
 */
#define CODEWORD 0xdeadbeef

/*
 * size of staging buffer for report receipt
 */
#define FBUFSIZE 102400

/**
 * TYPE: Request
 * client operation request encodings
 */
typedef enum
{
	TEST = 0x1,     /* load a Codemon and run alone (or view @0) */
	SELF = 0x2,		/* load 2 Codemon and run a match */
	PVP2 = 0x3,		/* load 1 Codemon + 1 other player and run a match */
	PVP3 = 0x4,		/* load 1 Codemon + 2 other players and run a match */
	PVP4 = 0x5,		/* load 1 Codemon + 3 other players and run a match */
	RPRT = 0x6		/* requesting a game report */
} Request;

/**
 * TYPE: Packet
 * packet encodings for network response
 */
#define NPACKETS 2
typedef enum
{
	ACK = 0x0,		/* load a Codemon and display */
	BYE = 0x1,      /* load a Codemon and run alone */
} Packet;

/*
 * information package structure
 */
struct info_pkg
{
	char id[MAXNAME+1];
	Request req;
	uint32 val;
};

/*
 * Codemon package structure
 */
struct Codemon_pkg
{
	char name[MAXNAME+1];
	uint32 lines;
	uint32 begin;
	uint64 program[MAXINST];
};


/* --- file: client.c --- */
uint32 runTest   (struct Codemon_pkg *, struct Codemon_pkg *, uint32);
uint32 runPvP    (struct Codemon_pkg *, uint32);
int32  getReport (uint32, FILE *);


#endif