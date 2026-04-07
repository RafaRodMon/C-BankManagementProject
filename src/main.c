/*
 * main.c
 *
 *  Created on: 24 mar 2026
 *      Author: parsero
 */
#include <stdio.h>
#include "config/config.h"
#include "datos/sqlite3.h"
#include "datos/ficheros.h"
#include "ui/menu.h"

int main(void) {
    Config cfg;
    if (cargarConfig("../data/config.cfg", &cfg) == -1) {
        printf("Error: no se puede abrir el fichero de configuracion\n");
        return 1;
    }

    sqlite3 *db;
    if (abrirDB(cfg.db_ruta, &db) == -1) {
        printf("Error: no se puede abrir la base de datos\n");
        return 1;
    }
    if (crearTablas(db) == -1) {
        printf("Error: no se pueden crear las tablas\n");
        return 1;
    }

    printf("Base de datos lista\n");
    menuLogin(&cfg,db);

    sqlite3_close(db);
    return 0;
}
