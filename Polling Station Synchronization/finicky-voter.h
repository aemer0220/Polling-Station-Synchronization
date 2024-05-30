/* Alexandra Emerson
 *
 * CS 441/541: Finicky Voter 
 * Last Modified: 4/18/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include "semaphore_support.h"

/*****************************
 * Defines
 *****************************/

/*****************************
 * Structures
 *****************************/


/*****************************
 * Global Variables
 *****************************/

/* Numbers of each thread with defaults */
int num_booths;
int num_democrats;
int num_republicans;
int num_independents;
/* total number of voters and number in the polling station */
int total_voters;
int in_station_count;

/* line (for open booths) counts */
int republican_line;
int democrat_line;
int independent_line;
int total_line;

/* Counts those waiting to enter polling station */
int waiting_count;

/* Party Threads */
pthread_t *democrat_thread;
pthread_t *republican_thread;
pthread_t *independent_thread;

/* protect calls to printf */
semaphore_t print_mutex;

/* protect booths */
semaphore_t booth_sem;

/* barrier for entering station */
semaphore_t station_sem;

/* Allow voters to get in line without conflicts */
semaphore_t line_mutex;

/* protects total number of voters in line */
semaphore_t total_line_mutex;

/* Protects count */
semaphore_t count_sem;

/* Independents waiting to vote */
semaphore_t waiting_independents;

/* Democrats waiting to vote */
semaphore_t waiting_democrats;

/* Republicans waiting to vote */
semaphore_t waiting_republicans;

/* signals voters to vote once they are all in the polling station (turnstile)*/
semaphore_t vote_lock;

/* Buffer to indicate booth occupancy */
char *booth_buffer;

/*****************************
 * Function Declarations
 *****************************/

/*
 * Processes command line arguments
 *
 * Parameters:
 *   argc (from main)
 *   argv (from main)
 * Returns:
 *   integer array of arguments
 */
int * process_args(int argc, char **argv);

/* 
 * Determine whether input is an integer.
 *
 * Parameters:
 *   input: a string
 * Returns:
 *   0 if true
 *   1 if false
 */
int is_integer(char *input);

/* 
 * Prints the output header
 *
 * Parameters:
 *   None
 * Returns:
 *   None
 */
void print_header();

/* 
 * Prints the separator hyphens
 *
 * Parameters:
 *   None
 * Returns:
 *   None
 */
void print_separator();

/* 
 * Initializes the semaphores
 *
 * Parameters:
 *   None
 * Returns:
 *   None
 */
void create_sems();


/* 
 * Creates and Joins each voter thread
 *
 * Parameters:
 *   None
 * Returns:
 *   None
 */
void create_voters();
void join_voters();

/* semaphore methods */
void* republican(void *thread_id);
void* democrat(void *thread_id);
void* independent(void *thread_id);

/* 
 * Prints the middle part of the
 * output, the booths.
 *
 * Parameters:
 *   buffer (booth_buffer)
 *   num_of_booths (num_booths)
 * Returns:
 *   The final string
 */
char * booth_to_string(char *buffer, int num_of_booths);

/* 
 * Prints "Waiting for polling..."
 *
 * Parameters:
 *   t_name: formatted string
 *   including political party and thread_id 
 * Returns:
 *   None
 */
void print_waiting_open(char *t_name);

/* 
 * Prints "Entering the polling station"
 *
 * Parameters:
 *   t_name: formatted string
 *   including political party and thread_id 
 * Returns:
 *   None
 */
void print_entering(char *t_name);

/* 
 * Prints "Waiting on a voting booth"
 *
 * Parameters:
 *   t_name: formatted string
 *   including political party and thread_id 
 * Returns:
 *   None
 */
void print_waiting_booth(char *t_name);

/* 
 * Prints "Voting!"
 *
 * Parameters:
 *   t_name: formatted string
 *   including political party and thread_id 
 * Returns:
 *   None
 */
void print_voting(char *t_name);

/* 
 * Finds an open booth
 *
 * Parameters:
 *   None 
 * Returns:
 *   booth number on success
 *   -1 on failure
 */
int find_booth();

/* 
 * Prints "Leaving the polling station"
 *
 * Parameters:
 *   t_name: formatted string
 *   including political party and thread_id 
 * Returns:
 *   None
 */
void print_leaving(char *t_name);
