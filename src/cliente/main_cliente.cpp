/*
 * main_cliente.cpp
 *
 *  Created on: 14 may 2026
 *      Author: i.tejedor
 */

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>

#include "../protocolo.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

int main() {
    // 1. Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        return 1;
    }

    // 2. Crear el socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Error al crear el socket: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    // 3. Configurar la dirección del servidor
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080); // El puerto que definas
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP local

    // 4. Conectar
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        cerr << "No se pudo conectar con el servidor bancario" << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    cout << "Conectado al servidor con exito." << endl;

    // --- AQUÍ IRÍA TU MENÚ DE LA FASE 1 ---
    int opcion = -1;
    float saldo_cache = -1.0; // Implementación de la CACHÉ (Requisito Fase 2)

    while (opcion != 0) {
        cout << "\n1. Consultar Saldo\n0. Salir\nSeleccion: ";
        cin >> opcion;

        if (opcion == 1) {
            if (saldo_cache != -1.0) {
                cout << "[Memoria Local] Saldo: " << saldo_cache << endl;
            } else {
                MensajeRed msg;
                msg.tipo = OP_CONSULTAR_CUENTAS;

                // Enviar petición
                send(sock, (char*)&msg, sizeof(msg), 0);

                // Recibir respuesta
                recv(sock, (char*)&msg, sizeof(msg), 0);

                // Supongamos que guardamos el saldo en msg.data
                saldo_cache = atof(msg.data);
                cout << "[Servidor] Saldo actualizado: " << saldo_cache << endl;
            }
        }
    }

    // 5. Limpieza
    closesocket(sock);
    WSACleanup();
    return 0;
}
