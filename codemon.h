/*
 ============================================================================
 Name        : codemon.h
 Author      : Alexander Lapena 
 ID 		 : 0844071
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

#include "common.h"
#include "arch.h"
typedef struct labelHandler{
	int line;
	char label[50];
}labelHandler;

static const char codes[18][4] = {"DAT","MOV","ADD","SUB","MUL","DIV","MOD",
	"JMP","JMZ","JMN","DJN","SEQ","SNE","SLT","SET","CLR","FRK","NOP"};
	
static const char addMode[11] = "$#[]*@{}<>";

void toBinary(char * fileName);

int testMode(char * fileName, int limit);

int selfMode(char * fileName, char * fileName2, int limit);

int pvpMode(char * fileName, int type);

void replaceString(char *line,char *search, char *replace);

int adjust(int value);

uint64 opCodeShift(char * instruction);

uint64 modeAShift(char * instruction);

struct Codemon_pkg build(char * filename);

uint64 modeBShift(char * instruction);
