#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../shared/sqlite3.h" //Libreria para la bd
#include "../shared/auth.h"
#include "../shared/models.h" //Los structs creados a partir de la bd




// Función auxiliar para manejar errores de SQLite
void check_db_error(int rc, sqlite3 *db) {
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error de SQL: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
}

//Funcion de busqueda de usuario (se pide pasaporte solamente, en el futuro posible implementacion pero solo con nombre/email)
void buscarUsuario() {
    sqlite3 *db;
    sqlite3_stmt *res;
    char pasaporte[20];

    printf("\n--- BÚSQUEDA DE USUARIO ---\n");
    printf("Introduce el pasaporte: ");
    fgets(pasaporte, sizeof(pasaporte), stdin);
    pasaporte[strcspn(pasaporte, "\n")] = 0;

    int rc = sqlite3_open("data/airsafe.db", &db);
    check_db_error(rc, db);

    const char *sql = "SELECT nombre, email, telefono FROM Usuarios WHERE pasaporte = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    sqlite3_bind_text(res, 1, pasaporte, -1, SQLITE_STATIC);

    if (sqlite3_step(res) == SQLITE_ROW) {
        printf("\n[USUARIO ENCONTRADO]\n");
        printf("Nombre: %s\n", sqlite3_column_text(res, 0));
        printf("Email:  %s\n", sqlite3_column_text(res, 1));
        printf("Telf:   %s\n", sqlite3_column_text(res, 2));
    } else {
        printf("No existe ningún usuario con pasaporte: %s\n", pasaporte);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}
//Funcion de búsqueda de vuelos. Pedimos el ID
void buscarVuelos() {
    sqlite3 *db;
    sqlite3_stmt *res;
    int id_buscado;
    char buffer[10];

    printf("\n--- BÚSQUEDA DE VUELO REAL ---\n");
    printf("Introduce el ID del vuelo: ");
    fgets(buffer, sizeof(buffer), stdin);
    if (sscanf(buffer, "%d", &id_buscado) != 1) return;

    if (sqlite3_open("data/airsafe.db", &db) != SQLITE_OK) {
        printf("Error al abrir la base de datos\n");
        return;
    }
    
    // SQL corregido (añadida coma y ordenada la ruta)
    const char *sql = "SELECT V.id_vuelo, V.fecha_salida, IFNULL(A.matricula, 'SIN ASIGNAR'), V.ruta "
                      "FROM Vuelos V LEFT JOIN Aviones A ON V.id_avion = A.id_avion "
                      "WHERE V.id_vuelo = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, 0) != SQLITE_OK) {
        printf("Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_int(res, 1, id_buscado);

    if (sqlite3_step(res) == SQLITE_ROW) {
        printf("\n[DATOS DEL VUELO]\n");
        printf("ID Vuelo: %d\n", sqlite3_column_int(res, 0));
        printf("Fecha:    %s\n", sqlite3_column_text(res, 1));
        printf("Avion:    %s\n", sqlite3_column_text(res, 2));

        // Manejo de la ruta con strtok
        const char *ruta_raw = (const char*)sqlite3_column_text(res, 3);
        if (ruta_raw != NULL) {
            char ruta_copy[100];
            strcpy(ruta_copy, ruta_raw); // Copiamos para no romper el puntero original

            char *origen = strtok(ruta_copy, "-");
            char *destino = strtok(NULL, "-");

            if (origen && destino) {
                printf("Origen:   %s\n", origen);
                printf("Destino:  %s\n", destino);
            } else {
                printf("Ruta:     %s\n", ruta_raw);
            }
        } else {
            printf("Ruta:     No especificada\n");
        }
    } else {
        printf("Vuelo %d no encontrado.\n", id_buscado);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}

// DEFINICION DE FUNCIONES USADAS DE PROGRAMAS EXTERNOS
void gestionarVuelos(int opcion);
void limpiarbuffer();
void crearVuelo();
void eliminarVuelo();
void modificarVuelo();

int panelAcceso();

// ----------------- MAIN -------------------
int main(void) {
    int running = 1; 
    char buffer[10];
    int seleccion;

    panelAcceso();

    while (running) {
        printf("\n------------- MENÚ DE ADMINISTRADOR -------------\n");
        printf("1. Buscar usuario\n");
        printf("2. Buscar vuelo\n");
        printf("3. Crear vuelo\n");
        printf("4. Eliminar vuelo\n");
        printf("5. Modificar vuelo\n");
        printf("6. Cerrar sesión\n");
        printf("Seleccione una opción: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &seleccion) != 1) {
                printf("Error: Por favor, introduce un número.\n");
                continue;
            }

            switch (seleccion) {
                case 1: buscarUsuario(); break;
                case 2: buscarVuelos(); break;
                case 3: case 4: case 5: gestionarVuelos(seleccion); break;
                case 6:
                    printf("Cerrando sesión...\n");
                    running = 0; 
                    break;
                default:
                    fprintf(stderr, "Opción %d no válida.\n", seleccion);
            }
        }
    }
    return 0;
}

