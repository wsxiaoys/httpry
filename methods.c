/*

  ----------------------------------------------------
  httpry - HTTP logging and information retrieval tool
  ----------------------------------------------------

  Copyright (c) 2005-2008 Jason Bittel <jason.bittel@gmail.com>

*/

/*
   The methods data structure is stored as a dynamic array of
   strings. The array is allocated at run-time (in BLOCKSIZE
   chunks) to allow the array to handle an arbitrary number of
   methods to parse without significant wasted space. It's
   possible that a tree or similar structure could be more
   efficient in some circumstances, but given the low number
   of methods used in most runs, this method is simple and
   effective.
*/

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "methods.h"
#include "utility.h"

#define BLOCKSIZE 4

int insert_method(char *str);

static char **methods = NULL;

/* Parse and insert methods from input string */
void parse_methods_string(char *str) {
        char *method, *tmp, *i;
        int num_methods = 0;

#ifdef DEBUG
        ASSERT(str);
#endif

        if (strlen(str) == 0)
                LOG_DIE("Empty methods string provided");

        /* Make a temporary copy of the string so we don't modify the original */
        if ((tmp = malloc(strlen(str) + 1)) == NULL)
                LOG_DIE("Cannot allocate memory for methods string buffer");
        strcpy(tmp, str);

        for (i = tmp; (method = strtok(i, ",")); i = NULL) {
                method = str_strip_whitespace(method);
                method = str_tolower(method);

                if (strlen(method) == 0) continue;
                if (insert_method(method)) num_methods++;
        }

        free(tmp);

        if (num_methods == 0)
                LOG_DIE("No valid methods found in string");

#ifdef DEBUG
        int methods_cnt = 0;
        int blocks_cnt = 1;
        char **j;

        for (j = methods; *j; j++) {
                methods_cnt++;

                if (methods_cnt > (BLOCKSIZE * blocks_cnt)) blocks_cnt++;
        }

        PRINT("----------------------------");
        PRINT("Block size:         %d", BLOCKSIZE);
        PRINT("Block count:        %d", blocks_cnt);
        PRINT("Total slots:        %d", (BLOCKSIZE * blocks_cnt));
        PRINT("Methods inserted:   %d", methods_cnt);
        PRINT("Empty slots:        %d", (BLOCKSIZE * blocks_cnt) - methods_cnt);
        PRINT("----------------------------");
#endif

        return;
}

/* Insert a new method into the array */
int insert_method(char *method) {
        static char **mv, **tmp;
        static int size = 0;

#ifdef DEBUG
        ASSERT(method);
        ASSERT(strlen(method) > 0);
#endif

        /* Initialize the methods array if necessary */
        if (!methods) {
                if ((methods = (char **) malloc(BLOCKSIZE * sizeof(char *))) == NULL) {
                        LOG_DIE("Cannot malloc memory for methods array");
                }

                mv = methods;
                *mv = NULL;
        }

        /* Check if method has already been inserted */
        if (is_request_method(method)) {
                WARN("Method '%s' already provided", method);
                return 0;
        }

        /* Insert new method into array */
        if ((*mv = (char *) malloc(strlen(method) + 1)) == NULL) {
                *mv = NULL;
                free_methods();
                LOG_DIE("Cannot malloc memory for method");
        }
        strcpy(*mv, method);

        /* Resize the methods array as necessary */
        if (++size % BLOCKSIZE == 0) {
                tmp = realloc(methods, ((size + BLOCKSIZE) * sizeof(char *)));
                if (!tmp) {
                        *mv = NULL;
                        free_methods();
                        LOG_DIE("Cannot realloc memory for methods array");
                }
                methods = tmp;
                mv = methods + size - 1;
        }

        /* One NULL slot is required at the end of the list
           for use as a loop terminator */
        mv++;
        *mv = NULL;

        return 1;
}

/* Search parameter string for a matching method */
int is_request_method(const char *str) {
        char **i;

#ifdef DEBUG
        ASSERT(methods);
        ASSERT(str);
#endif

        if (strlen(str) == 0) return 0;

        for (i = methods; *i; i++) {
                if (str_compare(str, *i, strlen(*i)) == 0) return 1;
        }

        return 0;
}

/* Free all allocated memory for array; only called at
   program termination */
void free_methods() {
        char **i;

        if (!methods) return;

        for (i = methods; *i; i++) {
                free(*i);
        }

        free(methods);

        return;
}