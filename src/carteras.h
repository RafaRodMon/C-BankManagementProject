/*
 * carteras.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_CARTERAS_H_
#define SRC_CARTERAS_H_

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

#endif /* SRC_CARTERAS_H_ */
