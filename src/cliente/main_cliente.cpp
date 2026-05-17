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

    // --- MENÚ REAL DE TU FASE 1 ---
    int opcion = -1;
    float saldo_cache = -1.0; // Tu implementación de caché para OP_CONSULTAR_CUENTAS

    while (opcion != 0) {
        cout << "\n===================================";
        cout << "\n     SISTEMA BANCARIO      ";
        cout << "\n===================================";
        cout << "\n1. Iniciar Sesion (Login)";
        cout << "\n2. Consultar Cuentas / Saldo";
        cout << "\n3. Realizar Transferencia";
        cout << "\n4. Ver Historial de Transacciones";
        cout << "\n0. Salir";
        cout << "\n===================================";
        cout << "\nSeleccion: ";
        cin >> opcion;

        if (opcion == 1) {
            MensajeRed msg;
            msg.tipo = OP_LOGIN;

            cout << "Introduce Usuario y Contrasena (ej: usuario,clave): ";
            cin.ignore();
            cin.getline(msg.data, sizeof(msg.data));

            send(sock, (char*)&msg, sizeof(msg), 0);
            recv(sock, (char*)&msg, sizeof(msg), 0);
            cout << "[Servidor]: " << msg.data << endl;

        } else if (opcion == 2) {
            // Aquí aplicas tu lógica de CACHÉ para no saturar al servidor
            if (saldo_cache != -1.0) {
                cout << "[Memoria Local] Saldo de la cuenta: " << saldo_cache << " EUR" << endl;
            } else {
                MensajeRed msg;
                msg.tipo = OP_CONSULTAR_CUENTAS;

                // Si necesitas pasar un ID de cuenta, lo pides aquí:
                cout << "Introduce el ID de cuenta a consultar: ";
                cin >> msg.data;

                send(sock, (char*)&msg, sizeof(msg), 0);
                recv(sock, (char*)&msg, sizeof(msg), 0);

                // Guardamos el resultado en la caché (asumiendo que el servidor responde el número en texto)
                saldo_cache = atof(msg.data);
                cout << "[Servidor] Saldo actualizado: " << saldo_cache << " EUR" << endl;
            }

        } else if (opcion == 3) {
            MensajeRed msg;
            msg.tipo = OP_TRANSFERENCIA;

            cout << "Introduce los datos (ej: CuentaOrigen,CuentaDestino,Cantidad): ";
            cin.ignore();
            cin.getline(msg.data, sizeof(msg.data));

            send(sock, (char*)&msg, sizeof(msg), 0);
            recv(sock, (char*)&msg, sizeof(msg), 0);
            cout << "[Servidor]: " << msg.data << endl;

            // ¡IMPORTANTE! Al hacer una transferencia el saldo cambia,
            // así que invalidamos la caché para que la próxima consulta pida datos reales.
            saldo_cache = -1.0;

        } else if (opcion == 4) {
            MensajeRed msg;
            msg.tipo = OP_HISTORIAL;

            cout << "Introduce el ID de cuenta para ver su historial: ";
            cin >> msg.data;

            send(sock, (char*)&msg, sizeof(msg), 0);
            recv(sock, (char*)&msg, sizeof(msg), 0);
            cout << "\n--- HISTORIAL DE TRANSACCIONES ---" << endl;
            cout << msg.data << endl;

        } else if (opcion == 0) {
            MensajeRed msg;
            msg.tipo = OP_SALIR;
            send(sock, (char*)&msg, sizeof(msg), 0);
            cout << "Cerrando conexion con el banco. ¡Hasta pronto!" << endl;
        }

    }

    // 5. Limpieza
    closesocket(sock);
    WSACleanup();
    return 0;
}
