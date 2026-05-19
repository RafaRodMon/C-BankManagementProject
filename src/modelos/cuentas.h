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


void crearCuentaAutomatica(sqlite3 *db, int id_cliente, char *msg_out);

void consultarSaldoRed(sqlite3 *db, int id_cliente, char *msg_out);

void depositarDineroRed(sqlite3 *db, int id_cliente, char *nombreCuenta, float deposito, char *msg_out);

void retirarDineroRed(sqlite3 *db, int id_cliente, char *nombreCuenta, float retiro, char *msg_out);

void transferirDineroRed(sqlite3 *db, int id_cliente, char *ordenante, char *beneficiario, float transferencia, char *msg_out);

void registrarMovimiento(sqlite3 *db, char *nombreCuenta, char *tipo, float importe, char *ordenante, char *beneficiaria);

void consultarHistorialRed(sqlite3 *db, int id_cliente, char *nombreCuenta, char *msg_out);

#endif /* SRC_CUENTAS_H_ */
