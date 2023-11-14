#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <crypt.h>
#include "thread_crypt.h"

void print_help(void);

int main(int argc, char *argv[]) {
	int opt;
    	char * input_file = NULL;
    	char * output_file = NULL;
    	int algorithm = 0; // Default algorithm
    	int salt_length = 0; // Default salt length
    	int rounds = 5000; // Default rounds
    	long seed = 0; // Default seed
    	int num_threads = 1; // Default number of threads
    	int verbose = 0; // Default verbose mode
	
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

	if (intput_file == NULL) {
		fprintf(stderr, "No input file specified\n");
		print_help();
		exit(EXIT_FAILURE);	
	}

 
    // Initialize variables based on options and defaults
    
    
    // Read input file and perform necessary operations
    // Implement multi-threading for enhanced performance

    return 0;
}

void print_help(void) {
	printf("./thread_crypt ...\n");
	printf("\tOptions: i:o:hva:l:R:t:r:\n");
	printf("\t-i file	  input file name (required)");
	printf("\t-o file	  output file name (default stdout)");
	printf("\t-a #		  algorithm to use for hashing [0,1,5,6] (default 0 = DES)\n");
	printf("\t-l #		  length of salt (default 2 for DES, 8 for MD-5, 16 for SHA)\n");
	printf("\t-r #		  rounds to use for SHA-256, or SHA-512 (default 5000)\n");
	printf("\t-R #		  seed for rand() (default none)\n");
	printf("\t-t #		  number of threads to create (default 1)\n");
	printf("\t-v		  enable verbose mode\n");
	printf("\t-h		  helpful text\n");

}
