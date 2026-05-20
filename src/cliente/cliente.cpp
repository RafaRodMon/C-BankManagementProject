#include <iostream>
#include <string>
#include <stdexcept>
#include <winsock2.h>
#include "../protocolo.h"

using namespace std;

class ErrorConexion : public runtime_error {
public:
    ErrorConexion(const string& msg) : runtime_error(msg) {}
};

class BancoCliente {
private:
    SOCKET sock;
    struct sockaddr_in serv_addr;
    bool conectado = false;

public:
    BancoCliente() {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        sock = INVALID_SOCKET;
    }

    ~BancoCliente() {
        cerrar();
    }

    BancoCliente(const BancoCliente&) = delete;
    BancoCliente& operator=(const BancoCliente&) = delete;

    bool conectar(string ip, int puerto) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(puerto);
        serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            throw ErrorConexion("Error de conexion con " + ip);
        }
        conectado = true;
        return true;
    }

    MensajeRed enviarPeticion(const MensajeRed& peticion) {
        if (!conectado) throw ErrorConexion("No hay conexión activa");

        MensajeRed respuesta = peticion;
        send(sock, (char*)&respuesta, sizeof(respuesta), 0);
        recv(sock, (char*)&respuesta, sizeof(respuesta), 0);
        return respuesta;
    }

    bool estaConectado() const {
        return conectado;
    }

    void cerrar() {
        if (conectado) {
            closesocket(sock);
            WSACleanup();
            conectado = false;
        }
    }
};
