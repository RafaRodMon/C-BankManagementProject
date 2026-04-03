/*
 * menu.c
 *
 *  Created on: 3 abr 2026
 *      Author: ander.belloso
 */

#include "menu.h"
#include <stdio.h>
#include <string.h>

int login(Config *cfg) {
    char usuario[100];
    char contrasenya[100];

    printf("Introduce el usuario: \n");
    fflush(stdout);
    scanf("%s", usuario);
    printf("Introduce la contrasenya: \n");
    fflush(stdout);
    scanf("%s", contrasenya);

    if (strcmp(usuario, cfg->admin_usuario) == 0 && strcmp(contrasenya, cfg->admin_password) == 0) {
        return 0;
    }
    return -1;
}

void menuPrincipal(sqlite3 *db) {
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
        printf("Elige una opcion: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: printf("CREAR UNA NUEVA CUENTA BANCARIA\n"); break;
            case 2: printf("CONSULTAR EL SALDO DE MIS CUENTAS\n"); break;
            case 3: printf("REALIZAR DEPOSITO DE DINERO\n"); break;
            case 4: printf("REALIZAR RETIRADA DE DINERO\n"); break;
            case 5: printf("REALIZAR TRANSFERENCIA ENTRE CUENTAS\n"); break;
            case 6: printf("CONSULTAR HISTORIAL DE MOVIMIENTOS\n"); break;
            case 7: printf("Saliendo...\n"); break;
            default: printf("Opcion no valida\n");
        }
    } while(opcion != 7);
}
