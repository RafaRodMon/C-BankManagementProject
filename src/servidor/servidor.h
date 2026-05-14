/*
 * servidor.h
 *
 *  Created on: 14 may 2026
 *      Author: i.tejedor
 */

#ifndef SRC_SERVIDOR_SERVIDOR_H_
#define SRC_SERVIDOR_SERVIDOR_H_

#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "../datos/sqlite3.h"
#include "../config/config.h"

#define PUERTO_DEFAULT  8888
#define TAM_BUFFER      2048

/* ── Códigos de operación del protocolo ─────────────────────────────────────
 * El cliente envía:  CMD|param1|param2|...\n
 * El servidor responde: OK|datos\n  o  ERR|mensaje\n
 * ─────────────────────────────────────────────────────────────────────────── */
#define CMD_LOGIN           "LOGIN"
#define CMD_LOGOUT          "LOGOUT"
#define CMD_ALTA_CLIENTE    "ALTA_CLI"
#define CMD_BAJA_CLIENTE    "BAJA_CLI"
#define CMD_INFO_CLIENTE    "INFO_CLI"
#define CMD_CREAR_CUENTA    "CREAR_CTA"
#define CMD_SALDO           "SALDO"
#define CMD_DEPOSITAR       "DEPOSITAR"
#define CMD_RETIRAR         "RETIRAR"
#define CMD_TRANSFERIR      "TRANSFERIR"
#define CMD_HISTORIAL       "HISTORIAL"
#define CMD_MERCADO         "MERCADO"
#define CMD_COMPRAR_ACCION  "COMPRAR_ACC"
#define CMD_VENDER_ACCION   "VENDER_ACC"
#define CMD_DESCONECTAR     "BYE"

#define RESP_OK  "OK"
#define RESP_ERR "ERR"

/* ── Estado de sesión del cliente conectado ─────────────────────────────────*/
typedef struct {
    int  autenticado;
    int  id_cliente;
    char nombre[50];
} Sesion;

/* ── API pública ─────────────────────────────────────────────────────────────*/
int  iniciarServidor(int puerto);
void bucleServidor(int sockfd_servidor, sqlite3 *db, const Config *cfg);
void atenderCliente(int sockfd_cliente, sqlite3 *db, const Config *cfg);
void cerrarServidor(int sockfd_servidor);

#endif /* SERVIDOR_H_ */

#endif /* SRC_SERVIDOR_SERVIDOR_H_ */
