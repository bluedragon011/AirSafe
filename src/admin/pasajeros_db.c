#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../shared/sqlite3.h"
#include "../shared/models.h"

//Usamos la variable global parametrizada por el fichero de configuración de la Fase 1
extern char RUTA_DB[150]; 

// FUNCIONALIDAD 1: COMPRAR VUELO CON CONTROL DE AFORO REAL
int comprarVueloDB(int id_usuario, int id_vuelo) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    int asientos_reservados = 0;
    int capacidad_total = 0;

    if (sqlite3_open(RUTA_DB, &db) != SQLITE_OK) {
        printf(">> [ERROR] No se pudo abrir la BD para la compra.\n");
        return 0;
    }

    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, NULL, NULL);

    const char *sql_capacidad = "SELECT CC.num_filas * CC.asientos_por_fila "
                                "FROM Vuelos V "
                                "JOIN Config_cabina CC ON V.id_avion = CC.id_avion "
                                "WHERE V.id_vuelo = ?;";
    
    if (sqlite3_prepare_v2(db, sql_capacidad, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_vuelo);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            capacidad_total = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    //Si no tiene una configuración de cabina explícita, asignamos un aforo estándar (ej. 150)
    if (capacidad_total == 0) {
        capacidad_total = 150; 
    }

    const char *sql_contar = "SELECT COUNT(*) FROM Reserva WHERE id_vuelo = ?;";
    if (sqlite3_prepare_v2(db, sql_contar, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_vuelo);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            asientos_reservados = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (asientos_reservados >= capacidad_total) {
        printf(">> [DENEGADO] Error: El vuelo con ID %d esta completo (%d/%d asientos).\n", 
               id_vuelo, asientos_reservados, capacidad_total);
        sqlite3_close(db);
        return -1; //Código de error para vuelo lleno
    }

    const char *sql_insert = "INSERT INTO Reserva (id_usuario, fecha_reserva, precio, id_vuelo, id_asiento) "
                             "VALUES (?, date('now'), 89.99, ?, ?);";

    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL) != SQLITE_OK) {
        printf(">> [ERROR SQL] %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }

    int siguiente_asiento = asientos_reservados + 1;

    sqlite3_bind_int(stmt, 1, id_usuario);
    sqlite3_bind_int(stmt, 2, id_vuelo);
    sqlite3_bind_int(stmt, 3, siguiente_asiento);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf(">> [EXITO] Reserva completada. Usuario %d -> Vuelo %d (Asiento asignado: %d).\n", 
               id_usuario, id_vuelo, siguiente_asiento);
        // TODO: llamar a la funcion para escribir en el fichero log
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    } else {
        printf(">> [ERROR] Fallo en la persistencia. Verifique los IDs de Usuario y Vuelo.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
}

//FUNCIONALIDAD 2: CONSULTAR HISTORIAL (PASADOS Y FUTUROS SEPARADOS)
void consultarMisVuelosDB(int id_usuario) {
    sqlite3 *db;
    sqlite3_stmt *stmt;

    if (sqlite3_open(RUTA_DB, &db) != SQLITE_OK) {
        printf(">> [ERROR] No se pudo abrir la BD para la consulta.\n");
        return;
    }

    const char *sql_futuros = "SELECT R.id_reserva, V.fecha_salida, V.ruta, R.precio, R.id_asiento "
                              "FROM Reserva R JOIN Vuelos V ON R.id_vuelo = V.id_vuelo "
                              "WHERE R.id_usuario = ? AND V.fecha_salida >= datetime('now') "
                              "ORDER BY V.fecha_salida ASC;";

    if (sqlite3_prepare_v2(db, sql_futuros, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_usuario);
        printf("\n===================================================\n");
        printf("     PROXIMOS VUELOS / RESERVAS ACTIVAS (Usuario %d) \n", id_usuario);
        printf("===================================================\n");
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            count++;
            printf("  [%d] Reserva #%d | Fecha: %s | Trayecto: %s | Asiento: %d | Importe: %.2f EUR\n",
                   count, sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2), sqlite3_column_int(stmt, 4), sqlite3_column_double(stmt, 3));
        }
        if (count == 0) printf("  No tienes itinerarios planificados próximamente.\n");
        sqlite3_finalize(stmt);
    }

    // --- PARTE 2: VUELOS PASADOS (Historial de viajes) ---
    const char *sql_pasados = "SELECT R.id_reserva, V.fecha_salida, V.ruta, R.precio "
                              "FROM Reserva R JOIN Vuelos V ON R.id_vuelo = V.id_vuelo "
                              "WHERE R.id_usuario = ? AND V.fecha_salida < datetime('now') "
                              "ORDER BY V.fecha_salida DESC;";

    if (sqlite3_prepare_v2(db, sql_pasados, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_usuario);
        printf("\n===================================================\n");
        printf("     HISTORIAL DE VUELOS REALIZADOS (PASADOS)     \n");
        printf("===================================================\n");
        int count = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            count++;
            printf("  ✓ Reserva #%d | Fecha de viaje: %s | Trayecto: %s | Importe: %.2f EUR\n",
                   sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2), sqlite3_column_double(stmt, 3));
        }
        if (count == 0) printf("  Tu historial de viajes volados está vacío.\n");
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
}