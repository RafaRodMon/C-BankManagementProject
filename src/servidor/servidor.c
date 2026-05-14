/*
 * servidor.c
 *
 *  Created on: 14 may 2026
 *      Author: i.tejedor
 */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

// Esto le dice al compilador que use la librería de red de Windows
#pragma comment(lib, "ws2_32.lib")

#include "../protocolo.h"
#include "../datos/ficheros.h"
#include "../datos/sqlite3.h"
#include "../servicios/acciones.h"

void manejar_cliente(int socket_cliente) {
    MensajeRed msg;

    // Recibir petición
    if (recv(socket_cliente, &msg, sizeof(msg), 0) > 0) {
        printf("Petición recibida: Tipo %d\n", msg.tipo);

        // Aquí conectas con tus modelos (cuentas.c, clientes.c, etc)
        switch(msg.tipo) {
            case OP_LOGIN:
                // llamar a funciones de clientes.c
                break;
            case OP_CONSULTAR_CUENTAS:
                // llamar a funciones de cuentas.c
                break;
            default:
                break;
        }

        // Ejemplo de Log (Requisito Fase 2)
        registrar_log("Operación procesada con éxito");

        // Enviar respuesta (puedes reutilizar la misma struct)
        send(socket_cliente, &msg, sizeof(msg), 0);
    }
    close(socket_cliente);
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
    direccion.sin_port = htons(8080); // Asegúrate de que PORT esté definido

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
        nuevo_socket = accept(server_fd, (struct sockaddr *)&direccion, &addrlen);

        if (nuevo_socket != INVALID_SOCKET) {
            // 1. Log de conexión
            registrar_log("CONEXIÓN: Un cliente se ha conectado.");

            // 2. Procesar la petición
            MensajeRed msg;
            int bytes = recv(nuevo_socket, (char*)&msg, sizeof(msg), 0);

            if (bytes > 0) {
                // Ejemplo: Si el cliente pide login
                if (msg.tipo == OP_LOGIN) {
                    // ... lógica de validación con SQLite ...
                    registrar_log("OPERACIÓN: Intento de inicio de sesión.");
                }
                // Ejemplo: Si el cliente pide transferencia
                else if (msg.tipo == OP_TRANSFERENCIA) {
                    registrar_log("OPERACIÓN: Transferencia bancaria procesada.");
                }
            }

            registrar_log("CONEXIÓN: Cliente desconectado.");
            closesocket(nuevo_socket);
        }
    }
    return 0;
}
