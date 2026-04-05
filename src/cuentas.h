/*
 * cuentas.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_CUENTAS_H_
#define SRC_CUENTAS_H_
#include "sqlite3.h"

typedef struct {
    int id_cuenta;
    char nombreCuenta[50];
    int id_cliente;
    float saldo;
} Cuenta;


void crearCuenta(sqlite3 *db, int id_cliente);

void consultarSaldo(sqlite3 *db, int id_cliente);
#endif /* SRC_CUENTAS_H_ */
