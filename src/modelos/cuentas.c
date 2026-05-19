/*
 * cuentas.c
 *
 *  Created on: Apr 3, 2026
 *      Author: rafar_l7ukexx
 */

#include <stdio.h>
#include <string.h>
#include "cuentas.h"
#include "../datos/sqlite3.h"
#include "../protocolo.h" // Asegúrate de incluirlo para tener la estructura MensajeRed

// 1. CREAR CUENTA AUTOMÁTICA AL REGISTRARSE (Sin pedir datos por consola)
void crearCuentaAutomatica(sqlite3 *db, int id_cliente, char *msg_out) {
    char nombreCuenta[50];
    // Generamos un número de cuenta estándar basado en su ID único (Ej: SBN-000005)
    snprintf(nombreCuenta, sizeof(nombreCuenta), "SBN-%06d", id_cliente);
    float saldo_inicial = 100.00; // Regalo de bienvenida

    char sql[500];
    snprintf(sql, sizeof(sql), "INSERT INTO Cuenta (nombreCuenta, id_cliente, saldo) VALUES ('%s', %d, %.2f);",
            nombreCuenta, id_cliente, saldo_inicial);

    int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (result == SQLITE_OK) {
        snprintf(msg_out, 512, "Usuario registrado. Cuenta asignada: %s con %.2f EUR.", nombreCuenta, saldo_inicial);
    } else {
        snprintf(msg_out, 512, "Error interno del servidor al asignar cuenta bancaria.");
    }
}

// 2. CONSULTAR SALDO (Escribe el resultado en el buffer de red)
void consultarSaldoRed(sqlite3 *db, int id_cliente, char *msg_out) {
    char sql[200];
    snprintf(sql, sizeof(sql), "SELECT nombreCuenta, saldo FROM Cuenta WHERE id_cliente = %d;", id_cliente);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    // Inicializamos el mensaje de salida limpio
    msg_out[0] = '\0';

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        // Obtenemos el saldo y lo enviamos en crudo (o con formato) para que el cliente lo procese
        // Si el cliente usa atof(), es mejor enviar solo el número: "2550.75"
        snprintf(msg_out, 512, "%.2f", sqlite3_column_double(stmt, 1));
    } else {
        snprintf(msg_out, 512, "Error: No se encontró ninguna cuenta asociada a tu usuario.");
    }
    sqlite3_finalize(stmt);
}

// 3. DEPOSITAR DINERO (Los datos ya vienen extraídos de la red)
void depositarDineroRed(sqlite3 *db, int id_cliente, char *nombreCuenta, float deposito, char *msg_out) {
    char sql[300];
    snprintf(sql, sizeof(sql), "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s' AND id_cliente = %d;",
            deposito, nombreCuenta, id_cliente);

    int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
        snprintf(msg_out, 512, "Deposito de %.2f EUR realizado correctamente.", deposito);
        registrarMovimiento(db, nombreCuenta, "DEPOSITO", deposito, "", "");
    } else {
        snprintf(msg_out, 512, "Error: Cuenta no encontrada o no pertenece al usuario.");
    }
}

// 4. RETIRAR DINERO
void retirarDineroRed(sqlite3 *db, int id_cliente, char *nombreCuenta, float retiro, char *msg_out) {
    char sql[300];
    snprintf(sql, sizeof(sql), "UPDATE Cuenta SET saldo = saldo - %.2f WHERE nombreCuenta = '%s' AND id_cliente = %d AND saldo >= %.2f;",
            retiro, nombreCuenta, id_cliente, retiro);

    int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
        snprintf(msg_out, 512, "Retiro de %.2f EUR completado con exito.", retiro);
        registrarMovimiento(db, nombreCuenta, "RETIRO", retiro, "", "");
    } else {
        snprintf(msg_out, 512, "Error: Saldo insuficiente o cuenta no válida.");
    }
}

