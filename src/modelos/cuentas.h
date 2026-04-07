/*
 * cuentas.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_CUENTAS_H_
#define SRC_CUENTAS_H_
#include "../datos/sqlite3.h"

typedef struct {
    int id_cuenta;
    char nombreCuenta[50];
    int id_cliente;
    float saldo;
} Cuenta;


void crearCuenta(sqlite3 *db, int id_cliente);

void consultarSaldo(sqlite3 *db, int id_cliente);

void depositarDinero(sqlite3 *db, int id_cliente);

void retirarDinero(sqlite3 *db, int id_cliente);

void transferirDinero(sqlite3 *db, int id_cliente);

void registrarMovimiento(sqlite3 *db, char *nombreCuenta, char *tipo, float importe, char *ordenante, char *beneficiaria);

void consultarHistorial(sqlite3 *db, int id_cliente);

#endif /* SRC_CUENTAS_H_ */
