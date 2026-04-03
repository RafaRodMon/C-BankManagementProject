/*
 * clientes.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_CLIENTES_H_
#define SRC_CLIENTES_H_

typedef struct {
    int id_cliente;
    char nombre[50];
    char apellido[50];
    char dni[10];
    char password[50];
} Cliente;

#endif /* SRC_CLIENTES_H_ */