// 5. TRANSFERIR DINERO
void transferirDineroRed(sqlite3 *db, int id_cliente, char *ordenante, char *beneficiario, float transferencia, char *msg_out) {
    char sql[300];
    snprintf(sql, sizeof(sql), "UPDATE Cuenta SET saldo = saldo - %.2f WHERE nombreCuenta = '%s' AND id_cliente = %d AND saldo >= %.2f;",
            transferencia, ordenante, id_cliente, transferencia);

    char sql2[300];
    snprintf(sql2, sizeof(sql2), "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s';",
            transferencia, beneficiario);

    int result = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
        result = sqlite3_exec(db, sql2, NULL, NULL, NULL);
        if (result == SQLITE_OK && sqlite3_changes(db) > 0) {
            snprintf(msg_out, 512, "Transferencia de %.2f EUR realizada correctamente.", transferencia);
            registrarMovimiento(db, ordenante, "TRANSFERENCIA", transferencia, ordenante, beneficiario);
        } else {
            // Reversión de saldo si falla el destino (Sino el dinero desaparecería)
            char sql_rollback[300];
            snprintf(sql_rollback, sizeof(sql_rollback), "UPDATE Cuenta SET saldo = saldo + %.2f WHERE nombreCuenta = '%s';", transferencia, ordenante);
            sqlite3_exec(db, sql_rollback, NULL, NULL, NULL);
            snprintf(msg_out, 512, "Error: Cuenta beneficiaria no encontrada. Operacion cancelada.");
        }
    } else {
        snprintf(msg_out, 512, "Error: Saldo insuficiente o cuenta ordenante incorrecta.");
    }
}

// 6. CONSULTAR HISTORIAL (Concatena los movimientos en una sola cadena de texto)
void consultarHistorialRed(sqlite3 *db, int id_cliente, char *nombreCuenta, char *msg_out) {
    char sql[500];
    snprintf(sql, sizeof(sql),
        "SELECT M.tipo, M.importe, M.fecha, M.cuenta_ordenante, M.cuenta_beneficiaria "
        "FROM Movimiento M "
        "JOIN Cuenta C ON M.id_cuenta = C.id_cuenta "
        "WHERE C.nombreCuenta = '%s' AND C.id_cliente = %d;",
        nombreCuenta, id_cliente);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    char temp[150];
    snprintf(msg_out, 512, "=== HISTORIAL DE %s ===\n", nombreCuenta);
    int found = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        char *tipo = (char*)sqlite3_column_text(stmt, 0);
        float importe = sqlite3_column_double(stmt, 1);
        char *fecha = (char*)sqlite3_column_text(stmt, 2);

        snprintf(temp, sizeof(temp), "[%s] %s: %.2f EUR\n", fecha, tipo, importe);
        strncat(msg_out, temp, 512 - strlen(msg_out) - 1);
    }

    if (!found) {
        snprintf(msg_out, 512, "No hay movimientos registrados en esta cuenta.");
    }
    sqlite3_finalize(stmt);
}

void registrarMovimiento(sqlite3 *db, char *nombreCuenta, char *tipo,
		float importe, char *ordenante, char *beneficiaria) {

    char sqlId[200];
    snprintf(sqlId, sizeof(sqlId), "SELECT id_cuenta FROM Cuenta WHERE nombreCuenta = '%s';", nombreCuenta);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sqlId, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return; // Si falla la consulta por lo que sea, salimos para no romper el programa
    }

    int id_cuenta = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        id_cuenta = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    // Si no encontramos la cuenta, no podemos registrar el movimiento
    if (id_cuenta == -1) return;

    // Insertamos el movimiento usando la función datetime('now') propia de SQLite para la fecha
    char sql[500];
    snprintf(sql, sizeof(sql),
        "INSERT INTO Movimiento (id_cuenta, tipo, importe, fecha, cuenta_ordenante, cuenta_beneficiaria) "
        "VALUES (%d, '%s', %.2f, datetime('now'), '%s', '%s');",
        id_cuenta, tipo, importe, ordenante, beneficiaria);

    sqlite3_exec(db, sql, NULL, NULL, NULL);
}
