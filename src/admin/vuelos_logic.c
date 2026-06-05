#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../shared/sqlite3.h"
#include "../shared/auth.h"
#include "../shared/models.h"
#include "../shared/logs.h"

// FUNCION AUXILIAR PARA LIMPIAR RASTRO DEL TECLADO
void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Función centralizada para abrir la base de datos
sqlite3* abrirDB() {
    sqlite3 *db;
    int rc = sqlite3_open("data/airsafe.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error al abrir la base de datos: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

void crearVuelo() {
    sqlite3 *db = abrirDB();
    if (!db) return;

    Vuelo nuevo;
    sqlite3_stmt *stmt;

    printf("\n--- FORMULARIO: CREAR NUEVO VUELO ---\n");
    printf("ID del avion a asignar: ");
    scanf("%d", &nuevo.id_avion);
    limpiarBuffer();

    printf("Fecha de salida  (YYYY-MM-DD HH:MM:SS): ");
    fgets(nuevo.fecha_salida, sizeof(nuevo.fecha_salida), stdin);
    nuevo.fecha_salida[strcspn(nuevo.fecha_salida, "\n")] = 0;

    printf("Fecha de llegada (YYYY-MM-DD HH:MM:SS): ");
    fgets(nuevo.fecha_llegada, sizeof(nuevo.fecha_llegada), stdin);
    nuevo.fecha_llegada[strcspn(nuevo.fecha_llegada, "\n")] = 0;

    printf("Ruta (Ej: Barajas-Seve Ballesteros): ");
    fgets(nuevo.ruta, sizeof(nuevo.ruta), stdin);
    nuevo.ruta[strcspn(nuevo.ruta, "\n")] = 0;

    const char *sql = "INSERT INTO Vuelos (fecha_salida, fecha_llegada, id_avion, ruta) VALUES (?, ?, ?, ?);";  

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, nuevo.fecha_salida, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, nuevo.fecha_llegada, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, nuevo.id_avion);
        sqlite3_bind_text(stmt, 4, nuevo.ruta, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf(">> [EXITO] Vuelo registrado en la base de datos correctamente.\n");
        } else {
            printf(">> [ERROR] No se pudo insertar el vuelo (Verifica que el ID del avion existe).\n");
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void eliminarVuelo() {
    sqlite3 *db = abrirDB();
    if (!db) return;

    int id;
    sqlite3_stmt *stmt;

    printf("\n--- MODO: ELIMINAR VUELO ---\n");
    printf("Introduce el ID del vuelo que deseas eliminar: ");
    scanf("%d", &id);
    limpiarBuffer();

    const char *sql = "DELETE FROM Vuelos WHERE id_vuelo = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            int filas = sqlite3_changes(db);
            if (filas > 0)
                printf(">> [EXITO] Vuelo con ID %d eliminado.\n", id);
            else
                printf(">> [INFO] No se encontro ningun vuelo con ID %d.\n", id);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void modificarVuelo() {
    sqlite3 *db = abrirDB();
    if (!db) return;

    int id;
    sqlite3_stmt *stmt;
    int opcion;
    char buffer[10];

    printf("\n---- MODO: MODIFICAR VUELO ------\n");
    printf("Introduce el ID del vuelo a editar: ");
    scanf("%d", &id);
    limpiarBuffer();

    // Primero verificamos si el vuelo existe
    const char *sql_check = "SELECT COUNT(*) FROM Vuelos WHERE id_vuelo = ?;";
    int existe = 0;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            existe = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (!existe) {
        printf(">> [INFO] El ID de vuelo %d no existe.\n", id);
        sqlite3_close(db);
        return;
    }

    // Menú interno de modificaciones
    printf("\n¿Qué campo deseas modificar?\n");
    printf("1. Fecha de Salida (YYYY-MM-DD HH:MM:SS)\n");
    printf("2. Ruta / Trayecto (Ej: Madrid-Barcelona)\n");
    printf("3. ID del Avión Asignado\n");
    printf("Seleccione una opción: ");
    
    fgets(buffer, sizeof(buffer), stdin);
    if (sscanf(buffer, "%d", &opcion) != 1) {
        printf("Error: Opción no válida.\n");
        sqlite3_close(db);
        return;
    }

    if (opcion == 1) {
        char nueva_fecha[25];
        printf("Nueva Fecha de Salida (YYYY-MM-DD HH:MM:SS): ");
        fgets(nueva_fecha, sizeof(nueva_fecha), stdin);
        nueva_fecha[strcspn(nueva_fecha, "\n")] = 0;

        const char *sql = "UPDATE Vuelos SET fecha_salida = ? WHERE id_vuelo = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, nueva_fecha, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, id);
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf(">> [EXITO] Fecha de salida actualizada.\n");
                
                // USAMOS TU LOG: Creamos un mensaje informativo
                char msg_log[100];
                sprintf(msg_log, "ADMIN modificó la fecha del vuelo ID %d a: %s", id, nueva_fecha);
                registrar_log(msg_log);
            }
        }
        sqlite3_finalize(stmt);

    } else if (opcion == 2) {
        char nueva_ruta[100];
        printf("Nueva Ruta (Ej: Origen-Destino): ");
        fgets(nueva_ruta, sizeof(nueva_ruta), stdin);
        nueva_ruta[strcspn(nueva_ruta, "\n")] = 0;

        const char *sql = "UPDATE Vuelos SET ruta = ? WHERE id_vuelo = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, nueva_ruta, -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, id);
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf(">> [EXITO] Ruta del vuelo actualizada.\n");
                
                // USAMOS TU LOG
                char msg_log[100];
                sprintf(msg_log, "ADMIN modificó la ruta del vuelo ID %d a: %s", id, nueva_ruta);
                registrar_log(msg_log);
            }
        }
        sqlite3_finalize(stmt);

    } else if (opcion == 3) {
        int nuevo_avion;
        printf("Introduce el ID del nuevo Avión: ");
        scanf("%d", &nuevo_avion);
        limpiarBuffer();

        const char *sql = "UPDATE Vuelos SET id_avion = ? WHERE id_vuelo = ?;";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_int(stmt, 1, nuevo_avion);
            sqlite3_bind_int(stmt, 2, id);
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf(">> [EXITO] Avión asignado al vuelo actualizado.\n");
                
                // USAMOS TU LOG
                char msg_log[100];
                sprintf(msg_log, "ADMIN modificó el avión del vuelo ID %d al ID de avión: %d", id, nuevo_avion);
                registrar_log(msg_log);
            }
        }
        sqlite3_finalize(stmt);

    } else {
        printf("Opción no reconocida.\n");
    }

    sqlite3_close(db);
}

void gestionarVuelos(int opcion) {
    switch (opcion) {
        case 3: crearVuelo(); break;
        case 4: eliminarVuelo(); break;
        case 5: modificarVuelo(); break;
        default: printf("Opcion no reconocida.\n");
    }
}