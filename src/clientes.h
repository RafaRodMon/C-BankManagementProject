/*
 * clientes.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_CLIENTES_H_
#define SRC_CLIENTES_H_

#include "sqlite3.h"

typedef struct {
    int id_cliente;
    char nombre[50];
    char apellido[50];
    char dni[20];
    char contrasenya[50];
} Cliente;

void altaCliente(sqlite3 *db);
void consultarCliente(sqlite3 *db);
void bajaCliente(sqlite3 *db);


#endif /* SRC_CLIENTES_H_ */
