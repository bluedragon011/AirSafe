#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../shared/sqlite3.h"
#include "../shared/auth.h"
#include "../shared/models.h"

//Función interna para no repetir código de apertura
static sqlite3* conectarDB() {
    sqlite3 *db;
    if (sqlite3_open("data/airsafe.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Error conexión: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

void mostrarPantallaInicio(void) {
    printf("\n==================================================\n");
    printf("            AIRSAFE - ACCESO AL SISTEMA           \n");
    printf("==================================================\n\n");
    printf("1. Iniciar Sesion (Login)\n");
    printf("2. Registrarse (Solo Pasajeros)\n");
    printf("0. Salir\n\n");
    printf("Seleccione una opcion: ");
}

int validarCredenciales(char email[], char password_hash[]) {
    sqlite3 *db = conectarDB();
    if (!db) return 0;

    sqlite3_stmt *stmt;
    int login_valido = 0;

    //Verificacion de existencia de email & contraseña
    const char *sql = "SELECT id_usuario FROM Usuarios WHERE email = ? AND password_hash = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password_hash, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            login_valido = 1; // Encontrado
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return login_valido;
}

void obtenerTipoUsuario(char email[], char tipo_out[]) {
    sqlite3 *db = conectarDB();
    if (!db) {
        strcpy(tipo_out, "");
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT tipo_usuario FROM Usuarios WHERE email = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            strcpy(tipo_out, (const char*)sqlite3_column_text(stmt, 0));
        } else {
            strcpy(tipo_out, "");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int iniciarSesion(void) { //IMPORTANTE: Este iniciar sesion es solo para admins. 
    char email[100], password[64], tipo[15];

    printf("\n--- LOGIN ---\n");
    printf("Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;

    printf("Contrasena: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    if (!validarCredenciales(email, password)) {
        printf(">> [ERROR] Email o contrasena incorrectos.\n");
        return 0;
    }

    obtenerTipoUsuario(email, tipo);

    if (strcmp(tipo, "ADMIN") == 0) {
        printf(">> [OK] Bienvenido, Administrador.\n");
        return 1;
    }

    printf(">> [DENEGADO] Este acceso es solo para personal Administrativo.\n");
    return 0;
}

void registrarUsuario(void) {
    sqlite3 *db = conectarDB();
    if (!db) return;

    Usuario n; //Usamos el struct de models.h
    char confirmacion[64];
    sqlite3_stmt *stmt;

    printf("\n--- REGISTRO DE PASAJERO ---\n");
    printf("Nombre completo: ");
    fgets(n.nombre, sizeof(n.nombre), stdin);
    n.nombre[strcspn(n.nombre, "\n")] = 0;

    printf("Pasaporte: ");
    fgets(n.pasaporte, sizeof(n.pasaporte), stdin);
    n.pasaporte[strcspn(n.pasaporte, "\n")] = 0;

    printf("Telefono: ");
    fgets(n.telefono, sizeof(n.telefono), stdin);
    n.telefono[strcspn(n.telefono, "\n")] = 0;

    printf("Email: ");
    fgets(n.email, sizeof(n.email), stdin);
    n.email[strcspn(n.email, "\n")] = 0;

    printf("Establecer Contrasena: ");
    fgets(n.password_hash, sizeof(n.password_hash), stdin);
    n.password_hash[strcspn(n.password_hash, "\n")] = 0;

    printf("Repita Contrasena: ");
    fgets(confirmacion, sizeof(confirmacion), stdin);
    confirmacion[strcspn(confirmacion, "\n")] = 0;

    if (strcmp(n.password_hash, confirmacion) != 0) {
        printf(">> [ERROR] Las contrasenas no coinciden.\n");
        sqlite3_close(db);
        return;
    }

    const char *sql = "INSERT INTO Usuarios (nombre, telefono, pasaporte, email, password_hash, tipo_usuario) "
                      "VALUES (?, ?, ?, ?, ?, 'CLIENTE');";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, n.nombre, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, n.telefono, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, n.pasaporte, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, n.email, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, n.password_hash, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf(">> [EXITO] Registro completado. Ya puedes loguearte como cliente en la App de Usuario.\n");
        } else {
            printf(">> [ERROR] No se pudo registrar (¿Email o Pasaporte duplicado?).\n");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int panelAcceso(void) {

    int running_login = 1;

    char buffer[10];

    int seleccion;
    while (running_login) {

        mostrarPantallaInicio();
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {

            if (sscanf(buffer, "%d", &seleccion) != 1) {
                printf("Error: introduce un numero.\n");
                continue;
            }
            switch (seleccion) {
                case 1:
                    if (iniciarSesion() == 1) {
                        return 1; /*Login exitoso -> pasar al menu de admin*/
                    }
                    break;
                case 2:
                    registrarUsuario();
                    break;
                case 0:
                    printf("Saliendo de AIRSAFE...\n");
                    return 0;
                default:
                    printf("Opcion %d no valida.\n", seleccion);
            }
        }
    }

    return 0;

}
