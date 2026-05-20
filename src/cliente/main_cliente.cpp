#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
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
   serv_addr.sin_port = htons(8080);
   serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

   // 4. Conectar
   if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
       cerr << "No se pudo conectar con el servidor bancario" << endl;
       closesocket(sock);
       WSACleanup();
       return 1;
   }
   cout << "Conectado al servidor con exito." << endl;

   // --- VARIABLES DE CONTROL DE FLUJO ---
   int opcion_acceso = -1;
   bool sesion_iniciada = false;
   float saldo_cache = -1.0;

   // BUCLE MAESTRO DE LA APLICACIÓN
   while (opcion_acceso != 0) {

       //   FASE A: MENÚ DE ACCESO (ANTES DE INICIAR SESIÓN)
       if (!sesion_iniciada) {
           cout << "\n===================================";
           cout << "\n       BIENVENIDO AL BANCO         ";
           cout << "\n===================================";
           cout << "\n1. Iniciar Sesion (Login)";
           cout << "\n2. Registrarse";
           cout << "\n0. Salir";
           cout << "\n===================================";
           cout << "\nSeleccion: ";
           cin >> opcion_acceso;

           cin.ignore((numeric_limits<streamsize>::max)(), '\n');

           if (opcion_acceso == 1) {
               string usuario, contrasenya;

               cout << "\nINICIO DE SESION";
               cout << "\n----------------";
               cout << "\nIntroduzca el usuario: ";
               cin >> usuario;
               cout << "Introduzca la contrasenya: ";
               cin >> contrasenya;

               MensajeRed msg;
               msg.tipo = OP_LOGIN;
               snprintf(msg.data, sizeof(msg.data), "%s,%s", usuario.c_str(), contrasenya.c_str());

               send(sock, (char*)&msg, sizeof(msg), 0);
               recv(sock, (char*)&msg, sizeof(msg), 0);

               // --- LÓGICA DE VALIDACIÓN DE LOGIN ---
               string respuesta(msg.data);
               // Si la respuesta del servidor contiene "Error", "incorrecto" o "no existe"
               if (respuesta.find("Error") != string::npos || respuesta.find("no existe") != string::npos || respuesta.find("incorrecta") != string::npos) {
                   cout << "\n[Servidor]: " << msg.data << endl;
                   cout << "Acceso denegado. El usuario no existe o la clave esta mal." << endl;
                   sesion_iniciada = false; // Bloqueado, se queda en este menú
               } else {
                   cout << "\n[Servidor]: " << msg.data << endl;
                   cout << "Conexion con el servidor realizada con exito! Presione cualquier tecla para continuar";
                   cin.ignore();
                   cin.get();
                   sesion_iniciada = true; // Desbloqueado, pasa al cajero
               }
           } else if (opcion_acceso == 2) {
               string usuario, contrasenya, apellido, dni;

               cout << "\nREGISTRO DE NUEVA CUENTA";
               cout << "\n------------------------";
               cout << "\nIntroduzca el nombre: ";
               cin >> usuario;
               cout << "Introduzca el apellido: ";
               cin >> apellido;
               cout << "Introduzca el DNI: ";
               cin >> dni;
               cout << "Introduzca la contrasenya: ";
               cin >> contrasenya;

               MensajeRed msg;
               msg.tipo = OP_REGISTRO;
               snprintf(msg.data, sizeof(msg.data), "%s,%s,%s,%s",
                        usuario.c_str(), apellido.c_str(), dni.c_str(), contrasenya.c_str());

               send(sock, (char*)&msg, sizeof(msg), 0);
               recv(sock, (char*)&msg, sizeof(msg), 0);

               string respuesta(msg.data);
               if (respuesta.find("Error") != string::npos || respuesta.find("ya existe") != string::npos) {
                   cout << "\n[Servidor]: " << msg.data << endl;
                   cout << "No se pudo registrar el usuario." << endl;
                   sesion_iniciada = false;
               } else {
                   cout << "\n[Servidor]: " << msg.data << endl;
                   cout << "Registro completado con exito! Presione cualquier tecla para continuar";
                   cin.ignore();
                   cin.get();
                   sesion_iniciada = true;
               }

           } else if (opcion_acceso == 0) {
               MensajeRed msg;
               msg.tipo = OP_SALIR;
               send(sock, (char*)&msg, sizeof(msg), 0);
               cout << "Cerrando conexion con el banco. ¡Hasta pronto!" << endl;
           }
       }

       //   FASE B: MENÚ PRINCIPAL OPERATIVO (SESIÓN ACTIVA)
       else {
           int opcion_main = -1;
           while (opcion_main != 0) {
               cout << "\n===================================";
               cout << "\n     SISTEMA BANCARIO - CAJERO     ";
               cout << "\n===================================";
               cout << "\n1. Consultar Cuentas / Saldo";
               cout << "\n2. Realizar Transferencia";
               cout << "\n3. Ver Historial de Transacciones";
               cout << "\n4. Depositar Dinero";
               cout << "\n5. Retirar Dinero";
               cout << "\n0. Cerrar Sesion";
               cout << "\n===================================";
               cout << "\nSeleccion: ";
               cin >> opcion_main;

               if (opcion_main == 1) {
                   if (saldo_cache != -1.0) {
                       cout << "[Memoria Local] Saldo de la cuenta: " << saldo_cache << " EUR" << endl;
                   } else {
                       MensajeRed msg;
                       msg.tipo = OP_CONSULTAR_CUENTAS;

                       string entrada_cuenta;
                       bool formato_valido = false;

                       // Bucle hasta que el usuario meta el formato correcto
                       while (!formato_valido) {
                           cout << "\nCONSULTA DE SALDO";
                           cout << "\n-----------------";
                           cout << "\nIntroduce el ID de cuenta (Formato: SBN-XXXXXX): ";
                           cin >> entrada_cuenta;

                           // Validación básica: Debe medir 10 caracteres y empezar por "SBN-"
                           if (entrada_cuenta.length() == 10 && entrada_cuenta.substr(0, 4) == "SBN-") {
                               formato_valido = true;
                           } else {
                               cout << "[ERROR] Formato incorrecto. Recuerda usar 'SBN-' seguido de 6 digitos (Ej: SBN-001234).\n";
                           }
                       }

                       // Copiamos la cadena validada a la estructura que viaja al servidor
                       strncpy(msg.data, entrada_cuenta.c_str(), sizeof(msg.data) - 1);

                       send(sock, (char*)&msg, sizeof(msg), 0);
                       recv(sock, (char*)&msg, sizeof(msg), 0);

                       // --- LÓGICA DE VALIDACIÓN DE CUENTA ---
                       string respuesta(msg.data);
                       if (respuesta.find("Error") != string::npos || respuesta.find("no existe") != string::npos) {
                           // Si el servidor avisa que la cuenta no existe, mostramos el aviso y NO guardamos nada
                           cout << "[Servidor]: " << msg.data << endl;
                           saldo_cache = -1.0;
                       } else {
                           // Si devuelve un número limpio, actualizamos la caché con éxito
                           saldo_cache = atof(msg.data);
                           cout << "[Servidor] Saldo actualizado: " << saldo_cache << " EUR" << endl;
                       }
                   }
               } else if (opcion_main == 2) {
                   MensajeRed msg;
                   msg.tipo = OP_TRANSFERENCIA;
                   cout << "Introduce los datos (ej: CuentaOrigen,CuentaDestino,Cantidad): ";
                   cin.ignore();
                   cin.getline(msg.data, sizeof(msg.data));

                   send(sock, (char*)&msg, sizeof(msg), 0);
                   recv(sock, (char*)&msg, sizeof(msg), 0);
                   cout << "[Servidor]: " << msg.data << endl;

                   saldo_cache = -1.0;

               } else if (opcion_main == 3) {
                   MensajeRed msg;
                   msg.tipo = OP_HISTORIAL;
                   cout << "Introduce el ID de cuenta para ver su historial: ";
                   cin >> msg.data;

                   send(sock, (char*)&msg, sizeof(msg), 0);
                   recv(sock, (char*)&msg, sizeof(msg), 0);
                   cout << "\n--- HISTORIAL DE TRANSACCIONES ---" << endl;
                   cout << msg.data << endl;

               } else if (opcion_main == 4) {
                   MensajeRed msg;
                   msg.tipo = OP_DEPOSITAR;
                   char cuenta[50];
                   float cantidad;
                   cout << "Introduce el numero de cuenta: ";
                   cin >> cuenta;
                   cout << "Introduce la cantidad a depositar: ";
                   cin >> cantidad;

                   snprintf(msg.data, sizeof(msg.data), "%s,%.2f", cuenta, cantidad);
                   send(sock, (char*)&msg, sizeof(msg), 0);
                   recv(sock, (char*)&msg, sizeof(msg), 0);
                   cout << "[Servidor]: " << msg.data << endl;

                   saldo_cache = -1.0;

               } else if (opcion_main == 5) {
                   MensajeRed msg;
                   msg.tipo = OP_RETIRAR;
                   char cuenta[50];
                   float cantidad;
                   cout << "Introduce el numero de cuenta: ";
                   cin >> cuenta;
                   cout << "Introduce la cantidad a retirar: ";
                   cin >> cantidad;

                   snprintf(msg.data, sizeof(msg.data), "%s,%.2f", cuenta, cantidad);
                   send(sock, (char*)&msg, sizeof(msg), 0);
                   recv(sock, (char*)&msg, sizeof(msg), 0);
                   cout << "[Servidor]: " << msg.data << endl;

                   saldo_cache = -1.0;

               } else if (opcion_main == 0) {
                   cout << "Cerrando sesion activa... Volviendo al menu de acceso." << endl;
                   sesion_iniciada = false;
                   opcion_main = 0;
               }
           }
       }
   }

   // 5. Limpieza final de sockets
   closesocket(sock);
   WSACleanup();
   return 0;
}
