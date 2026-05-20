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
    Config cfg;

    if (cargarConfig("data/config.txt", &cfg) != 0) {
        printf("Error al cargar config.txt. Usando valores por defecto.\n");
        strcpy(cfg.db_ruta, "data/banco.db");
        strcpy(cfg.log_ruta, "data/server.log");
        cfg.puerto = 8080;
    }

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
    direccion.sin_port = htons(cfg.puerto);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // 4. Bind
    if (bind(server_fd, (struct sockaddr *)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        printf("Error en bind: %d\n", WSAGetLastError());
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (sqlite3_open(cfg.db_ruta, &db) != SQLITE_OK) {
    	printf("Error al abrir la base de datos de SQLite.\n");
        return 1;
    }
    // 5. Listen
    listen(server_fd, 3);
    registrar_log(cfg.log_ruta,"SERVIDOR: Sistema iniciado y escuchando peticiones.");

    while(1) {
        nuevo_socket = accept(server_fd, (struct sockaddr*)&direccion, &addrlen);

        if (nuevo_socket != INVALID_SOCKET) {
            registrar_log(cfg.log_ruta,"CONEXIÓN: Un cliente se ha conectado.");

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
                    registrar_log(cfg.log_ruta,"OPERACIÓN: Intento de Login.");
                    {
                        sscanf(msg.data, "%49[^,],%49s", usuario, contrasenya);

                        char sql_login[300];
                        snprintf(sql_login, sizeof(sql_login),
                            "SELECT id_cliente, nombre FROM Cliente "
                            "WHERE nombre = '%s' AND contrasenya = '%s';",
                            usuario, contrasenya);

                        sqlite3_stmt *stmt;
                        int encontrado = 0;
                        char nombre_cliente[50] = {0};

                        if (sqlite3_prepare_v2(db, sql_login, -1, &stmt, NULL) == SQLITE_OK) {
                            if (sqlite3_step(stmt) == SQLITE_ROW) {
                                encontrado = 1;
                                strncpy(nombre_cliente,
                                    (const char*)sqlite3_column_text(stmt, 1),
                                    sizeof(nombre_cliente) - 1);
                            }
                            sqlite3_finalize(stmt);
                        }

                        if (encontrado) {
                            snprintf(msg.data, sizeof(msg.data), "Login OK. Bienvenido %s.", nombre_cliente);
                            registrar_log(cfg.log_ruta,"LOGIN: Acceso correcto.");
                        } else {
                            snprintf(msg.data, sizeof(msg.data), "Error: El usuario no existe o la clave es incorrecta.");
                            registrar_log(cfg.log_ruta,"LOGIN: Acceso denegado.");
                        }
                    }
                    break;

                    case OP_REGISTRO:
                        registrar_log(cfg.log_ruta,"OPERACIÓN: Registro de nuevo usuario y cuenta.");

                        // 1. Extraemos los datos enviados por el cliente ("nombre,apellido,dni,contrasenya")
                        char apellido[50] = {0};
                        char dni[20] = {0};
                        sscanf(msg.data, "%49[^,],%49[^,],%19[^,],%49s", usuario, apellido, dni, contrasenya);

                        // 2. Insertar el cliente completo en la BD
                        char sql_user[400];
                        snprintf(sql_user, sizeof(sql_user),
                                 "INSERT INTO Cliente (nombre, apellido, dni, contrasenya) VALUES ('%s', '%s', '%s', '%s');",
                                 usuario, apellido, dni, contrasenya);

                        int rc = sqlite3_exec(db, sql_user, NULL, NULL, NULL);

                        if (rc == SQLITE_OK) {
                            // 3. Recuperamos el ID generado automáticamente
                            int id_cliente_generado = (int)sqlite3_last_insert_rowid(db);

                            // 4. Creamos la cuenta bancaria automáticamente para ese cliente
                            crearCuentaAutomatica(db, id_cliente_generado, msg.data);

                            registrar_log(cfg.log_ruta,"REGISTRO: Usuario y cuenta creados con éxito.");
                        } else {
                            snprintf(msg.data, sizeof(msg.data), "Error: El nombre de usuario '%s' ya está en uso.", usuario);
                            registrar_log(cfg.log_ruta,"REGISTRO: Fallo al insertar usuario.");
                        }

                        // 5. Enviar respuesta al cliente
                        send(nuevo_socket, (char*)&msg, sizeof(msg), 0);
                        break;

                    case OP_CONSULTAR_CUENTAS:
                        registrar_log(cfg.log_ruta,"OPERACION: Consulta de Saldo.");
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
                        registrar_log(cfg.log_ruta,"OPERACIÓN: Transferencia.");
                        {
                            char cuenta_origen[50] = {0};
                            char cuenta_destino[50] = {0};
                            float cantidad_transferir = 0.0;
                            sscanf(msg.data, "%49[^,],%49[^,],%f", cuenta_origen, cuenta_destino, &cantidad_transferir);

                            if (cantidad_transferir <= 0) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Cantidad invalida.");
                                break;
                            }

                            /* Verificar saldo suficiente en cuenta origen */
                            char sql_check[300];
                            snprintf(sql_check, sizeof(sql_check),
                                "SELECT saldo FROM Cuenta WHERE nombreCuenta = '%s';", cuenta_origen);

                            sqlite3_stmt *stmt;
                            float saldo_origen = -1.0;
                            if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) == SQLITE_OK) {
                                if (sqlite3_step(stmt) == SQLITE_ROW)
                                    saldo_origen = (float)sqlite3_column_double(stmt, 0);
                                sqlite3_finalize(stmt);
                            }

                            if (saldo_origen < 0) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Cuenta origen no encontrada.");
                                break;
                            }
                            if (saldo_origen < cantidad_transferir) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Saldo insuficiente. Saldo actual: %.2f EUR.", saldo_origen);
                                break;
                            }

                            /* Verificar que cuenta destino existe */
                            char sql_check2[300];
                            snprintf(sql_check2, sizeof(sql_check2),
                                "SELECT id_cuenta FROM Cuenta WHERE nombreCuenta = '%s';", cuenta_destino);
                            int destino_existe = 0;
                            if (sqlite3_prepare_v2(db, sql_check2, -1, &stmt, NULL) == SQLITE_OK) {
                                if (sqlite3_step(stmt) == SQLITE_ROW) destino_existe = 1;
                                sqlite3_finalize(stmt);
                            }

                            if (!destino_existe) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Cuenta destino no encontrada.");
                                break;
                            }

                            /* Transferencia atomica */
                            sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL);

                            char sql_resta[300], sql_suma[300];
                            snprintf(sql_resta, sizeof(sql_resta),
                                "UPDATE Cuenta SET saldo = saldo - %.2f WHERE nombreCuenta = '%s';",
                                cantidad_transferir, cuenta_origen);
                            snprintf(sql_suma, sizeof(sql_suma),
                                "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s';",
                                cantidad_transferir, cuenta_destino);

                            int ok = sqlite3_exec(db, sql_resta, NULL, NULL, NULL) == SQLITE_OK &&
                                     sqlite3_exec(db, sql_suma,  NULL, NULL, NULL) == SQLITE_OK;

                            if (ok) {
                                sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
                                registrarMovimiento(db, cuenta_origen,  "TRANSFERENCIA", cantidad_transferir, cuenta_origen, cuenta_destino);
                                registrarMovimiento(db, cuenta_destino, "TRANSFERENCIA", cantidad_transferir, cuenta_origen, cuenta_destino);
                                snprintf(msg.data, sizeof(msg.data), "Transferencia de %.2f EUR de %s a %s realizada con exito.",
                                         cantidad_transferir, cuenta_origen, cuenta_destino);
                                registrar_log(cfg.log_ruta,"TRANSFERENCIA realizada con exito.");
                            } else {
                                sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
                                snprintf(msg.data, sizeof(msg.data), "Error: Fallo al realizar la transferencia.");
                            }
                        }
                        break;

                    case OP_DEPOSITAR:
                        registrar_log(cfg.log_ruta,"OPERACIÓN: Depósito.");
                        {
                            sscanf(msg.data, "%49[^,],%f", cuenta, &cantidad);

                            if (cantidad <= 0) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Cantidad invalida.");
                                break;
                            }

                            /* Verificar que la cuenta existe */
                            char sql_check[300];
                            snprintf(sql_check, sizeof(sql_check),
                                "SELECT id_cuenta FROM Cuenta WHERE nombreCuenta = '%s';", cuenta);

                            sqlite3_stmt *stmt;
                            int existe = 0;
                            if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) == SQLITE_OK) {
                                if (sqlite3_step(stmt) == SQLITE_ROW) existe = 1;
                                sqlite3_finalize(stmt);
                            }

                            if (!existe) {
                                snprintf(msg.data, sizeof(msg.data), "Error: La cuenta %s no existe.", cuenta);
                                break;
                            }

                            /* Actualizar saldo */
                            char sql_update[300];
                            snprintf(sql_update, sizeof(sql_update),
                                "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s';",
                                cantidad, cuenta);

                            if (sqlite3_exec(db, sql_update, NULL, NULL, NULL) == SQLITE_OK) {
                                registrarMovimiento(db, cuenta, "DEPOSITO", cantidad, "EXTERNO", cuenta);
                                snprintf(msg.data, sizeof(msg.data), "Deposito de %.2f EUR en %s completado.", cantidad, cuenta);
                                registrar_log(cfg.log_ruta,"DEPOSITO realizado con exito.");
                            } else {
                                snprintf(msg.data, sizeof(msg.data), "Error: Fallo al realizar el deposito.");
                            }
                        }
                        break;

                    case OP_RETIRAR:
                        registrar_log(cfg.log_ruta,"OPERACIÓN: Retiro.");
                        {
                            sscanf(msg.data, "%49[^,],%f", cuenta, &cantidad);

                            if (cantidad <= 0) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Cantidad invalida.");
                                break;
                            }

                            /* Verificar que la cuenta existe y tiene saldo suficiente */
                            char sql_check[300];
                            snprintf(sql_check, sizeof(sql_check),
                                "SELECT saldo FROM Cuenta WHERE nombreCuenta = '%s';", cuenta);

                            sqlite3_stmt *stmt;
                            float saldo_actual = -1.0;
                            if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) == SQLITE_OK) {
                                if (sqlite3_step(stmt) == SQLITE_ROW)
                                    saldo_actual = (float)sqlite3_column_double(stmt, 0);
                                sqlite3_finalize(stmt);
                            }

                            if (saldo_actual < 0) {
                                snprintf(msg.data, sizeof(msg.data), "Error: La cuenta %s no existe.", cuenta);
                                break;
                            }
                            if (saldo_actual < cantidad) {
                                snprintf(msg.data, sizeof(msg.data), "Error: Saldo insuficiente. Saldo actual: %.2f EUR.", saldo_actual);
                                break;
                            }

                            /* Actualizar saldo */
                            char sql_update[300];
                            snprintf(sql_update, sizeof(sql_update),
                                "UPDATE Cuenta SET saldo = saldo - %.2f WHERE nombreCuenta = '%s';",
                                cantidad, cuenta);

                            if (sqlite3_exec(db, sql_update, NULL, NULL, NULL) == SQLITE_OK) {
                                registrarMovimiento(db, cuenta, "RETIRADA", cantidad, cuenta, "EXTERNO");
                                snprintf(msg.data, sizeof(msg.data), "Retiro de %.2f EUR de %s completado.", cantidad, cuenta);
                                registrar_log(cfg.log_ruta,"RETIRO realizado con exito.");
                            } else {
                                snprintf(msg.data, sizeof(msg.data), "Error: Fallo al realizar el retiro.");
                            }
                        }
                        break;

                    case OP_HISTORIAL:
                        registrar_log(cfg.log_ruta,"OPERACIÓN: Historial.");
                        {
                            char sql[300];
                            snprintf(sql, sizeof(sql),
                                "SELECT tipo, importe, ordenante, beneficiaria, fecha "
                                "FROM Movimiento WHERE nombreCuenta = '%s' "
                                "ORDER BY fecha DESC LIMIT 20;",
                                msg.data);

                            sqlite3_stmt *stmt;
                            char historial[256] = "";
                            int primero = 1;

                            if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
                                while (sqlite3_step(stmt) == SQLITE_ROW) {
                                    char linea[100];
                                    snprintf(linea, sizeof(linea), "%s%s | %.2f EUR | %s -> %s | %s",
                                        primero ? "" : "\n",
                                        (const char*)sqlite3_column_text(stmt, 0),
                                        sqlite3_column_double(stmt, 1),
                                        (const char*)sqlite3_column_text(stmt, 2),
                                        (const char*)sqlite3_column_text(stmt, 3),
                                        (const char*)sqlite3_column_text(stmt, 4));
                                    strncat(historial, linea, sizeof(historial) - strlen(historial) - 1);
                                    primero = 0;
                                }
                                sqlite3_finalize(stmt);
                            }

                            if (primero)
                                snprintf(msg.data, sizeof(msg.data), "No hay movimientos para esta cuenta.");
                            else
                                strncpy(msg.data, historial, sizeof(msg.data) - 1);
                        }
                        break;

                    case OP_SALIR:
                        registrar_log(cfg.log_ruta,"CONEXIÓN: Cliente pidió salir.");
                        closesocket(nuevo_socket);
                        goto siguiente_cliente;

                    default:
                        snprintf(msg.data, sizeof(msg.data), "Error: Operación no reconocida.");
                        break;
                }
                // Enviar la estructura modificada de vuelta al cliente
                send(nuevo_socket, (char*)&msg, sizeof(msg), 0);
            }

            registrar_log(cfg.log_ruta,"CONEXIÓN: Cliente desconectado.");
            closesocket(nuevo_socket);
            siguiente_cliente:;
        }
    }
    return 0;
}
