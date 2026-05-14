/*
 * protocolo.h
 *
 *  Created on: 14 may 2026
 *      Author: i.tejedor
 */

#ifndef SRC_PROTOCOLO_H_
#define SRC_PROTOCOLO_H_


#define PORT 8080
#define BUFFER_SIZE 1024

// Tipos de operaciones
typedef enum {
    OP_LOGIN,
    OP_CONSULTAR_CUENTAS,
    OP_TRANSFERENCIA,
    OP_HISTORIAL,
    OP_SALIR
} TipoOperacion;

// Estructura de mensaje
typedef struct {
    TipoOperacion tipo;
    int id_usuario;
    char data[256]; // Para enviar contraseñas, montos, etc.
} MensajeRed;


#endif /* SRC_PROTOCOLO_H_ */
