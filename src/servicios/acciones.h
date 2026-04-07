/*
 * acciones.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_ACCIONES_H_
#define SRC_ACCIONES_H_
#include "sqlite3.h"

typedef struct {
    int id_accion;
    char nombre[50];
    float precio_actual;
} Accion;

void mostrarMercado(sqlite3 *db);
void comprarAccion(sqlite3 *db, int id_cliente);
void venderAccion(sqlite3 *db, int id_cliente);

#endif /* SRC_ACCIONES_H_ */
