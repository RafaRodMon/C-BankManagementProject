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

int login(Config *cfg);

void menuPrincipal(sqlite3 *db);

#endif /* SRC_MENU_H_ */
