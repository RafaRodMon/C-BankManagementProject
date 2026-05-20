/*
 * servidor.c
 *
 * Created on: 14 may 2026
 * Author: i.tejedor
 */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#include "../protocolo.h"
#include "../datos/ficheros.h"
#include "../datos/sqlite3.h"
#include "../servicios/acciones.h"
#include "../modelos/cuentas.h"

sqlite3 *db = NULL;

void manejar_cliente(int socket_cliente) {

}

int main() {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

    WSADATA wsaData;
    SOCKET server_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int addrlen = sizeof(direccion);

    // 1. Inicializar Winsock (OBLIGATORIO en Windows)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error al inicializar Winsock\n");
        return 1;
    }

    // 2. Crear Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        printf("Error al crear socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // 3. Configurar dirección
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(8080);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // 4. Bind
    if (bind(server_fd, (struct sockaddr *)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        printf("Error en bind: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (sqlite3_open("data/banco.db", &db) != SQLITE_OK) {
    	printf("Error al abrir la base de datos de SQLite.\n");
        return 1;
    }
    // 5. Listen
    listen(server_fd, 3);
    registrar_log("SERVIDOR: Sistema iniciado y escuchando peticiones.");

    while(1) {
        nuevo_socket = accept(server_fd, (struct sockaddr*)&direccion, &addrlen);

        if (nuevo_socket != INVALID_SOCKET) {
            registrar_log("CONEXIÓN: Un cliente se ha conectado.");

            MensajeRed msg;
            // Bucle para atender múltiples mensajes del mismo cliente
            while (recv(nuevo_socket, (char*)&msg, sizeof(msg), 0) > 0) {
                printf("Petición recibida: Tipo %d\n", msg.tipo);

                // Variables auxiliares para desempaquetar datos usando sscanf
                char usuario[50] = {0};
                char contrasenya[50] = {0};
                char cuenta[50] = {0};
                float cantidad = 0.0;

                switch(msg.tipo) {

                    case OP_LOGIN:
                        registrar_log("OPERACIÓN: Intento de Login.");
                        // Separamos el "usuario,contrasenya" enviado por el cliente
                        sscanf(msg.data, "%[^,],%s", usuario, contrasenya);

                        /* * TODO: Aquí debes conectar tu consulta SQLite/Fichero.
                         * Ejemplo: if (verificar_usuario_db(usuario, contrasenya)) ...
                         */
                        // --- PRUEBA TEMPORAL DE LÓGICA ---
                        if (strcmp(usuario, "admin") == 0 && strcmp(contrasenya, "1234") == 0) {
                            snprintf(msg.data, sizeof(msg.data), "Login OK. Bienvenido %s.", usuario);
                        } else {
                            snprintf(msg.data, sizeof(msg.data), "Error: El usuario no existe o la clave es incorrecta.");
                        }
                        break;

                    case OP_REGISTRO:
                        registrar_log("OPERACIÓN: Registro de nuevo usuario y cuenta.");

                        // 1. Extraemos los datos que envió el cliente ("usuario,contrasenya")
                        sscanf(msg.data, "%49[^,],%49s", usuario, contrasenya);

                        // 2. Aquí debes insertar el usuario en tu tabla de Usuarios de SQLite.
                        char sql_user[400];
                        snprintf(sql_user, sizeof(sql_user),
                                 "INSERT INTO Cliente (nombre, contrasenya) VALUES ('%s', '%s');",
                                 usuario, contrasenya);

                        int rc = sqlite3_exec(db, sql_user, NULL, NULL, NULL);

                        if (rc == SQLITE_OK) {
                            // 3. ¡MUY IMPORTANTE! Recuperamos el ID que SQLite le acaba de asignar al usuario
                            int id_cliente_generado = (int)sqlite3_last_insert_rowid(db);

                            // 4. Llamamos a tu función para crearle la cuenta bancaria automáticamente usando ese ID
                            // Esta función escribirá el resultado ("Usuario registrado. Cuenta asignada: SBN-XXXXXX...") en msg.data
                            crearCuentaAutomatica(db, id_cliente_generado, msg.data);

                            registrar_log("REGISTRO: Usuario y cuenta creados con éxito.");
                        } else {
                            // Si el nombre de usuario ya existía o hubo un error de BD
                            snprintf(msg.data, sizeof(msg.data), "Error: El nombre de usuario '%s' ya está en uso.", usuario);
                            registrar_log("REGISTRO: Fallo al insertar usuario.");
                        }

                        // 5. El servidor envía la estructura 'msg' de vuelta (llevará el mensaje de éxito o de error)
                        send(nuevo_socket, (char*)&msg, sizeof(msg), 0);
                        break;

                    case OP_CONSULTAR_CUENTAS:
                        registrar_log("OPERACION: Consulta de Saldo.");
                        {
                            char sql[300];
                            snprintf(sql, sizeof(sql),
                                "SELECT saldo FROM Cuenta WHERE nombreCuenta = '%s';",
                                msg.data);

                            sqlite3_stmt *stmt;
                            if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
                                if (sqlite3_step(stmt) == SQLITE_ROW) {
                                    snprintf(msg.data, sizeof(msg.data), "%.2f",
                                        sqlite3_column_double(stmt, 0));
                                } else {
                                    snprintf(msg.data, sizeof(msg.data), "Error: Cuenta no encontrada.");
                                }
                                sqlite3_finalize(stmt);
                            } else {
                                snprintf(msg.data, sizeof(msg.data), "Error interno de la BD.");
                            }
                        }
                        break;

                    case OP_TRANSFERENCIA:
                        registrar_log("OPERACIÓN: Transferencia.");
                        // TODO: Implementar validación de cuenta origen/destino y saldo suficiente
                        snprintf(msg.data, sizeof(msg.data), "Transferencia realizada con exito.");
                        break;

                    case OP_DEPOSITAR:
                        registrar_log("OPERACIÓN: Depósito.");
                        sscanf(msg.data, "%[^,],%f", cuenta, &cantidad);

                        // TODO: Modificar saldo en BD
                        if (strcmp(cuenta, "ES1234") == 0) {
                            snprintf(msg.data, sizeof(msg.data), "Deposito de %.2f EUR completado.", cantidad);
                        } else {
                            snprintf(msg.data, sizeof(msg.data), "Error: La cuenta %s no existe.", cuenta);
                        }
                        break;

                    case OP_RETIRAR:
                        registrar_log("OPERACIÓN: Retiro.");
                        sscanf(msg.data, "%[^,],%f", cuenta, &cantidad);

                        // TODO: Modificar saldo en BD controlando que no se quede en negativo
                        if (strcmp(cuenta, "ES1234") == 0) {
                            snprintf(msg.data, sizeof(msg.data), "Retiro de %.2f EUR completado.", cantidad);
                        } else {
                            snprintf(msg.data, sizeof(msg.data), "Error: La cuenta %s no existe.", cuenta);
                        }
                        break;

                    case OP_HISTORIAL:
                        registrar_log("OPERACIÓN: Historial.");
                        // Devuelve un string largo con los movimientos de la cuenta
                        snprintf(msg.data, sizeof(msg.data), "14/05/2026 - Deposito: +100 EUR\n15/05/2026 - Transferencia: -50 EUR");
                        break;

                    case OP_SALIR:
                        registrar_log("CONEXIÓN: Cliente pidió salir.");
                        closesocket(nuevo_socket);
                        goto siguiente_cliente;

                    default:
                        snprintf(msg.data, sizeof(msg.data), "Error: Operación no reconocida.");
                        break;
                }
                // Enviar la estructura modificada de vuelta al cliente
                send(nuevo_socket, (char*)&msg, sizeof(msg), 0);
            }

            registrar_log("CONEXIÓN: Cliente desconectado.");
            closesocket(nuevo_socket);
            siguiente_cliente:;
        }
    }
    return 0;
}
