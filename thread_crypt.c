//Lab 3 for CS 333
//by Max Burt 11/17/23

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <getopt.h>
#include <crypt.h>
#include <string.h>
#include "thread_crypt.h"

void print_help(void);
void process(const char * input_file, char * output_file, int algorithm, int salt_length, int rounds);
char * create_setting(int algorithm, int salth_length, int rounds);

static int verbose = 0;

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
		fprintf(stderr, "Running thread_crypt...\n");
	}
	
	if (seed < 0) seed = 0;
	
	if (output_file == NULL && verbose == 1) {
		fprintf(stderr, "Outputting to stdout\n");
	}
	
	srand((unsigned int)seed);

       	//process the input file and send it to output file	
    	process(input_file, output_file, algorithm, salt_length, rounds);
    
	
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

	//calculate length for the setting
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

void process(const char * input_file, char * output_file, int algorithm, int salt_length, int rounds) {
	FILE * file = fopen(input_file, "r");		
	FILE * out_file = NULL;
	char * setting = "";
       	char line[256]; // buffer for lines of input_file
	char result[512]; // buffer for hashed outputs
	struct crypt_data data;
	
	if (file == NULL) {
        	perror("Error opening input file");
        	exit(EXIT_FAILURE);
    	}

	//open output file, if given at command line
	if (output_file != NULL) out_file = fopen(output_file, "w");

        if (output_file != NULL && out_file == NULL) {
                perror("Error opening output file");
                exit(EXIT_FAILURE);
        }
	else if (out_file != NULL && verbose == 1) {
		printf("Outputting data to file: %s\n", output_file);
	}

    	data.initialized = 0;
	
	while (fgets(line, sizeof(line), file) != NULL) {
        	size_t len = strlen(line);
        	char * hashed_password = NULL;

		// Remove newline character from the end of the line
		if (line[len - 1] == '\n') {
            		line[len - 1] = '\0';
        	}

		setting = create_setting(algorithm, salt_length, rounds);	//salt is malloced and appropriate prefix is added
        	hashed_password = crypt_rn(line, setting, &data, (int)sizeof(data));
	
		if (hashed_password == NULL) {
    			fprintf(stderr, "Error in crypt_rn\n");
			exit(EXIT_FAILURE);
		}
        
		// Print the plaintext password and the corresponding hashed password
        	snprintf(result, sizeof(result), "%s:%s", line, hashed_password);
        	
		if (output_file == NULL) printf("%s\n", result);
		else fprintf(out_file, "%s\n", result);

		free(setting);
		setting = "";
    	}	
	fclose(file);
	if (out_file) fclose(out_file);
}
