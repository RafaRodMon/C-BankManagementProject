/*
 * main.c
 *
 *  Created on: 24 mar 2026
 *      Author: parsero
 */


#include "config.h"
#include <stdio.h>
#include "sqlite3.h"
#include "ficheros.h"
#include "menu.h"

int main(void) {
    Config cfg;
    if (cargarConfig("data/config.cfg", &cfg) == -1) {
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


    int intentos = 0;
    while (login(&cfg) == -1 && intentos < 3) {
        printf("Usuario o contrasenya incorrectos, intentelo de nuevo\n");
        intentos++;
    }
    if (intentos == 3) {
        printf("Demasiados intentos fallidos, cerrando programa\n");
        sqlite3_close(db);
        return 1;
    }
    printf("Login correcto\n");
    menuPrincipal(db);

    sqlite3_close(db);
    return 0;
}
