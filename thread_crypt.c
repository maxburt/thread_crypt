#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <crypt.h>
#include <string.h>
#include "thread_crypt.h"

void print_help(void);
void process(const char * input_file, char * output_file, int algorithm, int salt_length);
char * create_setting(int algorithm, int salth_length);

int main(int argc, char *argv[]) {
	int opt;
    	char * input_file = NULL;
    	char * output_file = NULL;
    	int algorithm = 0;
    	int salt_length = -1;
    	int rounds = 5000; 
    	long seed = 0; 
    	int num_threads = 1;
    	int verbose = 0;
	
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
		fprintf(stderr, "Running thread_crypt...\n");
	}
	
	if (seed < 0) seed = 0;
	
	if (output_file == NULL && verbose == 1) {
		fprintf(stderr, "outputting to stdout\n");
	}
	
	srand((unsigned int)seed);

       	//process the input file and send it to output file	
    	process(input_file, output_file, algorithm, salt_length);
    
	
	//Implement multi-threading for enhanced performance

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
char * create_setting(int algorithm, int salt_length) {
	
	int pref_length = 0;
	int idx = 0;
	char * setting = "";
	char * prefix = "";

	if (algorithm == 0) prefix = "";
        else if (algorithm == 1) prefix = "$1$";
        else if (algorithm == 5) prefix = "$5$";
        else if (algorithm == 6) prefix = "$6$";
        else {
                fprintf(stderr, "Not a valid value for algorithm\n");
                exit(EXIT_FAILURE);
        }

	pref_length = strlen(prefix);

	//default salt length set to -1
	if (algorithm == 0 && (salt_length < 0 || salt_length > 2)) salt_length = 2;
	else if (algorithm == 1 && (salt_length < 0 || salt_length > 8)) salt_length = 8;
	else if (algorithm == 5 && (salt_length < 0 || salt_length > 16)) salt_length = 16;
	else if (algorithm == 6 && (salt_length < 0 || salt_length > 16)) salt_length = 16;
	
	//allocates space for prefix plus salt
	setting = malloc(pref_length + salt_length + 1);
	
	//set first characters to prefix
	while (idx < pref_length) {
		setting[idx] = prefix[idx];
		idx++;
	}
	//set rest of characters to salt
	while (idx < salt_length + pref_length) {
		setting[idx] = SALT_CHARS[rand() % (strlen(SALT_CHARS))];
		idx++;
	}

	setting[idx] = '\0';
	return setting;
}

void process(const char * input_file, char * output_file, int algorithm, int salt_length) {
	FILE * file = fopen(input_file, "r");		
	char * setting = "";
       	char line[256]; // buffer for lines of input_file
	char result[512]; // buffer for hashed outputs
	struct crypt_data data;
	
	if (file == NULL) {
        	perror("Error opening input file");
        	exit(EXIT_FAILURE);
    	}

    	data.initialized = 0;
	
	while (fgets(line, sizeof(line), file) != NULL) {
        	size_t len = strlen(line);
        	char * hashed_password = NULL;

		// Remove newline character from the end of the line
		if (line[len - 1] == '\n') {
            		line[len - 1] = '\0';
        	}

		setting = create_setting(algorithm, salt_length);	//salt is malloced and appropriate prefix is added

        	hashed_password = crypt_rn(line, setting, &data, (int)sizeof(data));
	
		if (hashed_password == NULL) {
    			fprintf(stderr, "Error in crypt_rn\n");
			exit(EXIT_FAILURE);
		}
        
		// Print the plaintext password and the corresponding hashed password
        	snprintf(result, sizeof(result), "%s:%s", line, hashed_password);
        	printf("%s\n", result);

		free(setting);
		setting = "";
    	}	
}

//make new crypto here
//GOONCOIN
