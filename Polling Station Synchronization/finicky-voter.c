/* Alexandra Emerson
 *
 * CS 441/541: Finicky Voter 
 * Last Modified: 4/18/2024
 */
#include "finicky-voter.h"

int main(int argc, char * argv[]) {
	int *args;
	int i;
	
	args = process_args(argc, argv);
	num_booths = args[0];
	num_democrats = args[1];
	num_republicans = args[2];
	num_independents = args[3];
	total_voters = num_democrats + num_republicans + num_independents; //total number of voters
   
	republican_line = 0;
	democrat_line = 0;
	independent_line = 0;
	total_line = 0;
	waiting_count = 0;
	in_station_count = 0;
	
	//Initialize buffer to fully empty
	booth_buffer = malloc(sizeof(char) * num_booths);
	for(i = 0; i < num_booths; i++){
		booth_buffer[i] = '.'; //'.' for empty.
	}

	create_sems(); //create semaphores 
	print_header();
	create_voters(); //create voter threads
	
	/* frees */
	free(booth_buffer);
	booth_buffer = NULL;
	free(republican_thread);
	republican_thread = NULL;
	free(democrat_thread);
	democrat_thread = NULL;
	free(independent_thread);
	independent_thread = NULL;
  
	return 0;
}

int * process_args(int argc, char * argv[]){
	int i, *args;
	
	if (argc > 5){
		fprintf(stderr, "ERROR: Invalid number of arguments.\n");
		exit(-1);
	}
   
	args = (int *) malloc(sizeof(int) * 4);
	//defaults
	args[0] = 10;
	args[1] = 5;
	args[2] = 5;
	args[3] = 5;

	for(i = 1; i < argc; i++){
		if ((is_integer(argv[i]) == 0) || (argv[i] <= 0)){
			fprintf(stderr, "ERROR: Parameters incorrectly specified.\n");
			exit(-1);
		}
		args[i-1] = strtol(argv[i], NULL, 10); //convert to int and put in array	
	}
	return args;
}

void create_sems(){
	semaphore_create(&print_mutex, 1);
	semaphore_create(&booth_sem, num_booths);
	semaphore_create(&station_sem, 1);
	semaphore_create(&line_mutex, 1);
	semaphore_create(&total_line_mutex, 1);
	semaphore_create(&waiting_independents, 0);
	semaphore_create(&waiting_democrats, 0);
	semaphore_create(&waiting_republicans, 0);
	semaphore_create(&count_sem, 1);
	semaphore_create(&vote_lock, 0);
	srandom(time(NULL)); //seed random number generator
}

void create_voters(){
	semaphore_wait(&station_sem);
	int i, rc;
	
	//create republican threads
	republican_thread = (pthread_t *) malloc(sizeof(pthread_t) * num_republicans);
	for(i = 0; i < num_republicans; i++){
		rc = pthread_create(&(republican_thread[i]), NULL, republican, (void*)(intptr_t)i);
		if (rc != 0){
			fprintf(stderr, "ERROR:Failed to create thread.\n");
			exit(-1);
		}
	}

	//create democrat threads
	democrat_thread = (pthread_t *) malloc(sizeof(pthread_t) * num_democrats);
	for(i = 0; i < num_democrats; i++){
		rc = pthread_create(&(democrat_thread[i]), NULL, democrat, (void*)(intptr_t)i);
		if (rc != 0){
			fprintf(stderr, "ERROR:Failed to create thread.\n");
			exit(-1);
		}
	}

	//create independent threads
	independent_thread = (pthread_t *) malloc(sizeof(pthread_t) * num_independents);
	for(i = 0; i < num_independents; i++){
		rc = pthread_create(&(independent_thread[i]), NULL, independent, (void*)(intptr_t)i);
		if (rc != 0){
			fprintf(stderr, "ERROR:Failed to create thread.\n");
			exit(-1);
		}
	}
	
	sleep(2); //sleep for 2 seconds
	semaphore_post(&station_sem); //signal polling station
	join_voters();
	print_separator();
	
	semaphore_destroy(&print_mutex);
	semaphore_destroy(&booth_sem);
	semaphore_destroy(&station_sem);
	semaphore_destroy(&line_mutex);
	semaphore_destroy(&total_line_mutex);
	semaphore_destroy(&waiting_independents);
	semaphore_destroy(&waiting_democrats);
	semaphore_destroy(&waiting_republicans);
	semaphore_destroy(&count_sem);
}

