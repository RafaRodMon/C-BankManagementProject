/*
 * config.c
 *
 *  Created on: 2 abr 2026
 *      Author: ander.belloso
 */

#include <stdio.h>
#include <string.h>
#include "config.h"

int cargarConfig(const char *ruta, Config *cfg) {

	FILE * fichero = fopen(ruta, "r");

	if (fichero == NULL) {
		return -1;
	}

	char linea[200];
	char clave[100];
	char valor[100];

	while (fgets(linea, 200, fichero) != NULL) {
		sscanf(linea, "%[^=]=%s", clave, valor);

		if (strcmp(clave, "admin_usuario") == 0) {
		    strcpy(cfg->admin_usuario, valor);
		}

		if (strcmp(clave, "admin_password") == 0) {
		    strcpy(cfg->admin_password, valor);
		}

		if (strcmp(clave, "db_ruta") == 0) {
		    strcpy(cfg->db_ruta, valor);
		}

		if (strcmp(clave, "log_ruta") == 0) {
		    strcpy(cfg->log_ruta, valor);
		}
	}

	fclose(fichero);
	return 0;
}
