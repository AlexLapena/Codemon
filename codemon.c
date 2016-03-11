/*
 ============================================================================
 Name        : codemon.c
 Author      : Alexander Lapena 
 ID 	     : 0844071
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>
#include <errno.h>
#include <jni.h>

#include "codemon.h"
#include "common.h"
#include "arch.h"

/*Calls from the command line each instruction to see which mode to
 * access before running the functions*/
int main (int argc, char**argv)
{
	int z;
	int limit;
	int type;
	uint32 input;
	
	/* Check for correct command line input */
	if(argc == 1){
		printf("Error! Filename not entered.\n");
		return 0;
	}
	
	for(z = 0; z < argc; z++){
		if(strcmp(argv[z], "-c") == 0){
			toBinary(argv[2]);
		}
		else if(strcmp(argv[z], "-t") == 0){
			limit = atoi(argv[3]);
			testMode(argv[2], limit);
		}
		else if(strcmp(argv[z], "-s") == 0){
			limit = atoi(argv[4]);
			selfMode(argv[2], argv[3], limit);
		}
		else if(strcmp(argv[z], "-p") == 0){
			type = atoi(argv[2]);
			pvpMode(argv[3], type);
		}
		else if(strcmp(argv[z], "-r") == 0){
			input = (strtol(argv[2],NULL,10));
			getReport(input, stdout);
		}
	}
	
	return(0);
}

void toBinary(char * fileName){
	FILE *fp;
	char instruction[100];
	char tempBegin[50];
	char stringStore[50][50];
	char instructionStore[50][50];
	int line;
	int isLabel = 0, isMode = 0;
	int i = 0, j;
	int lineCount = 0, labelCount = 0;
	char *label, *token;
	char *mA, *mB;
	char opCode[50][50];
	char modeA[50][50];
	char modeB[50][50];
	char unparsedMode[50][50];
	char offset[50];
	
	uint64 oC;
	uint64 modA;
	uint64 modB;
	uint64 binCode;
	
	labelHandler lab[50];
	struct Codemon_pkg store;
	
	/*Opens the file*/
	fp = fopen(fileName, "r");
	if(fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fileName, strerror(errno));
		return;
	}
	
	while(!feof(fp)){
		line = fgetc(fp);
		
		/*Checks for whitespace*/
		if(line == ' ' || line == '\t'){
			continue;
		}
		/*Parses out the comments*/
		else if(line == '!'){
			while(line != '\n'){
				line = fgetc(fp);
			}
			line = fgetc(fp);
		}
		else if(line == ';'){
			instruction[i] = '\0';
			strcpy(stringStore[lineCount],instruction);
			lineCount++;
			i = 0;
			continue;
		}
		else if(line == '\n'){
			continue;
		}
		/*save statement*/
		else{
			instruction[i] = line;
			i++;
		}
	}
	/*Finding labels*/
	for(j = 0; j < lineCount; j++){
		isLabel = 0;
		for(i = 0; i < strlen(stringStore[j]); i++){
			if(stringStore[j][i] == ':'){
				isLabel = 1;
				label = strtok(stringStore[j], ":");
				token = strtok(NULL, "\0");
				strcpy(lab[labelCount].label, label);
				strcpy(instructionStore[j],token);
				lab[labelCount].line = j;
				labelCount++;
			}	
		}
		if(isLabel == 0){
			strcpy(instructionStore[j],stringStore[j]);
		}
	}
	
	/*begin line*/
	j = 0;
	for(i = 0; i < strlen(stringStore[0]); i++){
		if(isdigit(stringStore[0][i])){
			tempBegin[j] = stringStore[0][i];
			j++;
		}
	}
	
	/*Parsing out opcode from the modes*/
	for(i = 1; i < lineCount; i++){
		for(j = 0; j < strlen(instructionStore[i]); j++){
			if(j < 3){
				opCode[i][j] = instructionStore[i][j];
			}
			else if(j >= 3){
				unparsedMode[i][j-3] = instructionStore[i][j];
			}
			opCode[i][3] = '\0';
		}
	}
	
	/*Separating the modeA and modeB*/
	for(i = 1; i < lineCount; i++){
		for(j = 0; j < strlen(unparsedMode[i]); j++){
			/*checks if 2 modes*/
			if(unparsedMode[i][j] == ','){
				isMode = 1;
				mA = strtok(unparsedMode[i], ",");
				mB = strtok(NULL, "\0");
				strcpy(modeA[i],mA);
				strcpy(modeB[i],mB);
			}
		}	
		if(isMode == 0){
			strcpy(modeA[i], unparsedMode[i]);
			strcpy(modeB[i], " ");
		}
		isMode = 0;	
	}
	
	/*handle labels*/
	for(i = 0; i < labelCount; i++){
		for(j = 0; j < lineCount; j++){
			/*adjust for modeA*/
			if(strstr(modeA[j],lab[i].label)){
				sprintf(offset, "%d", (j-lab[i].line));
				replaceString(modeA[j], lab[i].label, offset);
			}
			if(strstr(modeB[j], lab[i].label)){
				sprintf(offset, "%d", (j-lab[i].line));
				replaceString(modeB[j], lab[i].label, offset);
			}
		}
	}
	
	/*translates instruction and bitshifts*/
	for(i = 1; i < lineCount; i++){
		oC = opCodeShift(opCode[i]);
		if(oC == -1){
			printf("Opcode not found!\n");
			exit(0);
		}
		if(strcmp(modeA[i],"\0") == 0){
			modA = 0;
		}
		else{
			modA = modeAShift(modeA[i]);
			if(modA == -1){
				printf("ModeA not found! %d\n", i);
				exit(0);
			}
		}
		if(strcmp(modeB[i]," ")==0){
			modB = 0;
		}
		else{
			modB = modeBShift(modeB[i]);
			if(modB == -1){
				printf("ModeB not found! %d\n", i);
				exit(0);
			}
		}
		binCode = (oC | modA | modB);
		store.program[i-1] = binCode;
	}
	store.begin = atoi(tempBegin);
	store.lines = (lineCount-1);
	fwrite(&store.begin, 4, 1, stdout);
	for(j = 0; j < store.lines; j++){
		fwrite(&store.program[j], 8, 1, stdout);
	}
	
	fclose(fp);
	
}