/* join threads */
void join_voters(){
	int i;
	for (i = 0; i < num_democrats; i++){
		pthread_join(democrat_thread[i], NULL);
	}
	for (i = 0; i < num_republicans; i++){
		pthread_join(republican_thread[i], NULL);
	}
	for (i = 0; i < num_independents; i++){
		pthread_join(independent_thread[i], NULL);
	}
}

void* republican(void *tid){
	int i, j, k;
	int thread_id = (intptr_t)tid;
	char thread_name[25];
	char thread_in_booth[25]; //for voting print method
	
	sprintf(thread_name, "Republican %4d       ", thread_id); //header used for printing methods
 	
	print_waiting_open(thread_name);          //waiting for polling station to open
   
	semaphore_wait(&station_sem); 
	semaphore_post(&station_sem);             //station has opened

	print_entering(thread_name);              //entering station

	if(in_station_count == total_voters){
		semaphore_post(&vote_lock);           //release turnstile if you're the last voter in
	}

	usleep(random() % 50000);                 //registration

	semaphore_wait(&vote_lock);               //wait for all voters to enter
	semaphore_post(&vote_lock);               //signals voters can vote

	semaphore_wait(&line_mutex);              //allows voter to get in their own party's line
	republican_line++;                        //republican gets in their own line
	
	if(democrat_line > 0){                    // if democrats in line
		semaphore_wait(&waiting_republicans); //wait until democrats signal they can wait for a booth
	}
	
	print_waiting_booth(thread_name);         //waiting for a booth
	
	semaphore_wait(&total_line_mutex);
	total_line++;                             //increment total voters in line(s)
	semaphore_post(&total_line_mutex);
	semaphore_post(&line_mutex);              //signal other republicans can get in line

	semaphore_wait(&booth_sem);
	i = find_booth();                         //find voting booth
	if(i == -1){
		fprintf(stderr, "ERROR: Unable to find booth\n");
		exit(-1);
	}
	
	booth_buffer[i] = 'R';                    //occupy booth
	
	sprintf(thread_in_booth, "Republican %4d in %2d ", thread_id, i); //create header
	
	print_voting(thread_in_booth);            //Voting!
	
	usleep(random()%100000);                  //time for voting

	booth_buffer[i] = '.';
	print_leaving(thread_name);               //leaving polling station
	semaphore_post(&booth_sem);
	
	semaphore_wait(&total_line_mutex);
	total_line--;                             //voter has left, decrement line
	
	int open_spots = num_booths-total_line;   //if any open spots, let independents in
	for(j = 0; j < open_spots; j++){
		semaphore_post(&waiting_independents); //signal independents to vote based on how many spots open
	}
	semaphore_post(&total_line_mutex);

	semaphore_wait(&count_sem); //protect republican line count
	republican_line--;

	if (republican_line == 0){ //if last republican has left line
		for(k = 0; k < democrat_line; k++){
			semaphore_post(&waiting_democrats); //signal democrats can vote
		}
   	}
	semaphore_post(&count_sem);

	pthread_exit(NULL);
}

