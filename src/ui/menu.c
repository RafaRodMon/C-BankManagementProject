/*
 * menu.c
 *
 *  Created on: 3 abr 2026
 *      Author: ander.belloso
 */

#include "menu.h"
#include <stdio.h>
#include <string.h>
#include "../datos/sqlite3.h"

int login(Config *cfg, sqlite3 *db) {
    char usuario[100];
    char contrasenya[100];

    printf("Introduce el usuario: \n");
    fflush(stdout);
    scanf("%s", usuario);
    printf("Introduce la contrasenya: \n");
    fflush(stdout);
    scanf("%s", contrasenya);

    char sql[200];
    sprintf(sql, "SELECT * FROM Cliente WHERE nombre='%s' AND contrasenya='%s';", usuario, contrasenya);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
    	int id = sqlite3_column_int(stmt, 0);
    	printf("DEBUG id_cliente=%d\n", id);
        sqlite3_finalize(stmt);
        return id;
    }

    sqlite3_finalize(stmt);
    return -1;
}

void menuLogin(Config *cfg, sqlite3 *db) {
    int opcion;
    int id_cliente;
    do {
        printf("\n====== BIENVENIDO ======\n");
        printf("1. INICIAR SESION\n");
        printf("2. REGISTRARSE\n");
        printf("3. SALIR\n");
        printf("ELIJA UNA OPCION: ");
        scanf("%d", &opcion);

        switch(opcion) {
        case 1:
        	id_cliente = login(cfg, db);
        	if (id_cliente != -1) {
        	    menuPrincipal(db, id_cliente);  // pasar id_cliente al menú
        	} else {
        		printf("Usuario o contrasenya incorrectos\n");
        	}
        	break;

        case 2: altaCliente(db); break;
        case 3: printf("SALIENDO...\n"); break;
        default: printf("Opcion no valida\n");
        }
    } while(opcion != 3);
}

void menuPrincipal(sqlite3 *db, int id_cliente) {
    int opcion;
    do {
        printf("\n====== MENU PRINCIPAL ======\n");
        printf("1. CREAR UNA NUEVA CUENTA BANCARIA\n");
        printf("2. CONSULTAR EL SALDO DE MIS CUENTAS\n");
        printf("3. REALIZAR DEPOSITO DE DINERO\n");
        printf("4. REALIZAR RETIRADA DE DINERO\n");
        printf("5. REALIZAR TRANSFERENCIA ENTRE CUENTAS\n");
        printf("6. CONSULTAR HISTORIAL DE MOVIMIENTOS\n");
        printf("7. SALIR\n");
        printf("ELIJA UNA OPCION: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: crearCuenta(db, id_cliente); break;
            case 2: consultarSaldo(db,id_cliente); break;
            case 3: depositarDinero(db, id_cliente); break;
            case 4: retirarDinero(db, id_cliente); break;
            case 5: transferirDinero(db, id_cliente); break;
            case 6: consultarHistorial(db, id_cliente); break;
            case 7: printf("Saliendo...\n"); break;
            default: printf("Opcion no valida\n");
        }
    } while(opcion != 7);
}