/*String Replacer: http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
* Replaces the string with the substring
*/
void replaceString(char *line, char *search, char *replace){
    char *stringPointer;

	if ((stringPointer = strstr(line, search)) == NULL) {
		exit(0);
	}	
	
	int searchLength = strlen(search);
	int replaceLength = strlen(replace);
	/*Searches from right to left*/
	if (searchLength > replaceLength) {
		char *source = stringPointer + searchLength;
		char *destination = stringPointer + replaceLength;
		while((*destination = *source) != '\0'){	
			destination++;
			source++; 
		}
	}
	/*Searches from left to right*/
	else if (searchLength < replaceLength) {
		int tLen = strlen(stringPointer) - searchLength;
		char *stop = stringPointer + replaceLength;
		char *source = stringPointer + searchLength + tLen;
		char *destination = stringPointer + replaceLength + tLen;
		while(destination >= stop){
			*destination = *source; 
			destination--; 
			source--; 
		}
	}
	memcpy(stringPointer, replace, replaceLength);
}

/*runs the test mode so that it will connect to the server. Returns the ID for the 
 * user to then pull the report with the '-r' flag*/
int testMode(char * fileName, int limit){
	struct Codemon_pkg codem;
	uint32 id;
	
	codem = build(fileName);
	id = runTest(&codem, NULL, limit);
	printf("ID: %u\n",id);
	return(id);
}

/*Runs self mode with 2 codemons*/
int selfMode(char * fileName, char * fileName2, int limit){
	struct Codemon_pkg codem, codem2;
	uint32 id;
	
	codem = build(fileName);
	codem2 = build(fileName2);
	id = runTest(&codem, &codem2, limit);
	printf("ID: %u\n",id);
	return(id);
}