void* democrat(void *tid){
	int i, j, k;
	int thread_id = (intptr_t)tid;
	char thread_name[25];
	char thread_in_booth[25];               //for voting print method
	
	sprintf(thread_name, "Democrat   %4d       ", thread_id);

	print_waiting_open(thread_name);        //waiting for station to open

	semaphore_wait(&station_sem);
	semaphore_post(&station_sem);           //station has opened

	print_entering(thread_name);            //entering station
	
  	if(in_station_count == total_voters){
		semaphore_post(&vote_lock);         //release turnstile if you're the last voter in
	}

	usleep(random() % 50000);               //registration

	semaphore_wait(&vote_lock);             //wait for all voters to enter
	semaphore_post(&vote_lock);             //signal voters can vote
	
	semaphore_wait(&line_mutex);            //allows voter to get in their own party's line
	
	democrat_line++;                        //democrat gets in their own line
	
	if(republican_line > 0){                // if republicans in line
		semaphore_wait(&waiting_democrats); //wait until coast is clear
	}
	
	print_waiting_booth(thread_name);       //waiting for open booth
	
	semaphore_wait(&total_line_mutex);
	total_line++;                           //increment total voters in line
	semaphore_post(&total_line_mutex);
	
	semaphore_post(&line_mutex);            //signal other democrats can get in line

	semaphore_wait(&booth_sem);
	i = find_booth();                       //find voting booth
	if(i == -1){
		fprintf(stderr, "ERROR: Unable to find booth\n");
		exit(-1);
	}
	
	booth_buffer[i] = 'D';                  //Occupy booth
	
	sprintf(thread_in_booth, "Democrat   %4d in %2d ", thread_id, i); //create header for printing
	
	print_voting(thread_in_booth);           //Voting!
	
	usleep(random()%100000);                 //time for voting
	
	booth_buffer[i] = '.';
	print_leaving(thread_name);              //leaving polling station
	semaphore_post(&booth_sem);
	
	semaphore_wait(&total_line_mutex);
	total_line--;                            //voter has left, decrement line
	
	int open_spots = num_booths-total_line;    //if any open spots, let independents in
	for(j = 0; j < open_spots; j++){
		semaphore_post(&waiting_independents); //signal independents to vote based on how many spots open
	}
	semaphore_post(&total_line_mutex);

	semaphore_wait(&count_sem);                  //protect democrat line count
	democrat_line--;
	
	if (democrat_line == 0){                     //if last democrat has left line
		for(k = 0; k < republican_line; k++){
			semaphore_post(&waiting_republicans); //signal republicans can vote
		}
   	}
	semaphore_post(&count_sem);

	pthread_exit(NULL);
}

void* independent(void *tid){
	int i, j;
	int thread_id = (intptr_t)tid;
	char thread_name[25];
	char thread_in_booth[25]; //for voting print method
	
	sprintf(thread_name, "Independent%4d       ", thread_id);
   
	print_waiting_open(thread_name);            //waiting for station to open

	semaphore_wait(&station_sem);
	semaphore_post(&station_sem);               //station has opened

	print_entering(thread_name);                //entering station

	if(in_station_count == total_voters){
		semaphore_post(&vote_lock);             //release turnstile if you're the last voter in
	}

	usleep(random() % 50000);                   //registration

	semaphore_wait(&vote_lock);                 //wait for all voters to enter
	semaphore_post(&vote_lock);                 //signal voters can vote

	if (total_line == num_booths){              //if line for a booth is full, wait. Prevents cutting in line
		semaphore_wait(&line_mutex);
		semaphore_wait(&waiting_independents);

		semaphore_wait(&total_line_mutex);
		total_line++;
		semaphore_post(&total_line_mutex);
		
		print_waiting_booth(thread_name);

		semaphore_post(&line_mutex);
	}

	else{                                       //else, there's room, no need to wait
		semaphore_wait(&total_line_mutex);
		total_line++;
		semaphore_post(&total_line_mutex);
		print_waiting_booth(thread_name);
	}

	semaphore_wait(&booth_sem);
	i = find_booth();                       //find voting booth
	if(i == -1){
		fprintf(stderr, "ERROR: Unable to find booth.\n");
		exit(-1);
	}
	
	booth_buffer[i] = 'I';
	sprintf(thread_in_booth, "Independent%4d in %2d ", thread_id, i);

	print_voting(thread_in_booth);           //Voting!
	
	usleep(random()%100000);                 //time for voting
	
	booth_buffer[i] = '.';
	print_leaving(thread_name);              //leaving polling station
	
	semaphore_wait(&total_line_mutex);
	total_line--;                            //voter has left, decrement line
	
	int open_spots = num_booths-total_line;
	for(j = 0; j < open_spots; j++){
		semaphore_post(&waiting_independents); //signal independents to vote based on how many spots open
	}
	semaphore_post(&total_line_mutex);

	semaphore_post(&booth_sem);

	pthread_exit(NULL);
}

