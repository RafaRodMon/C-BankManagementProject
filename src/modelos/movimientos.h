/*
 * movimientos.h
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#ifndef SRC_MOVIMIENTOS_H_
#define SRC_MOVIMIENTOS_H_

typedef struct {
    int id_movimiento;
    int id_cuenta;
    char tipo[20];   // "INGRESO", "RETIRADA", "TRANSFERENCIA"
    float importe;
    char fecha[20];  // "DD/MM/AAAA"
} Movimiento;

#endif /* SRC_MOVIMIENTOS_H_ */
