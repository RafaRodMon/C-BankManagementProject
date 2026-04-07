/*
 * menu.h
 *
 *  Created on: 3 abr 2026
 *      Author: ander.belloso
 */

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

#include "../config/config.h"
#include "../datos/sqlite3.h"
#include "../modelos/clientes.h"

void menuLogin(Config *cfg, sqlite3 *db);

void menuPrincipal(sqlite3 *db, int id_cliente);

#endif /* SRC_MENU_H_ */
