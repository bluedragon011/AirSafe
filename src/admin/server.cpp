#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../shared/protocolo.h"
#include "../shared/sqlite3.h"


using namespace std;


extern "C" {
    #include "../shared/auth.h"
    #include "../shared/logs.h"
}


string RUTA_DB = "data/airsafe.db";
int PUERTO_SERVER = 8080;


void cargarConfiguracion() {
    ifstream f("data/config.dat");
    if (f.is_open()) {
        string linea;
        while (getline(f, linea)) {

            if (linea.rfind("db_path=", 0) == 0) {
                RUTA_DB = linea.substr(8);
                if (!RUTA_DB.empty() && RUTA_DB.back() == '\r') RUTA_DB.pop_back();
            }

            if (linea.rfind("server_port=", 0) == 0) {
                string puerto_str = linea.substr(12);
                if (!puerto_str.empty() && puerto_str.back() == '\r') puerto_str.pop_back();
                try {
                    PUERTO_SERVER = stoi(puerto_str);
                } catch (...) {
                    PUERTO_SERVER = 8080;
                }
            }
        }
        f.close();
        cout << ">> [CONFIG] Cargada con exito. BD: " << RUTA_DB << " | Puerto: " << PUERTO_SERVER << "\n";
    } else {
        cout << ">> [CONFIG] No se encontro config.dat. Usando valores por defecto.\n";
    }
}

// Abre la base de datos en la ruta configurada
sqlite3* abrirDB() {
    sqlite3 *db;
    if (sqlite3_open(RUTA_DB.c_str(), &db) != SQLITE_OK) {
        return NULL;
    }
    return db;
}

// Trocea "a|b|c" en sus campos
vector<string> trocear(string texto, char sep) {
    vector<string> partes;
    string actual = "";
    for (size_t i = 0; i < texto.length(); i++) {
        if (texto[i] == sep) { partes.push_back(actual); actual = ""; }
        else actual += texto[i];
    }
    partes.push_back(actual);
    return partes;
}

// Devuelve "id|nombre" del usuario por su email (o "" si no existe)
string datosLoginUsuario(string email) {
    sqlite3 *db = abrirDB();
    if (!db) return "";
    sqlite3_stmt *stmt;
    string res = "";
    const char *sql = "SELECT id_usuario, IFNULL(nombre, '') FROM Usuarios WHERE email = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            res = to_string(sqlite3_column_int(stmt, 0));
            res += "|";
            res += (const char*)sqlite3_column_text(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return res;
}

// Lista todos los vuelos: "id|salida|llegada|id_avion|ruta" por linea
string listarVuelos() {
    sqlite3 *db = abrirDB();
    if (!db) return RES_ERROR;
    sqlite3_stmt *stmt;
    string resp = "";
    const char *sql = "SELECT id_vuelo, IFNULL(fecha_salida,''), IFNULL(fecha_llegada,''), "
                      "IFNULL(id_avion,0), IFNULL(ruta,'') FROM Vuelos;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            resp += to_string(sqlite3_column_int(stmt, 0)); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 1); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 2); resp += "|";
            resp += to_string(sqlite3_column_int(stmt, 3)); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 4); resp += "\n";
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (resp.empty()) return RES_ERROR;
    return resp;
}

// Lista los asientos libres de un vuelo: "id_asiento|num_asiento" por linea
string listarAsientos(string idVuelo) {
    sqlite3 *db = abrirDB();
    if (!db) return RES_ERROR;
    sqlite3_stmt *stmt;
    string resp = "";
    // asientos del avion de ese vuelo que no esten ya reservados en ese vuelo
    const char *sql = "SELECT a.id_asiento, a.num_asiento FROM Asiento a "
                      "WHERE a.id_avion = (SELECT id_avion FROM Vuelos WHERE id_vuelo = ?) "
                      "AND a.id_asiento NOT IN (SELECT id_asiento FROM Reserva WHERE id_vuelo = ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, atoi(idVuelo.c_str()));
        sqlite3_bind_int(stmt, 2, atoi(idVuelo.c_str()));
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            resp += to_string(sqlite3_column_int(stmt, 0)); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 1); resp += "\n";
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (resp.empty()) return RES_ERROR;
    return resp;
}

// Inserta una reserva. Devuelve COMPRA_OK / COMPRA_COMPLETO (asiento ocupado) / ERROR
string comprarVuelo(string idUsr, string idVuelo, string idAsiento, string precio) {
    sqlite3 *db = abrirDB();
    if (!db) return RES_ERROR;
    sqlite3_stmt *stmt;

    // 1) comprobar si el asiento ya esta cogido en ese vuelo
    int ocupado = 0;
    const char *sqlCheck = "SELECT COUNT(*) FROM Reserva WHERE id_vuelo = ? AND id_asiento = ?;";
    if (sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, atoi(idVuelo.c_str()));
        sqlite3_bind_int(stmt, 2, atoi(idAsiento.c_str()));
        if (sqlite3_step(stmt) == SQLITE_ROW) ocupado = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    if (ocupado > 0) { sqlite3_close(db); return RES_COMPRA_LLENO; }

    // 2) insertar la reserva (fecha de hoy)
    string resultado = RES_ERROR;
    const char *sqlIns = "INSERT INTO Reserva (id_usuario, fecha_reserva, precio, id_vuelo, id_asiento) "
                         "VALUES (?, date('now'), ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sqlIns, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, atoi(idUsr.c_str()));
        sqlite3_bind_double(stmt, 2, atof(precio.c_str()));
        sqlite3_bind_int(stmt, 3, atoi(idVuelo.c_str()));
        sqlite3_bind_int(stmt, 4, atoi(idAsiento.c_str()));
        if (sqlite3_step(stmt) == SQLITE_DONE) resultado = RES_COMPRA_OK;
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return resultado;
}

