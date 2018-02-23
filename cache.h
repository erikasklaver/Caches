//
//  cache.h
//  Proj4
//
//  Created by Erika on 4/22/16.
//  Copyright (c) 2016 Erika. All rights reserved.
//


/*
 * cache.h - Prototypes for cache helper functions
 */

#ifndef CACHE_TOOLS_H
#define CACHE_TOOLS_H
/*
 * printSummary - This function provides a standard way for your cache
 * simulator to display its final hit and miss statistics
 */
void printSummary(int hits,  /* number of  hits */
                  int misses, /* number of misses */
                  int evictions); /* number of evictions */

#endif /* CACHE_TOOLS_H */