/*Runs the pvp mode in which it shows how many codemons are able to 
 * fight against each other in the codex arena*/
int pvpMode(char * fileName, int type){
	struct Codemon_pkg codem;
	uint32 id;
	
	codem = build(fileName);
	id = runPvP(&codem, type);
	printf("ID: %u\n",id);
	return(id);
}

/*Adjusts for negative numbers*/
int adjust(int value){
	
	if(value < 0){
		value = value + 8192;
	}
	return value;
}

/*bitshifts the opcode*/
uint64 opCodeShift(char * instruction){	
	int i;
	
	for(i = 0; i < 18; i++){
		if(strcmp(instruction,codes[i]) == 0){
			return ((uint64)i << 58);
		}
	}
	return (-1);
}

/*bitshifts the address mode and the A field*/
uint64 modeAShift(char * instruction){
	int i,j;
	int a;
	int length;
	char temp[50];
	uint64 aMode;
	uint64 aVal;
	uint64 returnVal;
	
	for(i = 0; i < 10; i++){
		/*If no address mode is present*/
		if(isdigit(instruction[0])){
			a = atoi(instruction);
			aVal = ((uint64)a << 29);
			returnVal = aVal;
			return(returnVal);
		}
		/*if negative and no adress mode*/
		else if(instruction[0] == '-'){
			aMode = ((uint64)0 << 54);
			length = strlen(instruction);
			for(j = 1; j < length; j++){
				temp[j] = instruction[j];
			}
			a = atoi(temp);
			a = adjust(a);
			aVal = ((uint64)a << 29);
			returnVal = (aMode | aVal);
			return(returnVal);
		}
		/*Look up table for finding the address mode if present*/
		else if(addMode[i] == instruction[0]){
			aMode = ((uint64)i << 54);
			length = strlen(instruction);
			for(j = 1; j < length; j++){
				temp[j-1] = instruction[j];
			}
			a = atoi(temp);
			aVal = ((uint64)a << 29);
			returnVal = (aMode | aVal);
			return(returnVal);
		}
	}
	return(-1);
}

/*bitshifts the address mode and the B field*/
uint64 modeBShift(char * instruction){
	int i,j;
	int a;
	int length;
	char temp[50];
	uint64 aMode;
	uint64 aVal;
	uint64 returnVal;
	
	for(i = 0; i < 10; i++){
		/*If no address mode is present*/
		if(isdigit(instruction[0])){
			a = atoi(instruction);
			aVal = ((uint64)a);
			returnVal = aVal;
			return(returnVal);
		}
		/*if negative and no adress mode*/
		else if(instruction[0] == '-'){
			a = atoi(instruction);
			aVal = ((uint64)a);
			returnVal = aVal;
			return(returnVal);
		}
		/*Look up table for finding the address mode if present*/
		else if(instruction[0] == addMode[i]){
			aMode = ((uint64)i << 25);
			length = strlen(instruction);
			for(j = 1; j < length; j++){
				temp[j-1] = instruction[j];
			}
			a = atoi(temp);
			aVal = ((uint64)a);
			returnVal = (aMode | aVal);
			return(returnVal);
		}
	}
	return(-1);
}
/*Builds the codemon to be sent to the modes*/
struct Codemon_pkg build(char * filename){
	FILE * fp;
	int size;
	int i;
	struct Codemon_pkg cdm;
	
	fp = fopen(filename, "r");
	if(fp == NULL){
		perror(0);
		exit(0);
	}
	fseek(fp,0,SEEK_END);
    size = ftell(fp);
    fseek(fp, 0 , SEEK_SET);
    
    cdm.lines = ((size-4)/8);
    fread(&cdm.begin, 4, 1, fp);
    strcpy(cdm.name,filename);
    for(i = 0; i < cdm.lines; i++){
    	fread(&cdm.program[i], 8, 1, fp);
    }
    return (cdm);

}
