/*
 * acciones.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */
#include <stdio.h>
#include <string.h>
#include "acciones.h"
#include "datos/sqlite3.h"

void mostrarMercado(sqlite3 *db) {
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT id_accion, nombre, precio_actual FROM Accion;", -1, &stmt, NULL);

    printf("\n=== MERCADO DE ACCIONES ===\n");
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        printf("___________________________________________\n");
        printf("ID: %d | Nombre: %s | Precio: %.2f EUR\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_double(stmt, 2));
    }
    if (!found) printf("No hay acciones disponibles en el mercado\n");
    sqlite3_finalize(stmt);
}

void comprarAccion(sqlite3 *db, int id_cliente) {
    int id_accion;
    int cantidad;
    char nombreCuenta[50];

    mostrarMercado(db);

    printf("\nID de la accion a comprar: \n");
    fflush(stdout);
    scanf("%d", &id_accion);
    printf("Cantidad de titulos a comprar: \n");
    fflush(stdout);
    scanf("%d", &cantidad);
    printf("Nombre de la cuenta con la que pagar: \n");
    fflush(stdout);
    scanf("%s", nombreCuenta);

    if (cantidad <= 0) {
        printf("La cantidad debe ser mayor que 0\n");
        return;
    }

    /* Obtener precio actual de la accion */
    char sqlPrecio[200];
    sprintf(sqlPrecio, "SELECT precio_actual, nombre FROM Accion WHERE id_accion = %d;", id_accion);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sqlPrecio, -1, &stmt, NULL);

    float precio_actual = -1;
    char nombreAccion[50] = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        precio_actual = (float)sqlite3_column_double(stmt, 0);
        strcpy(nombreAccion, (char*)sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    if (precio_actual < 0) {
        printf("Accion no encontrada\n");
        return;
    }

    float coste_total = precio_actual * cantidad;
    printf("Coste total: %.2f EUR. Confirmar? (1=Si / 0=No): ", coste_total);
    int confirmar;
    scanf("%d", &confirmar);
    if (confirmar != 1) {
        printf("Compra cancelada\n");
        return;
    }

    /* Descontar dinero de la cuenta del cliente */
    char sqlPago[300];
    sprintf(sqlPago,
        "UPDATE Cuenta SET saldo = saldo - %.2f "
        "WHERE nombreCuenta = '%s' AND id_cliente = %d AND saldo >= %.2f;",
        coste_total, nombreCuenta, id_cliente, coste_total);

    int result = sqlite3_exec(db, sqlPago, NULL, NULL, NULL);
    if (result != SQLITE_OK || sqlite3_changes(db) == 0) {
        printf("Saldo insuficiente o cuenta no encontrada\n");
        return;
    }

    /* Obtener id_cartera del cliente */
    char sqlCartera[200];
    sprintf(sqlCartera, "SELECT id_cartera FROM Cartera WHERE id_cliente = %d;", id_cliente);
    sqlite3_prepare_v2(db, sqlCartera, -1, &stmt, NULL);

    int id_cartera = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_cartera = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (id_cartera == -1) {
        /* Si no tiene cartera, la creamos */
        char sqlNuevaCartera[200];
        sprintf(sqlNuevaCartera, "INSERT INTO Cartera (id_cliente) VALUES (%d);", id_cliente);
        sqlite3_exec(db, sqlNuevaCartera, NULL, NULL, NULL);
        id_cartera = (int)sqlite3_last_insert_rowid(db);
    }

    /* Comprobar si ya tiene esa accion para calcular precio medio ponderado */
    char sqlExiste[300];
    sprintf(sqlExiste,
        "SELECT cantidad, precio_compra FROM AccionCartera "
        "WHERE id_cartera = %d AND id_accion = %d;", id_cartera, id_accion);
    sqlite3_prepare_v2(db, sqlExiste, -1, &stmt, NULL);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        /* Ya tiene la accion: actualizar con precio medio ponderado */
        int cantidad_anterior = sqlite3_column_int(stmt, 0);
        float precio_anterior = (float)sqlite3_column_double(stmt, 1);
        sqlite3_finalize(stmt);

        float precio_medio = (cantidad_anterior * precio_anterior + cantidad * precio_actual)
                             / (cantidad_anterior + cantidad);

        char sqlUpdate[400];
        sprintf(sqlUpdate,
            "UPDATE AccionCartera SET cantidad = cantidad + %d, precio_compra = %.4f "
            "WHERE id_cartera = %d AND id_accion = %d;",
            cantidad, precio_medio, id_cartera, id_accion);
        sqlite3_exec(db, sqlUpdate, NULL, NULL, NULL);
    } else {
        sqlite3_finalize(stmt);
        /* Primera vez que compra esta accion */
        char sqlInsert[400];
        sprintf(sqlInsert,
            "INSERT INTO AccionCartera (id_cartera, id_accion, cantidad, precio_compra) "
            "VALUES (%d, %d, %d, %.4f);",
            id_cartera, id_accion, cantidad, precio_actual);
        sqlite3_exec(db, sqlInsert, NULL, NULL, NULL);
    }

    printf("Compra realizada: %d titulos de %s a %.2f EUR/titulo\n",
        cantidad, nombreAccion, precio_actual);
}

