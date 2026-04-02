/*
 * ficheros.h
 *
 *  Created on: 2 abr 2026
 *      Author: ander.belloso
 */

#ifndef SRC_FICHEROS_H_
#define SRC_FICHEROS_H_

#include "sqlite3.h"
#include "config.h"

int abrirDB(const char *ruta, sqlite3 **db);
int crearTablas(sqlite3 *db);

#endif /* SRC_FICHEROS_H_ */
