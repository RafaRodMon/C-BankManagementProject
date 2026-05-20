/*
 * config.h
 *
 *  Created on: 2 abr 2026
 *      Author: ander.belloso
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

typedef struct {
    char admin_usuario[50];
    char admin_password[50];
    char db_ruta[100];
    char log_ruta[100];
    int puerto;
} Config;

int cargarConfig(const char *ruta, Config *cfg);

#endif /* SRC_CONFIG_H_ */
