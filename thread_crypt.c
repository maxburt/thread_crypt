//Lab 3 for CS 333
//by Max Burt 11/17/23

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <crypt.h>
#include <string.h>
#include <pthread.h>
#include "thread_crypt.h"

//prints helpful text
void print_help(void);

//function that gets called in main
void process(const char * input_file, char * output_file, int algorithm, int salt_length, int rounds, int num_threads);

//creates setting value with prefix, rounds, salt, etc.
char * create_setting(int algorithm, int salth_length, int rounds);

//function that each thread executes
void * threadFunction(void * arg);

//the data which every thread needs to execute the crypt function
//and output result
struct thread_data {
	FILE * in_file;
	FILE * out_file;
	int algorithm;
	int salt_length;
	int rounds;
};

static int verbose = 0;

pthread_mutex_t rand_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
	int opt;
    	char * input_file = NULL;
    	char * output_file = NULL;
    	int algorithm = 0;
    	int salt_length = -1;
    	int rounds = 5000; 
    	long seed = 0; 
    	int num_threads = 1;
	
	// Parse command line options using getopt()
 	while ((opt = getopt(argc, argv, "i:o:a:l:r:R:t:vh")) != -1) {
        	switch (opt) {
			case 'h':
                                print_help();
                                exit(EXIT_SUCCESS);
            		case 'i':
                		input_file = optarg;
                		break;
            		case 'o':
                		output_file = optarg;
                		break;
            		case 'a':
                		algorithm = atoi(optarg);
                		break;
            		case 'l':
                		salt_length = atoi(optarg);
                		break;
            		case 'r':
                		rounds = atoi(optarg);
                		if (rounds < 1000) rounds = 1000;
                		if (rounds > 999999999) rounds = 999999999;
                		break;
            		case 'R':
                		seed = atol(optarg);
                		break;
            		case 't':
                		num_threads = atoi(optarg);
                		if (num_threads < 1) num_threads = 1;
                		if (num_threads > 20) num_threads = 20;
                		break;
            		case 'v':
                		verbose = 1;
                		break;
            		default:
                		fprintf(stderr, "Unknown option: %c\n", opt);
                		print_help();
                		exit(EXIT_FAILURE);
        	}
	}
	if (input_file == NULL) {
		fprintf(stderr, "must provide input file name\n");
		exit(EXIT_FAILURE);	
	}
	if (verbose == 1) {
		fprintf(stderr, "Rounds = %d\n", rounds);
	}
	
	if (seed < 0) seed = 0;
	
	srand((unsigned int)seed);

    	process(input_file, output_file, algorithm, salt_length, rounds, num_threads);
    
    return 0;
}

void print_help(void) {
	printf("./thread_crypt ...\n");
	printf("\tOptions: i:o:hva:l:R:t:r:\n");
	printf("\t-i file\t\tinput file name (required)\n");
	printf("\t-o file\t\toutput file name (default stdout)\n");
	printf("\t-a #\t\talgorithm to use for hashing [0,1,5,6] (default 0 = DES)\n");
	printf("\t-l #\t\tlength of salt (default 2 for DES, 8 for MD-5, 16 for SHA)\n");
	printf("\t-r #\t\trounds to use for SHA-256, or SHA-512 (default 5000)\n");
	printf("\t-R #\t\tseed for rand() (default none)\n");
	printf("\t-t #\t\tnumber of threads to create (default 1)\n");
	printf("\t-v\t\tenable verbose mode\n");
	printf("\t-h\t\thelpful text\n");
}

//returns a malloc'ed "setting" value for running crypt function
char * create_setting(int algorithm, int salt_length, int rounds) {
	int pref_length = 0;
	int idx;
	int idx2;
	char * setting = "";
	char * prefix = "";
	char str_rounds[15];	//holds string for rounds
	int str_rounds_length = 0;
	int setting_length = 0;
	
	//determine prefix
	if (algorithm == 0) prefix = "";
        else if (algorithm == 1) prefix = "$1$";
        else if (algorithm == 5) prefix = "$5$";
        else if (algorithm == 6) prefix = "$6$";
        else {
                fprintf(stderr, "Not a valid value for algorithm\n");
                exit(EXIT_FAILURE);
        }

	//length of prefix
	pref_length = strlen(prefix);

	//converts int rounds value to string	5000 -> "5000"
        snprintf(str_rounds, sizeof(str_rounds), "%d", rounds);
	
	str_rounds_length = strlen(str_rounds);

	str_rounds[str_rounds_length] = '$';	//5000 -> 5000$
						//
	str_rounds[str_rounds_length + 1] = '\0';	//null terminate string

	str_rounds_length = strlen(str_rounds);	

	//calculate length of salt and setting
	if (algorithm == 0) {
		if (salt_length < 0 || salt_length > 2) salt_length = 2;
		setting_length = pref_length + salt_length;
	}
	else if (algorithm == 1) {
		if (salt_length < 0 || salt_length > 8) salt_length = 8;
		setting_length = pref_length + salt_length;
	}
	else if (algorithm == 5 || algorithm == 6) {
		if (salt_length < 0 || salt_length > 16) salt_length = 16;
		setting_length = pref_length + 7 /*"rounds=*/ + str_rounds_length + salt_length; //3 + 7 + 5 + 16 = 31
	}
	
	//allocates space for prefix + salt + null character
	setting = malloc(setting_length + 1);

	//idx is index for setting char array
	idx = 0;	

	//set prefix chars
	while (idx < pref_length) {
		setting[idx] = prefix[idx];
		idx++;
	}

	if (algorithm == 5 || algorithm == 6) {
		char * add1 = "rounds=";
		idx2 = 0;

		//add rounds=
		while (idx < pref_length + 7) {
			setting[idx] = add1[idx2];
			idx++;
			idx2++;
		}
		idx2 = 0;

		//e.g. adds 5000$
		while (idx < pref_length + 7 + str_rounds_length) {
			setting[idx] = str_rounds[idx2];
			idx++;
			idx2++;
		}
	}

	//add random salt chars
	while (idx < setting_length) {
		setting[idx] = SALT_CHARS[rand() % (strlen(SALT_CHARS))];
		idx++;
	}

	//null terminate string
	setting[idx] = '\0';
	return setting;
}

