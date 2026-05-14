/*
 * acciones.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_ACCIONES_H_
#define SRC_ACCIONES_H_

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include "../datos/sqlite3.h"

typedef struct {
    int id_accion;
    char nombre[50];
    float precio_actual;
} Accion;

void mostrarMercado(sqlite3 *db);
void comprarAccion(sqlite3 *db, int id_cliente);
void venderAccion(sqlite3 *db, int id_cliente);
void procesar_peticion(SOCKET socket_cliente);

#endif /* SRC_ACCIONES_H_ */
