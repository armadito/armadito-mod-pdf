/***

Copyright (C) 2015 - 2017 Teclib'

This file is part of Armadito module PDF.

Armadito module PDF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module PDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module PDF.  If not, see <http://www.gnu.org/licenses/>.

***/


#include "libarmadito-pdf/armaditopdf.h"
#include "libarmadito-pdf/armaditopdf/osdeps.h"
#include "libarmadito-pdf/armaditopdf/log.h"
#include "libarmadito-pdf/armaditopdf/errors.h"
#include "libarmadito-pdf/armaditopdf/parsing.h"
#include "getopt.h"

#define TOOL_VERSION "0.13.2"


struct scan_options {
	char *path_to_scan;
	enum log_level log_level;
};


static struct option cli_option_def[] = {
	{"help",        no_argument, 0, 'h'},
	{"version",     no_argument, 0, 'v'},
	{"log-level",   required_argument, 0, 'l'},
	{0, 0, 0, 0}
};


void Usage(){

	fprintf(stderr, "usage: armadito-pdf [options] FILE\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Armadito PDF scanner\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  --help  -h                    print help and quit\n");
	fprintf(stderr, "  --version -V                  print program version\n");
	fprintf(stderr, "  --log-level=LEVEL | -l LEVEL  set log level [error, warn, info, debug, none]\n");
	fprintf(stderr, "\n");

	exit(-1);

}

void Version(){

	printf("armadito-pdf v%s (using libarmadito-pdf v%s) \nCopyright (C) 2015 - 2017 by Teclib.\n",TOOL_VERSION,getVersion());
	exit(1);
}


int parse_options(int argc, char ** argv, struct scan_options * opts){

	while(1){

		int c, option_index = 0;

		c = getopt_long (argc, argv, "hvil:", cli_option_def, &option_index);
		
		/* Detect the end of the options. */
		if (c == -1){
			break;
		}

		switch(c){

			case 'h':
				Usage();
			break;

			case 'v':
				Version();
			break;

			case 'l':
				
				if(!strcmp("error",optarg))
					opts->log_level = LOG_LEVEL_ERROR;
				else if(!strcmp("warn",optarg))
					opts->log_level = LOG_LEVEL_WARNING;
				else if(!strcmp("info",optarg))
					opts->log_level = LOG_LEVEL_INFO;
				else if(!strcmp("debug",optarg))
					opts->log_level = LOG_LEVEL_DEBUG;
				else if(!strcmp("none",optarg))
					opts->log_level = LOG_LEVEL_NONE;
				else{
					fprintf(stderr, "Option Error: Bad log level value\n");
					Usage();
					abort();
				}
			break;

			default:
				abort();
			break;
		}	

	}

	if (optind < argc){

		opts->path_to_scan = strdup(argv[optind]);

	}else{
		fprintf(stderr, "Argument Error: Missing file or directory path\n");
		Usage();
	}

	return 0;
}


// Launch a scan directory
int do_scan(struct scan_options * opts){

	int retcode = ERROR_SUCCESS;
	int coef = 0;
	FILE * f = NULL;
	int fd = -1;
	struct pdfDocument * pdf;

	// analysis with opened file descriptor.	
	if(!(f = os_fopen(opts->path_to_scan,"rb"))){
		err_log("Can't open file %s\n", opts->path_to_scan);
		return -1;
	}

	fd = os_fileno(f);

	pdf = pdf_load_fd(fd,opts->path_to_scan,&retcode);
	if(pdf == NULL){
		err_log("File loading failed with err code (0x%x)\n",retcode);
		return -1;
	}

	retcode = pdf_parse(pdf);
	if(retcode != ERROR_SUCCESS){
		err_log("Parsing failed with err code (0x%x)\n",retcode);
		pdf_unload(pdf);
		return -1;
	}

	retcode = calc_suspicious_coef(pdf);
	if(retcode != ERROR_SUCCESS && retcode != ERROR_ENCRYPTED_CONTENT){
		err_log("Analysis failed with err code (%d)\n",retcode);
		pdf_unload(pdf);
		return -1;
	}

	pdf_print_report(pdf);

	coef = pdf->coef;

	pdf_unload(pdf);
	fclose(f);

	return coef;
}


// launch a task according to options and parameters.
int process_opts(struct scan_options * opts){

	
	if(opts == NULL || opts->path_to_scan == NULL){
		return -1;
	}

	// Set log level
	if(opts->log_level > 0)
		set_current_log_level(opts->log_level);

	return do_scan(opts);

}


int main (int argc, char ** argv){

	int ret = 0;
	struct scan_options * opts = NULL;

	if( !(opts = (struct scan_options*)calloc(1,sizeof(struct scan_options)))){
		err_log("Memory allocation failed!\n");
		return -1;
	}

	opts->log_level = -1;
	opts->path_to_scan = NULL;

	parse_options(argc,argv,opts);

	ret = process_opts(opts);

	if(opts->path_to_scan != NULL){
		free(opts->path_to_scan);
		opts->path_to_scan = NULL;
	}

	free(opts);
	opts = NULL;

	return ret;
}