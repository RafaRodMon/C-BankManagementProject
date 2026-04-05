/*
 * clientes.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#include <stdio.h>
#include <string.h>
#include "clientes.h"
#include "sqlite3.h"

void altaCliente(sqlite3 *db) {
	Cliente c;
	char nombre[50];
	char apellido[50];
	char dni[20];
	char contrasenya[50];

	printf("Introduce el nombre: \n");
	fflush(stdout);
	scanf("%s", nombre);
	printf("Introduce el aoellido: \n");
	fflush(stdout);
	scanf("%s", apellido);
	printf("Introduce el dni: \n");
	fflush(stdout);
	scanf("%s", dni);
	printf("Introduce la contrasenya: \n");
	fflush(stdout);
	scanf("%s", contrasenya);

	strcpy(c.nombre, nombre);
	strcpy(c.apellido, apellido);
	strcpy(c.dni, dni);
	strcpy(c.contrasenya, contrasenya);

	char sql[500];
	sprintf(sql, "INSERT INTO Cliente (nombre, apellido, dni, contrasenya) VALUES ('%s', '%s', '%s', '%s');",
	    c.nombre, c.apellido, c.dni, c.contrasenya);

	int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (result == SQLITE_OK) {
	    printf("Cliente dado de alta correctamente\n");
	} else {
	    printf("Error al dar de alta el cliente\n");
	}
}

void consultarCliente(sqlite3 *db){
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT * FROM Cliente;", -1, &stmt, NULL);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
	    printf("ID: %d | Nombre: %s | Apellido: %s | DNI: %s\n",
	        sqlite3_column_int(stmt, 0),
	        sqlite3_column_text(stmt, 1),
	        sqlite3_column_text(stmt, 2),
	        sqlite3_column_text(stmt, 3));
	}
	sqlite3_finalize(stmt);
}

void bajaCliente(sqlite3 *db){


}
