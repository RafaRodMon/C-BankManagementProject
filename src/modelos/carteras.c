/*
 * carteras.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#include <stdio.h>
#include "carteras.h"
#include "../datos/sqlite3.h"

void mostrarCartera(sqlite3 *db, int id_cliente) {
    /* Obtener id_cartera del cliente */
    char sqlCartera[200];
    sprintf(sqlCartera, "SELECT id_cartera FROM Cartera WHERE id_cliente = %d;", id_cliente);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sqlCartera, -1, &stmt, NULL);

    int id_cartera = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_cartera = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (id_cartera == -1) {
        printf("No tienes ninguna cartera\n");
        return;
    }

    /* Mostrar acciones con calculo de rentabilidad */
    char sqlAcciones[500];
    sprintf(sqlAcciones,
        "SELECT A.id_accion, A.nombre, AC.cantidad, AC.precio_compra, A.precio_actual, "
        "(A.precio_actual - AC.precio_compra) * AC.cantidad AS ganancia "
        "FROM AccionCartera AC "
        "JOIN Accion A ON AC.id_accion = A.id_accion "
        "WHERE AC.id_cartera = %d;", id_cartera);

    sqlite3_prepare_v2(db, sqlAcciones, -1, &stmt, NULL);

    printf("\n=== MI CARTERA ===\n");
    int found = 0;
    float total_ganancia = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        int id_accion       = sqlite3_column_int(stmt, 0);
        const char nombre  = (const char)sqlite3_column_text(stmt, 1);
        int cantidad         = sqlite3_column_int(stmt, 2);
        float precio_compra = (float)sqlite3_column_double(stmt, 3);
        float precio_actual = (float)sqlite3_column_double(stmt, 4);
        float ganancia      = (float)sqlite3_column_double(stmt, 5);

        total_ganancia += ganancia;

        printf("_______________\n");
        printf("ID: %d | %s\n", id_accion, nombre);
        printf("  Cantidad:       %d titulos\n", cantidad);
        printf("  Precio compra:  %.2f EUR\n", precio_compra);
        printf("  Precio actual:  %.2f EUR\n", precio_actual);
        printf("  Valor actual:   %.2f EUR\n", precio_actual * cantidad);
        printf("  Ganancia/Perdida: %+.2f EUR\n", ganancia);
    }
    sqlite3_finalize(stmt);

    if (!found) {
        printf("Tu cartera esta vacia\n");
    } else {
        printf("_______________\n");
        printf("GANANCIA/PERDIDA TOTAL: %+.2f EUR\n", total_ganancia);
    }
}
