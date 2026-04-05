/*
 * menu.h
 *
 *  Created on: 3 abr 2026
 *      Author: ander.belloso
 */

#ifndef SRC_MENU_H_
#define SRC_MENU_H_

#include "config.h"
#include "sqlite3.h"
#include "clientes.h"

void menuLogin(Config *cfg, sqlite3 *db);

void menuPrincipal(sqlite3 *db, int id_cliente);

#endif /* SRC_MENU_H_ */
