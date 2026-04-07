/*
 * cuentas.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#include <stdio.h>
#include <string.h>
#include "cuentas.h"
#include "../datos/sqlite3.h"

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

void depositarDinero(sqlite3 *db, int id_cliente) {
	char nombreCuenta[50];
	float deposito;

	printf("Nombre de cuenta: \n");
	fflush(stdout);
	scanf("%s", nombreCuenta);
	printf("Cantidad de dinero a depositar: \n");
	fflush(stdout);
	scanf("%f", &deposito);

	char sql[300];
	sprintf(sql, "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s' AND id_cliente = %d;",
			deposito, nombreCuenta, id_cliente);

	int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
	    printf("Deposito realizado correctamente\n");
	} else {
	    printf("Cuenta no encontrada, volviendo al menu\n");
	}

	registrarMovimiento(db, nombreCuenta, "DEPOSITO", deposito, "", "");
}

void retirarDinero(sqlite3 *db, int id_cliente) {
	char nombreCuenta[50];
	float retiro;

	printf("Nombre de cuenta: \n");
	fflush(stdout);
	scanf("%s", nombreCuenta);
	printf("Cantidad de dinero a retirar: \n");
	fflush(stdout);
	scanf("%f", &retiro);

	char sql[300];
	sprintf(sql, "UPDATE Cuenta SET saldo = saldo - %.2f WHERE nombreCuenta = '%s' AND id_cliente = %d AND saldo >= %.2f;",
			retiro, nombreCuenta, id_cliente, retiro);

	int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
	    printf("Dinero retirado correctamente\n");
	} else {
	    printf("Saldo insuficiente o cuenta no encontrada\n");
	}

	registrarMovimiento(db, nombreCuenta, "RETIRO", retiro, "", "");
}

void transferirDinero(sqlite3 *db, int id_cliente) {
	char ordenante[50];
	char beneficiario[59];
	float transferencia;

	printf("Nombre de cuenta ordenante: \n");
	fflush(stdout);
	scanf("%s", ordenante);
	printf("Nombre de cuenta beneficiario: \n");
	fflush(stdout);
	scanf("%s", beneficiario);
	printf("Cantidad de dinero a transferir: \n");
	fflush(stdout);
	scanf("%f", &transferencia);

	char sql[300];
	sprintf(sql, "UPDATE Cuenta SET saldo = saldo - %.2f WHERE nombreCuenta = '%s' AND id_cliente = %d AND saldo >= %.2f;",
			transferencia, ordenante, id_cliente, transferencia);

	char sql2[300];
	sprintf(sql2, "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s';",
			transferencia, beneficiario);

	int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
	if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
		printf("Saldo retirado correctamente\n");
		result = sqlite3_exec(db, sql2, NULL, NULL, NULL);
		if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
			printf("Transferencia realizada correctamente\n");
		} else {
			printf("Cuenta beneficiaria no encontrada\n");
		}
	} else {
		printf("Saldo insuficiente o cuenta ordenante no encontrada\n");
	}

	registrarMovimiento(db, ordenante, "TRANSFERENCIA", transferencia, ordenante, beneficiario);
}

void registrarMovimiento(sqlite3 *db, char *nombreCuenta, char *tipo,
		float importe, char *ordenante, char *beneficiaria) {
    char sqlId[200];
    sprintf(sqlId, "SELECT id_cuenta FROM Cuenta WHERE nombreCuenta = '%s';", nombreCuenta);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sqlId, -1, &stmt, NULL);

    int id_cuenta = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_cuenta = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (id_cuenta == -1) return;

    char sql[500];
    sprintf(sql, "INSERT INTO Movimiento (id_cuenta, tipo, importe, fecha, cuenta_ordenante, cuenta_beneficiaria) "
        "VALUES (%d, '%s', %.2f, datetime('now'), '%s', '%s');",
        id_cuenta, tipo, importe, ordenante, beneficiaria);
    sqlite3_exec(db, sql, NULL, NULL, NULL);
}

void consultarHistorial(sqlite3 *db, int id_cliente) {
    char nombreCuenta[50];
    printf("Nombre de la cuenta: \n");
    fflush(stdout);
    scanf("%s", nombreCuenta);

    char sql[500];
    sprintf(sql, "SELECT M.tipo, M.importe, M.fecha, M.cuenta_ordenante, M.cuenta_beneficiaria "
        "FROM Movimiento M "
        "JOIN Cuenta C ON M.id_cuenta = C.id_cuenta "
        "WHERE C.nombreCuenta = '%s' AND C.id_cliente = %d;",
        nombreCuenta, id_cliente);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("\n=== HISTORIAL ===\n");
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
    	printf("___________________________________________\n");
        found = 1;
        char *tipo = (char*)sqlite3_column_text(stmt, 0);
        float importe = sqlite3_column_double(stmt, 1);
        char *fecha = (char*)sqlite3_column_text(stmt, 2);
        char *ordenante = (char*)sqlite3_column_text(stmt, 3);
        char *beneficiaria = (char*)sqlite3_column_text(stmt, 4);

        printf("FECHA: %s \nTIPO: %s \nIMPORTE: %.2f \n", fecha, tipo, importe);

        if (strcmp(tipo, "TRANSFERENCIA") == 0) {
            printf("CUENTA ORDENANTE: %s \nCUENTA BENEFICIARIA: %s", ordenante, beneficiaria);
        }
        printf("\n");
    }
    if (!found) printf("No hay movimientos para esta cuenta\n");
    sqlite3_finalize(stmt);
}