void process(const char * input_file, char * output_file, int algorithm, int salt_length, int rounds, int num_threads) {
	FILE * in_file = NULL;	
	FILE * out_file = NULL;
	struct thread_data thread_info;
	pthread_t threads[num_threads];

	//step 1 OPEN in and  out files
	
	in_file = fopen(input_file, "r");

	if (in_file == NULL) {
        	perror("Error opening input file");
        	exit(EXIT_FAILURE);
    	}

	if (output_file != NULL) {
		out_file = fopen(output_file, "w");
		if (out_file == NULL) {
			perror("Error opening output file");
               	 	exit(EXIT_FAILURE);
		}
		else if (verbose == 1) {
			printf("Outputting data to file: %s\n", output_file);
		}
	}
	
        thread_info.in_file = in_file;
        thread_info.out_file = out_file;
        thread_info.algorithm = algorithm;
        thread_info.salt_length = salt_length;
        thread_info.rounds = rounds;

	if (verbose == 1) {
		fprintf(stderr, "Threads spun up\n");
	}

	//Step 2 create all the threads
	for (int i = 0; i < num_threads; ++i) {
        	if (pthread_create(&threads[i], NULL, threadFunction, (void *)&thread_info) != 0) {
			perror("Error creating threads\n");
			exit(EXIT_FAILURE);
		}
	}

	//Step 3 join threads after they finish
	for (int i = 0; i < num_threads; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Error joining threads\n");
			exit(EXIT_FAILURE);
		}
	}
	if (verbose == 1) {
        	fprintf(stderr, "Threads joined\n");
        }

	fclose(in_file);
	if (out_file) fclose(out_file);
}

void * threadFunction(void * arg) {
	struct thread_data * thread_info = (struct thread_data *) arg;
	char * hashed_password = "";	
	char * setting = "";		// holds setting i.e. (password:$5$rounds=5000$fhkndjbkflsahs3$)
        char line[256];			// buffer for getting lines from file
        struct crypt_data data;
	data.initialized = 0;

	while (1) {
		size_t len;
	
		//start mutex lock because i want each line to be read and
		//setting to be generated together without interruption	
		pthread_mutex_lock(&rand_mutex);
		
		//read line from thread_info->in_file
		if (fgets(line, sizeof(line), thread_info->in_file) == NULL) {
			pthread_mutex_unlock(&rand_mutex);	//if file is empty, unlock mutex and break out of loop
			break;
		} 

		//generate setting, setting is malloced'
		setting = create_setting(thread_info->algorithm, thread_info->salt_length, thread_info->rounds);
		
		pthread_mutex_unlock(&rand_mutex);	//unlock mutex after setting is created
		
		len = strlen(line);

                // Remove newline character from the end of the line
                if (line[len - 1] == '\n') {
                        line[len - 1] = '\0';
                }
	
		//add values to crypt_data struct
		strcpy(data.setting, setting);
		strcpy(data.input, line);

               	hashed_password = crypt_rn(line, setting, &data, (int)sizeof(data));
		
               	if (hashed_password == NULL) {
                       	fprintf(stderr, "Error in crypt_rn\n");
                       	exit(EXIT_FAILURE);
                }

		//output result
               	if (thread_info->out_file == NULL) {	// stdout
			printf("%s:%s\n", data.input, data.output);
		}
                else { 					//output to file
			fprintf(thread_info->out_file, "%s:%s\n", data.input, data.output);
		}

		//free setting and reset strings to NULL
               	free(setting);
               	setting = "";
		hashed_password = "";
        }
	pthread_exit(NULL);
} 
