//
//  main.c
//  Proj4
//
//  Created by Erika on 4/18/16.
//  Copyright (c) 2016 Erika. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>
#include "cache.h"


//set index bits, associativity, number of block bits and number of sets in the cache
int s, lines, b, rows;


int hitCounter, missCounter, evictCounter;
char * filename;

//each line in the cache holds a valid bit, the LRU and the tag
typedef struct cacheLine{
    int validBit;
    int LRU;
    uint64_t tag;
} cacheLineType;

//functions used to simulate the cache
void makeCache (cacheLineType ** cache, bool verbose);
void cacheStuff (cacheLineType ** cache, uint64_t setBit, uint64_t tag, bool verbose);
void help ();


int main(int argc, char ** argv) {
    
    //initializing counters
    hitCounter = 0;
    missCounter =0;
    evictCounter = 0;
    
    //initializing flags
    int sflag = 0, Eflag = 0, bflag = 0, tflag = 0;
    
    int c = 0;
    bool verbose = false;
    
    //parsing the command line
    while((c = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch (c) {
            case 'h':
                help();
                break;
            case 'v':
                verbose = true;
                break;
            case 's':
                sflag = 1;
                s = atoi (optarg);
                break;
            case 'E':
                Eflag = 1;
                lines = atoi (optarg);
                break;
            case 'b':
                bflag = 1;
                b = atoi (optarg);
                break;
            case 't':
                tflag =1;
                filename = optarg;
                break;
        }
    }
    
    //indicates what is missing from the command line
    if (sflag == 0){
        printf("Missing -s option");
        exit(1);
    }
    if (Eflag == 0) {
        printf("Missing -E option");
        exit(1);
    }
    if (bflag == 0)        {
        printf("Missing -b option");
        exit(1);
    }
    if (tflag == 0)        {
        printf("Missing -t option");
        exit(1);
    }
    
    if (verbose) {
        printf("file=%s\n", filename);
    }
    
    
    //gets the number of "rows" in the array, which is the number of sets
    rows = pow(2, s);
    
    //Create space in memory for the 2D array of structs. I used the following website to see how this was done:
    //http://stackoverflow.com/questions/3275381/how-to-implement-a-2-dimensional-array-of-struct-in-c
    cacheLineType **theCache = (cacheLineType **)malloc(rows *sizeof (cacheLineType *));
    for (int i = 0; i< rows; i++){
        theCache[i] = (cacheLineType *)malloc(lines *sizeof (cacheLineType));
        for (int j =0; j< lines; j++){
            //set all of the valid bits to 0  and the LRU bits to 1 in the beginning
            theCache[i][j].validBit = 0;
            theCache[i][j].LRU = 0;
        }
    }
    
    //creates the cache with the values in the command line then reads
    makeCache(theCache, verbose);
    
    printSummary (hitCounter, missCounter, evictCounter);
    return 0;
}

void makeCache(cacheLineType ** cache, bool verbose){
    
    //opens the file
    FILE* trace;
    trace = fopen(filename, "r");
    if (trace == NULL){
        perror("Error opening file\n");
        exit(0);
    }
    
    //maximum number of spaces each could be
    char address[16];
    char operation;
    char traceLine[21];
    
    uint64_t decimal;
    uint64_t setBit;
    uint64_t tag;
    //size of the tag
    int tagSize = 64 -s-b;
    
    //goes through each line in the file
    while(fgets(traceLine, 21, trace) != NULL){
        int nread = sscanf(traceLine, " %c %[^,]", &operation, address);
        if (nread != 2) {
            if (verbose) {
                printf("skipping due to nread...\n");
            }
            
            continue;
        }
        
        //converts the address to decimal
        decimal = strtoull (address, NULL, 16);
        
        //gets the setBit in the address
        setBit = (decimal<<tagSize)>>(tagSize+b);
        
        //gets the tag within the address
        tag = decimal>>(s+b);
        
        //do the following if the operation is not I
        if (operation != 'I'){
            
            //if the operation is M, do two operations
            if (operation == 'M'){
                
                //if the v flag is included, print out the necessary information
                if (verbose){
                    printf("%c %s ", operation, address);
                }
                
                //determines when to move things into the cache and when to evict things
                cacheStuff(cache, setBit, tag, verbose);
                
                if (verbose){
                    printf("hit");
                }
                hitCounter++;
                
                
                
                
                //spacing
                if (verbose){
                    printf("\n");
                }
                
                //if the operation is L or S then only do one operation
            }else{
                
                //if the v flag is included, print out the necessary information
                if (verbose){
                    printf("%c %s ", operation, address);
                }
                cacheStuff(cache, setBit, tag, verbose);
                
                //spacing
                if (verbose){
                    printf("\n");
                }
            }
            
        }
    }
    fclose(trace);
}

//determines when and what to put in the cache and when and what to evict
void cacheStuff (cacheLineType ** cache, uint64_t setBit, uint64_t tag, bool verbose){
    
    bool eviction = true;
    
    
    //go through every line and see if there is a hit and that the valid bit is not 0 then there will be a hit
    for (int i = 0; i<lines && eviction; i++){
        if (cache[setBit][i].tag == tag && cache[setBit][i].validBit != 0){
            hitCounter++;
            
            if (verbose){
                printf("hit ");
            }
            
            //when something is put in a certain line, set the LRU to the number of lines + 1
         
            cache[setBit][i].LRU = 10;
            
            //go through everything in the set and if the LRU is not 1, then subtract 1 from the LRU so
            // that the line that was just accessed has the highest LRU
            
                        for (int j = 0; j<lines; j++){
            
                                cache[setBit][j].LRU--;
        
                        }
            
            //will exit the for loop because nothing has been evicted
            eviction = false;
            break;
        }
        
        
        //go through every struct in the line
        for (int i = 0; i<lines && eviction; i++){
            
            //valid bit is 0 so this will be a miss and we can put what we want in this line
            if (cache[setBit][i].validBit == 0){
                missCounter++;
                
                if (verbose){
                    printf("miss ");
                }
                
                //now set valid bit to 1 and put things in cache
                cache[setBit][i].validBit = 1;
                cache[setBit][i].tag = tag;
                
                //when something is put in a certain line, set the LRU to the number of lines + 1
                
                cache[setBit][i].LRU = 10;
                
                //go through every line in the set and if the LRU is not 1, then subtract 1 from the LRU so
                // that the line that was just accessed has the highest LRU
                
                            for (int j = 0; j<lines; j++){
                               
                                    cache[setBit][j].LRU--;
                                
                            }
                
                //will exit the for loop because nothing has been evicted
                eviction = false;
                break;
            }
        }
        
    }
    
    // if the valid bit is not 0 and we did not find a match for the tag, then eviction will be true
    if (eviction){
        evictCounter++;
        missCounter ++;
        
        if (verbose){
            printf("miss eviction ");
        }
        
        int min = 0;
        for (int j = 0; j<lines; j++){
            if(cache[setBit][j].LRU<=cache[setBit][j+1].LRU){
                min = cache[setBit][j].LRU;
            }else{
                min =cache[setBit][j+1].LRU;
            }
        }
        
        /*
         int minLruFound = BIG NUMBEr, int thatIndex = -1
         
         for (lines in this set)
            if lru < minLruFound, minLRUFound = lru, thatIndex = thisIndex
         
         
         */
        
        //go through each line in the set
        for (int i = 0; i<lines; i++){
            
            
            //We want to evict the LRU, which will have a value of 1 because if it was accessed most recently it would have
            //been set to the number lines above, and if it was accessed in the middle the LRU would be somewhere between 1 and
            //the number of lines as a result of the if-else statement above
            if (cache[setBit][i].LRU == min){
                cache[setBit][i].validBit = 1;
                cache[setBit][i].tag = tag;
                cache[setBit][i].LRU = 10;
                
                //go through everything in the set and if the LRU is not 1, then subtract 1 from the LRU so
                // that the line that was just accessed has the highest LRU
                
                                for (int j = 0; j<lines; j++){
                                  
                                        cache[setBit][j].LRU--;
                                        //break;
                                    
                
                                }}
        }
    }
}

//if the help flag is in the command line print out the follwing information
void help(){
    printf("Usage: ./cachesim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf(" -h         Print this help message.\n");
    printf("-v         Optional verbose flag.\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file.\n");
    printf("Examples:\n");
    printf("linux>  ./cachesim-ref -s 4 -E 1 -b 4 -t traces/t1.trace\n");
    printf ("linux>  ./cachesim-ref -v -s 8 -E 2 -b 4 -t traces/t1.trace\n");
    
    exit(0);
}
