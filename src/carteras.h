/*
 * carteras.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_CARTERAS_H_
#define SRC_CARTERAS_H_
#include "sqlite3.h"

typedef struct {
    int id_cartera;
    int id_cliente;
} Cartera;

typedef struct {
    int id_cartera;
    int id_accion;
    int cantidad;
    float precio_compra;  // media ponderada
} AccionCartera;

void mostrarCartera(sqlite3 *db, int id_cliente);

#endif /* SRC_CARTERAS_H_ */
