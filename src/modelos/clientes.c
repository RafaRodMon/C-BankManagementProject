/*
 * clientes.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#include <stdio.h>
#include <string.h>
#include "clientes.h"
#include "../datos/sqlite3.h"

void altaCliente(sqlite3 *db) {
	Cliente c;
	char nombre[50];
	char apellido[50];
	char dni[20];
	char contrasenya[50];

	printf("Introduce el nombre: \n");
	fflush(stdout);
	scanf("%s", nombre);
	printf("Introduce el apellido: \n");
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

	    /* Crear cartera automaticamente para el nuevo cliente */
	    int id_nuevo = (int)sqlite3_last_insert_rowid(db);
	    char sqlCartera[200];
	    sprintf(sqlCartera, "INSERT INTO Cartera (id_cliente) VALUES (%d);", id_nuevo);
	    sqlite3_exec(db, sqlCartera, NULL, NULL, NULL);
	} else {
	    printf("Error al dar de alta el cliente\n");
	}
}

//Mejorado consultar clientes
void consultarCliente(sqlite3 *db) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id_cliente, nombre, apellido, dni FROM Cliente;", -1, &stmt, NULL);

    printf("\n=== LISTA DE CLIENTES ===\n");
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        printf("___________________________________________\n");
        printf("ID: %d | Nombre: %s | Apellido: %s | DNI: %s\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_text(stmt, 2),
            sqlite3_column_text(stmt, 3));
    }
    if (!found) printf("No hay clientes registrados\n");
    sqlite3_finalize(stmt);
}


void bajaCliente(sqlite3 *db){
	char dni[20];

	    printf("Introduce el DNI del cliente a dar de baja: \n");
	    fflush(stdout);
	    scanf("%s", dni);

	    /* Primero comprobamos que el cliente existe y obtenemos su id */
	    char sqlBuscar[200];
	    sprintf(sqlBuscar, "SELECT id_cliente FROM Cliente WHERE dni = '%s';", dni);

	    sqlite3_stmt *stmt;
	    sqlite3_prepare_v2(db, sqlBuscar, -1, &stmt, NULL);

	    int id_cliente = -1;
	    if (sqlite3_step(stmt) == SQLITE_ROW) {
	        id_cliente = sqlite3_column_int(stmt, 0);
	    }
	    sqlite3_finalize(stmt);

	    if (id_cliente == -1) {
	        printf("No se encontro ningun cliente con ese DNI\n");
	        return;
	    }

	    /* Borramos los movimientos de sus cuentas */
	    char sqlMovimientos[300];
	    sprintf(sqlMovimientos,
	        "DELETE FROM Movimiento WHERE id_cuenta IN "
	        "(SELECT id_cuenta FROM Cuenta WHERE id_cliente = %d);", id_cliente);
	    sqlite3_exec(db, sqlMovimientos, NULL, NULL, NULL);

	    /* Borramos sus cuentas */
	    char sqlCuentas[200];
	    sprintf(sqlCuentas, "DELETE FROM Cuenta WHERE id_cliente = %d;", id_cliente);
	    sqlite3_exec(db, sqlCuentas, NULL, NULL, NULL);

	    /* Borramos las acciones de su cartera */
	    char sqlAccionesCartera[300];
	    sprintf(sqlAccionesCartera,
	        "DELETE FROM AccionCartera WHERE id_cartera IN "
	        "(SELECT id_cartera FROM Cartera WHERE id_cliente = %d);", id_cliente);
	    sqlite3_exec(db, sqlAccionesCartera, NULL, NULL, NULL);

	    /* Borramos su cartera */
	    char sqlCartera[200];
	    sprintf(sqlCartera, "DELETE FROM Cartera WHERE id_cliente = %d;", id_cliente);
	    sqlite3_exec(db, sqlCartera, NULL, NULL, NULL);

	    /* Finalmente borramos el cliente */
	    char sqlCliente[200];
	    sprintf(sqlCliente, "DELETE FROM Cliente WHERE id_cliente = %d;", id_cliente);
	    int result = sqlite3_exec(db, sqlCliente, NULL, NULL, NULL);

	    if (result == SQLITE_OK) {
	        printf("Cliente dado de baja correctamente junto con todas sus cuentas y datos\n");
	    } else {
	        printf("Error al dar de baja el cliente\n");
	    }
}