void venderAccion(sqlite3 *db, int id_cliente) {
    int id_accion;
    int cantidad;
    char nombreCuenta[50];

    /* Obtener cartera del cliente */
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

    /* Mostrar acciones que tiene el cliente */
    char sqlMostrar[400];
    sprintf(sqlMostrar,
        "SELECT AC.id_accion, A.nombre, AC.cantidad, AC.precio_compra, A.precio_actual "
        "FROM AccionCartera AC "
        "JOIN Accion A ON AC.id_accion = A.id_accion "
        "WHERE AC.id_cartera = %d;", id_cartera);
    sqlite3_prepare_v2(db, sqlMostrar, -1, &stmt, NULL);

    printf("\n=== TUS ACCIONES ===\n");
    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        printf("___________________________________________\n");
        printf("ID: %d | %s | Cantidad: %d | Precio compra: %.2f | Precio actual: %.2f\n",
            sqlite3_column_int(stmt, 0),
            sqlite3_column_text(stmt, 1),
            sqlite3_column_int(stmt, 2),
            sqlite3_column_double(stmt, 3),
            sqlite3_column_double(stmt, 4));
    }
    sqlite3_finalize(stmt);

    if (!found) {
        printf("No tienes acciones en tu cartera\n");
        return;
    }

    printf("\nID de la accion a vender: \n");
    fflush(stdout);
    scanf("%d", &id_accion);
    printf("Cantidad de titulos a vender: \n");
    fflush(stdout);
    scanf("%d", &cantidad);
    printf("Nombre de la cuenta donde recibir el dinero: \n");
    fflush(stdout);
    scanf("%s", nombreCuenta);

    if (cantidad <= 0) {
        printf("La cantidad debe ser mayor que 0\n");
        return;
    }

    /* Verificar que tiene suficientes titulos */
    char sqlVerificar[300];
    sprintf(sqlVerificar,
        "SELECT cantidad FROM AccionCartera "
        "WHERE id_cartera = %d AND id_accion = %d;", id_cartera, id_accion);
    sqlite3_prepare_v2(db, sqlVerificar, -1, &stmt, NULL);

    int cantidad_disponible = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cantidad_disponible = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (cantidad_disponible == 0) {
        printf("No tienes esa accion en tu cartera\n");
        return;
    }
    if (cantidad > cantidad_disponible) {
        printf("No tienes suficientes titulos. Tienes %d\n", cantidad_disponible);
        return;
    }

    /* Obtener precio actual */
    char sqlPrecio[200];
    sprintf(sqlPrecio, "SELECT precio_actual, nombre FROM Accion WHERE id_accion = %d;", id_accion);
    sqlite3_prepare_v2(db, sqlPrecio, -1, &stmt, NULL);

    float precio_actual = 0;
    char nombreAccion[50] = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        precio_actual = (float)sqlite3_column_double(stmt, 0);
        strcpy(nombreAccion, (char*)sqlite3_column_text(stmt, 1));
    }
    sqlite3_finalize(stmt);

    float ingreso_total = precio_actual * cantidad;

    /* Ingresar dinero en la cuenta */
    char sqlIngreso[300];
    sprintf(sqlIngreso,
        "UPDATE Cuenta SET saldo = saldo + %.2f "
        "WHERE nombreCuenta = '%s' AND id_cliente = %d;",
        ingreso_total, nombreCuenta, id_cliente);

    int result = sqlite3_exec(db, sqlIngreso, NULL, NULL, NULL);
    if (result != SQLITE_OK || sqlite3_changes(db) == 0) {
        printf("Cuenta no encontrada\n");
        return;
    }

    /* Actualizar o borrar de AccionCartera */
    if (cantidad == cantidad_disponible) {
        /* Vende todos: borrar la fila */
        char sqlBorrar[300];
        sprintf(sqlBorrar,
            "DELETE FROM AccionCartera WHERE id_cartera = %d AND id_accion = %d;",
            id_cartera, id_accion);
        sqlite3_exec(db, sqlBorrar, NULL, NULL, NULL);
    } else {
        /* Vende parte: reducir cantidad */
        char sqlReducir[300];
        sprintf(sqlReducir,
            "UPDATE AccionCartera SET cantidad = cantidad - %d "
            "WHERE id_cartera = %d AND id_accion = %d;",
            cantidad, id_cartera, id_accion);
        sqlite3_exec(db, sqlReducir, NULL, NULL, NULL);
    }
    printf("Venta realizada: %d titulos de %s a %.2f EUR/titulo. Ingresado: %.2f EUR\n",
        cantidad, nombreAccion, precio_actual, ingreso_total);
}

