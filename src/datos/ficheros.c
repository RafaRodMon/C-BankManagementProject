/*
 * ficheros.c
 *
 *  Created on: 2 abr 2026
 *      Author: ander.belloso
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include "sqlite3.h"
#include "ficheros.h"

int abrirDB(const char *ruta, sqlite3 **db) {
	int result = sqlite3_open(ruta,db);

	if (result != SQLITE_OK) {
	        printf("Error opening database\n");
	        return result;
	    }
	    printf("Database opened\n");

	return 0;
}

int crearTablas(sqlite3 *db) {

	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Cliente ("
		    "id_cliente INTEGER PRIMARY KEY AUTOINCREMENT,"
		    "nombre TEXT,"
		    "apellido TEXT,"
		    "dni TEXT,"
		    "contrasenya TEXT);", NULL, NULL, NULL);

	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Cuenta ("
			"id_cuenta INTEGER PRIMARY KEY AUTOINCREMENT,"
			"nombreCuenta TEXT,"
			"id_cliente INTEGER,"
			"saldo INTEGER);", NULL, NULL, NULL);


	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Movimiento ("
			"id_movimiento INTEGER PRIMARY KEY AUTOINCREMENT,"
			"id_cuenta INTEGER,"
			"tipo TEXT,"
			"importe REAL,"
			"fecha TEXT,"
			"cuenta_ordenante TEXT,"
			"cuenta_beneficiaria TEXT);", NULL, NULL, NULL);

	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Accion ("
			"id_accion INTEGER PRIMARY KEY AUTOINCREMENT,"
			"precio_actual REAL);", NULL, NULL, NULL);

	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Cartera ("
	        "id_cartera INTEGER PRIMARY KEY AUTOINCREMENT,"
	        "id_cliente INTEGER);", NULL, NULL, NULL);

	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS AccionCartera ("
	        "id_cartera INTEGER,"
	        "id_accion INTEGER,"
	        "cantidad INTEGER,"
	        "precio_compra REAL,"
	        "PRIMARY KEY (id_cartera, id_accion));", NULL, NULL, NULL);

	return 0;
}


void registrar_log(const char *ruta_log, const char *mensaje) {
    FILE* f = fopen(ruta_log, "a");
    if (f) {
        time_t ahora = time(NULL);
        char* fecha = ctime(&ahora);
        fecha[strlen(fecha)-1] = '\0';
        fprintf(f, "[%s] %s\n", fecha, mensaje);
        fclose(f);
    }
}
