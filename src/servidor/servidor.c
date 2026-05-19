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
                        registrar_log("OPERACIÓN: Registro de usuario.");
                        sscanf(msg.data, "%[^,],%s", usuario, contrasenya);

                        /* * TODO: Aquí insertarías el usuario en la base de datos.
                         * if (guardar_usuario_db(usuario, contrasenya)) ...
                         */
                        snprintf(msg.data, sizeof(msg.data), "Usuario %s registrado correctamente.", usuario);
                        break;

                    case OP_CONSULTAR_CUENTAS:
                        registrar_log("OPERACIÓN: Consulta de Saldo.");
                        // El cliente envía el ID de la cuenta directamente en msg.data
                        strncpy(cuenta, msg.data, sizeof(cuenta) - 1);

                        /*
                         * TODO: Consultar saldo real en SQLite.
                         * float saldo = obtener_saldo_db(cuenta);
                         */
                        // --- PRUEBA TEMPORAL DE LÓGICA ---
                        if (strcmp(cuenta, "ES1234") == 0) {
                            // Si existe, devolvemos SOLO el número en texto para que atof() lo lea en el cliente
                            snprintf(msg.data, sizeof(msg.data), "2550.75");
                        } else {
                            // Si no existe, mandamos la palabra "Error" que el cliente interceptará
                            snprintf(msg.data, sizeof(msg.data), "Error: La cuenta bancaria %s no existe.", cuenta);
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
