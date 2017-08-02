/***

Copyright (C) 2015, 2016 Teclib'

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

#include "modulePDF.h"

struct modulePDF_data {
	const char *tmp_dir;
	int late_days;
	int critical_days;
};

static enum a6o_mod_status modulePDF_init(struct a6o_module *module) {

	// This module doesn't need initialization.
	a6o_log(A6O_LOG_MODULE, A6O_LOG_LEVEL_INFO, "module PDF initialized successfully!");
	return A6O_MOD_OK;
}

static enum a6o_mod_status modulePDF_close(struct a6o_module *module) {

	// This modules doesn't need close instruction.
	return A6O_MOD_OK;
}

static enum a6o_update_status modulePDF_info(struct a6o_module *module, struct a6o_module_info *info){

	time_t ts = 0;
	struct tm timeptr = {0, 30, 8, 8, 5, 116}; // 01/03/2016 9:30

	ts=mktime(&timeptr);
	info->mod_update_ts = ts;

	return A6O_UPDATE_OK;
}


static enum a6o_file_status modulePDF_scan(struct a6o_module *module, int fd, const char *path, const char *mime_type, char **pmod_report) {

	enum a6o_file_status status = A6O_FILE_CLEAN;
	int retcode = EXIT_SUCCESS;
	struct pdfDocument * pdf;


	pdf = pdf_load_fd(fd,path,&retcode);
	if(pdf == NULL){
		err_log("File loading failed with err code (0x%x)\n",retcode);
		return A6O_FILE_IERROR;;
	}

	retcode = pdf_parse(pdf);
	if(retcode != ERROR_SUCCESS){
		err_log("Parsing failed with err code (0x%x)\n",retcode);
		pdf_unload(pdf);
		return A6O_FILE_IERROR;
	}

	retcode = calc_suspicious_coef(pdf);
	if(retcode != ERROR_SUCCESS && retcode != ERROR_ENCRYPTED_CONTENT){
		err_log("Analysis failed with err code (%d)\n",retcode);
		pdf_unload(pdf);
		return A6O_FILE_IERROR;
	}

	if (pdf->coef == -2) {
		status = A6O_FILE_UNDECIDED; // Not supported files (encrypted contents or bad header).
	}
	else if (pdf->coef < MALICIOUS_COEF) {
		status = A6O_FILE_CLEAN;
	}
	else if (pdf->coef >= MALICIOUS_COEF) {
		status = A6O_FILE_SUSPICIOUS;
		*pmod_report = os_strdup("ModulePDF!SuspiciousPDF");
	}

	pdf_unload(pdf);

	return status;
}


static const char *modulePDF_mime_types[] = {
	"application/pdf",
	NULL,
};

struct a6o_module module = {
	.init_fun = modulePDF_init,
	.conf_table = NULL,
	.post_init_fun = NULL,
	.scan_fun = modulePDF_scan,
	.close_fun = modulePDF_close,
	.info_fun = modulePDF_info,
	.supported_mime_types = modulePDF_mime_types,
	.name = "modulePDF",
	.size = sizeof(struct modulePDF_data),
};
