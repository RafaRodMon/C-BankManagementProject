/*
 * cliente.cpp
 *
 *  Created on: 14 may 2026
 *      Author: i.tejedor
 */

#include <iostream>
#include <string>
#include <winsock2.h>
#include "../protocolo.h" // El archivo que definimos antes

using namespace std;

class BancoCliente {
private:
    SOCKET sock;
    struct sockaddr_in serv_addr;

public:
    BancoCliente() {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
    }

    bool conectar(string ip, int puerto) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(puerto);
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            cout << "Error de conexion" << endl;
            return false;
        }
        return true;
    }

    void enviarPeticion(MensajeRed& msg) {
        send(sock, (char*)&msg, sizeof(msg), 0);
        // Recibir respuesta
        recv(sock, (char*)&msg, sizeof(msg), 0);
    }

    void cerrar() {
        closesocket(sock);
        WSACleanup();
    }
};