// Historial de un usuario: "id_reserva|fecha|ruta|asiento|precio" por linea
string consultarHistorial(string idUsr) {
    sqlite3 *db = abrirDB();
    if (!db) return RES_ERROR;
    sqlite3_stmt *stmt;
    string resp = "";
    const char *sql = "SELECT r.id_reserva, IFNULL(r.fecha_reserva,''), IFNULL(v.ruta,''), "
                      "IFNULL(a.num_asiento,''), IFNULL(r.precio,0) "
                      "FROM Reserva r "
                      "JOIN Vuelos v ON r.id_vuelo = v.id_vuelo "
                      "JOIN Asiento a ON r.id_asiento = a.id_asiento "
                      "WHERE r.id_usuario = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, atoi(idUsr.c_str()));
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            resp += to_string(sqlite3_column_int(stmt, 0)); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 1); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 2); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 3); resp += "|";
            resp += (const char*)sqlite3_column_text(stmt, 4); resp += "\n";
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (resp.empty()) return RES_ERROR;
    return resp;
}

// Función encargada de procesar los datos de red (Protocolo) e interactuar con la BD
void atenderCliente(SOCKET socket_cliente) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_recibidos = recv(socket_cliente, buffer, BUFFER_SIZE - 1, 0);

    if (bytes_recibidos > 0) {
        string mensaje(buffer);
        cout << "[SERVIDOR] Peticion entrante: " << mensaje << "\n";

        vector<string> campos = trocear(mensaje, '|');
        string comando = campos[0];
        string respuesta = RES_ERROR;

        if (comando == REQ_LOGIN && campos.size() >= 3) {
            string email = campos[1];
            string password = campos[2];

            char c_email[100];
            char c_password[64];
            strncpy(c_email, email.c_str(), sizeof(c_email));
            c_email[sizeof(c_email) - 1] = '\0';
            strncpy(c_password, password.c_str(), sizeof(c_password));
            c_password[sizeof(c_password) - 1] = '\0';

            if (validarCredenciales(c_email, c_password)) {
                char tipo[15];
                obtenerTipoUsuario(c_email, tipo);

                // Restriccion: por sockets (App Cliente) solo entran pasajeros ("CLIENTE")
                if (strcmp(tipo, "CLIENTE") == 0) {
                    respuesta = string(RES_LOGIN_OK) + "|" + datosLoginUsuario(email);
                    registrar_log(("Login RED EXITOSO - Cuenta Cliente: " + email).c_str());
                } else {
                    respuesta = RES_LOGIN_DENEGADO;
                    registrar_log(("Login RED RECHAZADO (Es Admin): " + email).c_str());
                }
            } else {
                respuesta = RES_LOGIN_DENEGADO;
                registrar_log(("Login RED ERRONEAS - Credenciales invalidas para: " + email).c_str());
            }
        }
        else if (comando == REQ_LISTAR_VUELOS) {
            respuesta = listarVuelos();
        }
        else if (comando == REQ_LISTAR_ASIENTOS && campos.size() >= 2) {
            respuesta = listarAsientos(campos[1]);
        }
        else if (comando == REQ_COMPRAR_VUELO && campos.size() >= 5) {
            respuesta = comprarVuelo(campos[1], campos[2], campos[3], campos[4]);
            registrar_log(("Reserva usuario " + campos[1] + " vuelo " + campos[2] + " asiento " + campos[3] + " -> " + respuesta).c_str());
        }
        else if (comando == REQ_CONSULTAR && campos.size() >= 2) {
            respuesta = consultarHistorial(campos[1]);
        }

        send(socket_cliente, respuesta.c_str(), respuesta.length(), 0);
    }

    closesocket(socket_cliente);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << ">> [ERROR] Fallo al inicializar Winsock.\n";
        return -1;
    }

    cargarConfiguracion();

    SOCKET server_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int addrlen = sizeof(direccion);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cerr << ">> [ERROR] No se pudo crear el socket maestro.\n";
        WSACleanup();
        return -1;
    }

    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PUERTO_SERVER);

    // Enlazar al puerto leído de la configuración
    if (bind(server_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        cerr << ">> [ERROR] Fallo el bind. El puerto " << PUERTO_SERVER << " podria estar en uso.\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        cerr << ">> [ERROR] Fallo la escucha de red.\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    cout << ">> [SERVIDOR AIRSAFE ONLINE] Escuchando activamente en el puerto " << PUERTO_SERVER << "...\n";

    // Bucle continuo para aceptar las conexiones de los clientes
    while (true) {
        if ((nuevo_socket = accept(server_fd, (struct sockaddr*)&direccion, &addrlen)) == INVALID_SOCKET) {
            cerr << ">> [ERROR] Conexion entrante rechazada por el sistema operativo.\n";
            break;
        }
        atenderCliente(nuevo_socket);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}
