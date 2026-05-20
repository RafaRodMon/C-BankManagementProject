#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include "../protocolo.h"
#ifdef _MSC_VER
  #pragma comment(lib, "ws2_32.lib")
#endif
using namespace std;

typedef struct {
	int id;
	string nombre;
	string cuenta;
	bool cargado = false;
}DatosUsuario;

typedef struct {
	string nombreCuenta;
	string datos;
	bool cargado = false;
}HistorialCache;

DatosUsuario usuario_cache;
HistorialCache historial_cache;


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
				if (respuesta.find("Error") != string::npos) {
					cout << "\n[Servidor]: " << msg.data << endl;
					sesion_iniciada = false;
				} else {
					sesion_iniciada = true;
					usuario_cache.cuenta = respuesta;
					usuario_cache.cargado = true;

					cout << "\nInicio de sesion correcto. Tu cuenta operativa es: " << usuario_cache.cuenta << endl;
					cin.get();
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
					cout << "Registro completado con exito! Por favor, inicie sesion.\n";
					cin.ignore();
					cin.get();
					sesion_iniciada = false;
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
						cout << "[Memoria Local] Saldo de tu cuenta (" << usuario_cache.cuenta << "): " << saldo_cache << " EUR" << endl;
					} else {
						MensajeRed msg;
						msg.tipo = OP_CONSULTAR_CUENTAS;

						strncpy(msg.data, usuario_cache.cuenta.c_str(), sizeof(msg.data) - 1);

						send(sock, (char*)&msg, sizeof(msg), 0);
						recv(sock, (char*)&msg, sizeof(msg), 0);

						string respuesta(msg.data);
						if (respuesta.find("Error") != string::npos) {
							cout << "[Servidor]: " << msg.data << endl;
						} else {
							saldo_cache = atof(msg.data);
							cout << "[Servidor] Saldo de tu cuenta (" << usuario_cache.cuenta << "): " << saldo_cache << " EUR" << endl;
						}
					}
				} else if (opcion_main == 2) {
					MensajeRed msg;
					msg.tipo = OP_TRANSFERENCIA;
					string cuenta_destino;
					float cantidad;

					cout << "\nREALIZAR TRANSFERENCIA";
					cout << "\n----------------------";
					cout << "Tu cuenta origen es: " << usuario_cache.cuenta << endl;
					cout << "Introduce el numero de cuenta DESTINO: ";
					cin >> cuenta_destino;
					cout << "Introduce la cantidad a transferir: ";
					cin >> cantidad;

					snprintf(msg.data, sizeof(msg.data), "%s,%s,%.2f",
							usuario_cache.cuenta.c_str(), cuenta_destino.c_str(), cantidad);

					send(sock, (char*)&msg, sizeof(msg), 0);
					recv(sock, (char*)&msg, sizeof(msg), 0);
					cout << "[Servidor]: " << msg.data << endl;

					saldo_cache = -1.0;
					historial_cache.cargado = false;
				} else if (opcion_main == 3) {
					if (historial_cache.cargado && historial_cache.nombreCuenta == usuario_cache.cuenta) {
						cout << "\n--- HISTORIAL DE TU CUENTA (" << usuario_cache.cuenta << ") [Memoria Local] ---" << endl;
						cout << historial_cache.datos << endl;
					} else {
						MensajeRed msg;
						msg.tipo = OP_HISTORIAL;

						strncpy(msg.data, usuario_cache.cuenta.c_str(), sizeof(msg.data) - 1);

						send(sock, (char*)&msg, sizeof(msg), 0);
						recv(sock, (char*)&msg, sizeof(msg), 0);

						cout << "\n--- HISTORIAL DE TU CUENTA (" << usuario_cache.cuenta << ") ---" << endl;
						cout << msg.data << endl;

						historial_cache.nombreCuenta = usuario_cache.cuenta;
						historial_cache.datos = msg.data;
						historial_cache.cargado = true;
					}
				} else if (opcion_main == 4) {
					MensajeRed msg;
					msg.tipo = OP_DEPOSITAR;
					float cantidad;

					// YA NO PEDIMOS LA CUENTA POR TECLADO
					cout << "\nTu cuenta operativa es: " << usuario_cache.cuenta << endl;
					cout << "Introduce la cantidad a depositar: ";
					cin >> cantidad;

					// Enviamos directamente la cuenta que tenemos guardada en la memoria del cliente
					snprintf(msg.data, sizeof(msg.data), "%s,%.2f", usuario_cache.cuenta.c_str(), cantidad);

					send(sock, (char*)&msg, sizeof(msg), 0);
					recv(sock, (char*)&msg, sizeof(msg), 0);
					cout << "[Servidor]: " << msg.data << endl;

					saldo_cache = -1.0;
					historial_cache.cargado = false;

				} else if (opcion_main == 5) {
					MensajeRed msg;
					msg.tipo = OP_RETIRAR;
					float cantidad;

					cout << "\nTu cuenta operativa es: " << usuario_cache.cuenta << endl;
					cout << "Introduce la cantidad a retirar: ";
					cin >> cantidad;

					snprintf(msg.data, sizeof(msg.data), "%s,%.2f", usuario_cache.cuenta.c_str(), cantidad);

					send(sock, (char*)&msg, sizeof(msg), 0);
					recv(sock, (char*)&msg, sizeof(msg), 0);
					cout << "[Servidor]: " << msg.data << endl;

					saldo_cache = -1.0;
					historial_cache.cargado = false;

				} else if (opcion_main == 0) {
					cout << "Cerrando sesion activa... Volviendo al menu de acceso." << endl;
					sesion_iniciada = false;
					opcion_main = 0;

					// Limpiar toda la caché al cerrar sesión
					saldo_cache = -1.0;
					historial_cache.cargado = false;
					historial_cache.nombreCuenta = "";
					historial_cache.datos = "";
					usuario_cache.cargado = false;
					usuario_cache.nombre = "";
				}
			}
		}
	}

	// 5. Limpieza final de sockets
	closesocket(sock);
	WSACleanup();
	return 0;
}
