#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  //intptr_t
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "customers.h"

void *customer_entry(void * cus_info);
void *checkin();
double getCurrentSimulationTime();

//Queues
cQueue* economy;
cQueue* business;

//Gloval Variables
struct timeval start_time;
int num_customer; // Total Number of customers
int customerList[2]; //Count of customer and business
int free_clerks = 5; // Number of free clerks
int clerk_status[5] = {0,0,0,0,0}; //0 -> Available, 1 -> Busy
double waiting_times[2]; //0 -> Economy, 1 -> Business

//Initialize Mutex
pthread_mutex_t economy_lock;
pthread_mutex_t business_lock;
pthread_mutex_t clerk_status_lock;
pthread_mutex_t num_clerks_lock;
pthread_mutex_t waiting_times_lock;

//convar
pthread_cond_t convar = PTHREAD_COND_INITIALIZER;


int main(int argc, char *argv[]){
	//check commands passed
	if (argc < 2 ){
		printf("Enter Input File.\n");
		return 0;
	}
	//check for text file
    char *extension = strchr(argv[1], '.');
	if (!extension || (strcmp(extension, ".txt") != 0)){
		printf("Error, file must be .txt\n");
		return 0;
	}

	if (pthread_mutex_init(&economy_lock, NULL) != 0){
        fprintf(stderr, "Failed to initialize Mutex\n");
        exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(&business_lock, NULL) != 0){
        fprintf(stderr, "Failed to initialize Mutex\n");
        exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(&clerk_status_lock, NULL) != 0){
        fprintf(stderr, "Failed to initialize Mutex\n");
        exit(EXIT_FAILURE);
	}

	if (pthread_mutex_init(&num_clerks_lock, NULL) != 0){
        fprintf(stderr, "Failed to initialize Mutex\n");
        exit(EXIT_FAILURE);
	}


	if (pthread_mutex_init(&waiting_times_lock, NULL) != 0){
        fprintf(stderr, "Failed to initialize Mutex\n");
        exit(EXIT_FAILURE);
	}

	//initialize Queues
	economy = createQueue();
	business = createQueue();

	// Read customer information from txt file and store them in the structure you created 
	FILE *input = fopen(argv[1], "r");
	if (!input){
		fprintf(stderr, "File Error\n");
		fclose(input);
		exit(EXIT_FAILURE);
	}

	char line[256];
	fgets(line, sizeof(line), input);
	num_customer = atoi(line);
	struct customer customers[num_customer];
	pthread_t customersThreads[num_customer];
	// Insert Customer Data into an array of customers
	int i = 0;
	while (fgets(line, sizeof(line), input)){
		customer newC;
		char *token = strtok(line, ":,\n");
		newC.cid = atoi(token);

		token = strtok(NULL, ":,\n");
		newC.class_type = atoi(token);
		customerList[newC.class_type]++;//keep count of number of business and economy

		token = strtok(NULL, ":,\n");
		newC.arrival_time = atoi(token);
		
		token = strtok(NULL, ":,\n");
		newC.service_time = atoi(token);

		customers[i++] = newC;
	}
	
	fclose(input);
	
	fprintf(stdout, "\nCheck-in Opens\n");
	sleep(1);
	gettimeofday(&start_time, NULL);

	//create customer threadZ
	for(int j = 0; j < num_customer; j++){ // number of customers
		//printf("%d\n", j);
		if (pthread_create(&customersThreads[j], NULL, customer_entry, (void *) &customers[j]) != 0){
			fprintf(stderr, "Error in pthread_create\n");
		}	
	}
	for (int k = 0; k < num_customer; k++){
		pthread_join(customersThreads[k], NULL);
	}
	
	fprintf(stdout, "\nAll Customers Served\n\n");

	//calculate waiting times
	double average = (waiting_times[0] + waiting_times[1]) / (customerList[1] + customerList[0]); // total waiting time / total customers
	double economy_average = waiting_times[0] / customerList[0];
	double business_average = waiting_times[1] / customerList[1];	

	fprintf(stdout, "The average waiting time for all customers in the system is: %.2f seconds.\n", average);
	fprintf(stdout, "The average waiting time for all business-class customers is: %.2f seconds.\n", business_average);
	fprintf(stdout, "The average waiting time for all economy-class customers is: %.2f seconds.\n", economy_average);


	free(business);
	free(economy);	
	
	return 0;
}
void * customer_entry(void * cus_info){
	struct customer * currentCustomer = (struct customer *) cus_info;
	//Sleep until customer arrives	
	usleep(100000 * currentCustomer->arrival_time);
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", currentCustomer->cid);
	
	/* Enqueue operation: get into either business queue or economy queue by using p_myInfo->class_type*/
	double cur_time = getCurrentSimulationTime();	
	if (currentCustomer->class_type == 1){
		pthread_mutex_lock(&business_lock);
		enQueue(business, currentCustomer, cur_time);
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", currentCustomer->class_type, business->len);
		pthread_mutex_unlock(&business_lock);
	}
	else if(currentCustomer->class_type == 0){
		pthread_mutex_lock(&economy_lock);
		enQueue(economy, currentCustomer, cur_time);
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", currentCustomer->class_type, economy->len);
		pthread_mutex_unlock(&economy_lock);
	}

	//Begin Checkin Process
	checkin();	
	return NULL;

}