void print_waiting_open(char *t_name){
	semaphore_wait(&print_mutex);
	waiting_count++;
	char *msg = "Waiting for polling station to open...";
	char *booth_str = booth_to_string(booth_buffer, num_booths);
	printf("%s %s %s\n", t_name, booth_str, msg);
	free(booth_str);
	fflush(stdout);

	if(waiting_count == total_voters){
		print_separator();
	}
	semaphore_post(&print_mutex);
}

void print_entering(char *t_name){
	semaphore_wait(&print_mutex);
	in_station_count++;
	char *msg = "Entering the polling station";
	char *booth_str = booth_to_string(booth_buffer, num_booths);
	printf("%s %s %s\n", t_name, booth_str, msg);
	free(booth_str);
	fflush(stdout);
	semaphore_post(&print_mutex);
}

void print_waiting_booth(char *t_name){
	semaphore_wait(&print_mutex);
	char *msg = "Waiting on a voting booth";
	char *booth_str = booth_to_string(booth_buffer, num_booths);
	printf("%s %s %s\n", t_name, booth_str, msg);
	free(booth_str);
	fflush(stdout);
	semaphore_post(&print_mutex);
}

void print_voting(char *t_name){
	semaphore_wait(&print_mutex);
	char *msg = "Voting!";
	char *booth_str = booth_to_string(booth_buffer, num_booths);
	printf("%s %s %s\n", t_name, booth_str, msg);
	free(booth_str);
	fflush(stdout);
	semaphore_post(&print_mutex);
}

int find_booth(){
	int i;
	for(i = 0; i < num_booths; i++){
		if (booth_buffer[i] == '.'){ //both is available
			return i;
		}
	}
	return -1;
}

 void print_leaving(char *t_name){
	semaphore_wait(&print_mutex);
	char *msg = "Leaving the polling station";
	char *booth_str = booth_to_string(booth_buffer, num_booths);
	printf("%s %s %s\n", t_name, booth_str, msg);
	free(booth_str);
	fflush(stdout);
	semaphore_post(&print_mutex);
 }

int is_integer(char *input){
	size_t length = strlen(input);
	int i;
	
	for(i = 0; i < length; i++){
		if (!isdigit(input[i])){ //if a non-digit character is called, return 0 (false)
			return 0; 
		}
	}
	return 1;
}

void print_header(){
	printf("Number of Voting Booths   :   %d\n", num_booths);
	printf("Number of Democrat        :   %d\n", num_democrats);
	printf("Number of Republican      :   %d\n", num_republicans);
	printf("Number of Independent     :   %d\n", num_independents);
	print_separator();
}

void print_separator(){
	int i;
	int num = num_booths * 3 + 9; //3 for each booth, plus 4 on each end. 1 more for null termination.
	char hyphens[num];
	for(i = 0; i < num; i++){
		hyphens[i] = '-';
	}
	hyphens[num-1] = '\0';
	printf("-----------------------+%s+--------------------------------\n", hyphens);
}

char * booth_to_string(char *buffer, int num_of_booths){
	int i;
	int length = num_of_booths * 3 + 11; //3 for each booth, plus 5 on each end. 1 more for null termination
	char print_str[length];
	sprintf(print_str, "|->  ");

	for (i = 0; i < num_of_booths; i++){
		sprintf(print_str, "%s[%c]", print_str, buffer[i]);
	}

	sprintf(print_str, "%s  <-|", print_str);
	return strdup(print_str);
}
