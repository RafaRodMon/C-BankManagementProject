/*
 * cuentas.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#include <stdio.h>
#include <string.h>
#include "cuentas.h"
#include "sqlite3.h"

void crearCuenta(sqlite3 *db, int id_cliente) {
	Cuenta c;
	char nombreCuenta[50];
	float saldo;

	printf("Introduce el nombre: \n");
	fflush(stdout);
	scanf("%s", nombreCuenta);
	printf("Introduce el saldo inicial: \n");
	fflush(stdout);
	scanf("%f", &saldo);

	strcpy(c.nombreCuenta, nombreCuenta);
	c.saldo = saldo;
	c.id_cliente = id_cliente;


	char sql[500];
	sprintf(sql, "INSERT INTO Cuenta (nombreCuenta, id_cliente, saldo) VALUES ('%s', %d, %.2f);",
			c.nombreCuenta, c.id_cliente, c.saldo);

	int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (result == SQLITE_OK) {
	    printf("Cuenta dada de alta correctamente\n");
	} else {
	    printf("Error al dar de alta la cuenta\n");
	}
}

void consultarSaldo(sqlite3 *db, int id_cliente) {
	char sql[200];
	    sprintf(sql, "SELECT nombreCuenta, saldo FROM Cuenta WHERE id_cliente = %d;", id_cliente);

	    sqlite3_stmt *stmt;
	    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

	    printf("\n=== TUS CUENTAS ===\n");
	    while (sqlite3_step(stmt) == SQLITE_ROW)
	    {
	    	printf("___________________________________________\n");
	        printf("NOMBRE DE CUENTA: %s \nSALDO EN LA CUENTA: %.2f\n",
	            sqlite3_column_text(stmt, 0),
	            sqlite3_column_double(stmt, 1));
	    }
	    sqlite3_finalize(stmt);
}