void * checkin(){

	pthread_mutex_lock(&num_clerks_lock);
	//Check for free clerks, if none wait for convar
	if(free_clerks == 0){
		pthread_cond_wait(&convar, &num_clerks_lock);
	}
	//mark clerk as busy
	free_clerks--;
	pthread_mutex_unlock(&num_clerks_lock);

	pthread_mutex_lock(&clerk_status_lock);
	//Find and occupy available clerk
	int current_clerk = 0;
	for (int i = 0; i<5; i++){
		if (clerk_status[i]==0){
			current_clerk = i + 1; //get Clerk Number as opposed to Index
			clerk_status[i] = 1; //busy
			break;
		}
	}
	pthread_mutex_unlock(&clerk_status_lock); //done using clerk status
	
	pthread_mutex_lock(&business_lock);
	pthread_mutex_lock(&economy_lock);
	cNode *current_customer; // Current Customer being served
	while(business->front != NULL || economy->front != NULL){
	//serve business customers
		if (business->front!=NULL){
			current_customer = deQueue(business);
			break;
		}

		else if (economy->front != NULL){
			current_customer = deQueue(economy);
			break;
		}
	}

		pthread_mutex_unlock(&business_lock);
		pthread_mutex_unlock(&economy_lock);
		double time_complete = getCurrentSimulationTime();
		double wait_time = time_complete - current_customer->entry_time;
		//Add wait time to counter
		pthread_mutex_lock(&waiting_times_lock);
		waiting_times[current_customer->cid->class_type] += wait_time;
		pthread_mutex_unlock(&waiting_times_lock);

		fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n", time_complete, current_customer->cid->cid, current_clerk);
		//sleep during customer being served
		usleep(current_customer->cid->service_time * 100000);

		double end_time = getCurrentSimulationTime(); 
		fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", end_time, current_customer->cid->cid, current_clerk);
	       	
		pthread_mutex_lock(&clerk_status_lock);
		clerk_status[current_clerk-1] = 0; //free clerk	
		pthread_mutex_unlock(&clerk_status_lock);

		// Increase count of available clerks
		pthread_mutex_lock(&num_clerks_lock);
		free_clerks++;
		pthread_cond_signal(&convar);
		pthread_mutex_unlock(&num_clerks_lock);


	free(current_customer);

	return NULL;

}

//from sample_gettimeofday.c
double getCurrentSimulationTime(){
	
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	//pthread_mutex_lock(&start_time_mtex); you may need a lock here
	init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
	//pthread_mutex_unlock(&start_time_mtex);
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;
}
