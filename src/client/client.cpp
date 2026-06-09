#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <winsock2.h>
#include "../shared/protocolo.h"

using namespace std;

// Los comandos del protocolo (incluidos LISTAR_VUELOS y LISTAR_ASIENTOS)
// estan definidos en protocolo.h. Formato de las respuestas del servidor:
//   LISTAR_VUELOS                              -> "id|salida|llegada|id_avion|ruta" por linea
//   LISTAR_ASIENTOS|id_vuelo                   -> "id_asiento|num_asiento" por linea
//   COMPRAR_VUELO|id_usr|id_vuelo|asiento|precio -> COMPRA_OK / COMPRA_COMPLETO / ERROR
//   CONSULTAR_HISTORIAL|id_usr                 -> "id_reserva|fecha|ruta|asiento|precio" por linea
//   LOGIN|email|pass                           -> "LOGIN_OK|id_usuario|nombre" / LOGIN_DENEGADO

const string PRECIO_FIJO = "49.99";

// clases del cliente

class Vuelo {
private:
    int id_vuelo;
    string fecha_salida;
    string fecha_llegada;
    int id_avion;
    string ruta;

public:
    Vuelo(int id_vuelo, string fecha_salida, string fecha_llegada, int id_avion, string ruta) {
        this->id_vuelo = id_vuelo;
        this->fecha_salida = fecha_salida;
        this->fecha_llegada = fecha_llegada;
        this->id_avion = id_avion;
        this->ruta = ruta;
    }

    void mostrar() const {
        cout << "  [" << id_vuelo << "] " << ruta
             << " | Salida: " << fecha_salida
             << " | Llegada: " << fecha_llegada << "\n";
    }
};

class Usuario {
private:
    int id_usuario;
    string nombre;
    string email;
    bool autenticado;

public:
    Usuario() {
        id_usuario = 0;
        autenticado = false;
    }

    int getId() const { return id_usuario; }
    string getNombre() const { return nombre; }
    string getEmail() const { return email; }

    void setId(int id) { this->id_usuario = id; }
    void setNombre(string nombre) { this->nombre = nombre; }
    void setEmail(string email) { this->email = email; }
    void setAutenticado(bool autenticado) { this->autenticado = autenticado; }

    bool logueado() const {
        return autenticado;
    }
};

//  Configuracion red

string IP_SERVER = "127.0.0.1";
int PUERTO_SERVER = 8080;

void cargarConfiguracion() {
    ifstream f("data/config.dat");
    if (f.is_open()) {
        string linea;
        while (getline(f, linea)) {
            if (linea.rfind("server_ip=", 0) == 0) {
                IP_SERVER = linea.substr(10);
                if (!IP_SERVER.empty() && IP_SERVER.back() == '\r') IP_SERVER.pop_back();
            }
            if (linea.rfind("server_port=", 0) == 0) {
                string p = linea.substr(12);
                if (!p.empty() && p.back() == '\r') p.pop_back();
                try { PUERTO_SERVER = stoi(p); } catch (...) { PUERTO_SERVER = 8080; }
            }
        }
        f.close();
        cout << ">> [CONFIG] Conectando a Servidor -> " << IP_SERVER << ":" << PUERTO_SERVER << "\n";
    } else {
        cout << ">> [CONFIG] No se encontro config.dat. Usando valores por defecto (localhost:8080).\n";
    }
}

string enviarPeticion(string peticion) {
    SOCKET sock;
    struct sockaddr_in servidor;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        return "ERROR_SOCKET";

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PUERTO_SERVER);
    servidor.sin_addr.s_addr = inet_addr(IP_SERVER.c_str());

    if (servidor.sin_addr.s_addr == INADDR_NONE) { closesocket(sock); return "ERROR_IP_INVALIDA"; }
    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) < 0) { closesocket(sock); return "ERROR_CONEXION"; }

    send(sock, peticion.c_str(), peticion.length(), 0);
    int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    string respuesta = (bytes > 0) ? string(buffer) : "ERROR_NO_RESPUESTA";

    closesocket(sock);
    return respuesta;
}

// parte un texto por un caracter ("a|b|c" -> ["a","b","c"])
vector<string> separar(string texto, char sep) {
    vector<string> partes;
    string actual = "";
    for (int i = 0; i < (int)texto.length(); i++) {
        if (texto[i] == sep) {
            partes.push_back(actual);
            actual = "";
        } else {
            actual += texto[i];
        }
    }
    partes.push_back(actual);
    return partes;
}

// Cache de vuelos para no volver a pedirsela al servidor.
map<string, vector<Vuelo>> cacheVuelos;

//menu

Usuario hacerLogin() {
    Usuario u;
    char email[100], password[64];

    cout << "\n==================================================\n";
    cout << "          AIRSAFE APP - LOGIN DE PASAJEROS        \n";
    cout << "==================================================\n";
    cout << "Introduce tu Email: ";
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;
    cout << "Introduce tu Contrasena: ";
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    string peticion = string(REQ_LOGIN) + "|" + string(email) + "|" + string(password);
    cout << ">> Enviando credenciales al servidor seguro...\n";
    string respuesta = enviarPeticion(peticion);

    // esperamos "LOGIN_OK|id_usuario|nombre"
    vector<string> campos = separar(respuesta, '|');
    if (campos[0] == RES_LOGIN_OK) {
        cout << "\n>> [ACCESO CONCEDIDO] Bienvenido a AirSafe!\n";
        u.setAutenticado(true);
        u.setEmail(string(email));
        if (campos.size() >= 2) {
            try { u.setId(stoi(campos[1])); } catch (...) {}
        }
        if (campos.size() >= 3) u.setNombre(campos[2]);
        else u.setNombre("Pasajero");
    } else if (respuesta == RES_LOGIN_DENEGADO) {
        cout << "\n>> [ERROR] Credenciales incorrectas o cuenta no es de tipo CLIENTE.\n";
    } else {
        cout << "\n>> [ERROR DE RED] No se pudo comunicar con el servidor (" << respuesta << ").\n";
    }
    return u;
}

void verVuelos() {
    string clave = "TODOS";

    cout << "\n--- VUELOS DISPONIBLES ---\n";

    // si ya lo pedimos antes, lo sacamos de la cache
    if (cacheVuelos.find(clave) != cacheVuelos.end()) {
        cout << ">> [CACHE] Cargando vuelos guardados (sin pedirlos al servidor).\n";
        for (int i = 0; i < (int)cacheVuelos[clave].size(); i++)
            cacheVuelos[clave][i].mostrar();
        return;
    }

    cout << ">> Pidiendo la lista de vuelos al servidor...\n";
    string respuesta = enviarPeticion(REQ_LISTAR_VUELOS);

    if (respuesta.rfind("ERROR", 0) == 0 || respuesta == RES_ERROR) {
        cout << ">> No se pudo obtener la lista de vuelos del servidor.\n";
        return;
    }

    // pasamos la respuesta a objetos Vuelo
    vector<Vuelo> vuelos;
    vector<string> lineas = separar(respuesta, '\n');
    for (int i = 0; i < (int)lineas.size(); i++) {
        if (lineas[i].empty()) continue;
        vector<string> c = separar(lineas[i], '|');
        if (c.size() >= 5) {
            int id, avion;
            try { id = stoi(c[0]); avion = stoi(c[3]); }
            catch (...) { continue; }
            Vuelo v(id, c[1], c[2], avion, c[4]);
            vuelos.push_back(v);
        }
    }

    if (vuelos.empty()) {
        cout << ">> No hay vuelos disponibles ahora mismo.\n";
        return;
    }

    for (int i = 0; i < (int)vuelos.size(); i++)
        vuelos[i].mostrar();

    cacheVuelos[clave] = vuelos; // lo guardamos para la proxima
}

void reservarVuelo(Usuario& u) {
    int id_vuelo, id_asiento;
    char buffer[20];

    cout << "\n--- RESERVAR VUELO ---\n";
    verVuelos();

    cout << "\nIntroduce el ID del vuelo: ";
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    if (sscanf(buffer, "%d", &id_vuelo) != 1) {
        cout << "ID no valido.\n";
        return;
    }

    // pedimos al servidor los asientos libres de ese vuelo
    cout << ">> Consultando asientos libres...\n";
    string resp = enviarPeticion(string(REQ_LISTAR_ASIENTOS) + "|" + to_string(id_vuelo));
    if (resp.rfind("ERROR", 0) == 0 || resp == RES_ERROR) {
        cout << ">> No se pudieron obtener los asientos del servidor.\n";
        return;
    }

    vector<string> lineas = separar(resp, '\n');
    int libres = 0;
    cout << "Asientos libres:\n";
    for (int i = 0; i < (int)lineas.size(); i++) {
        if (lineas[i].empty()) continue;
        vector<string> c = separar(lineas[i], '|');
        if (c.size() >= 2) {
            cout << "  ID " << c[0] << " -> Asiento " << c[1] << "\n";
            libres++;
        }
    }
    if (libres == 0) {
        cout << ">> Este vuelo no tiene asientos libres.\n";
        return;
    }

    cout << "Introduce el ID del asiento que quieres: ";
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    if (sscanf(buffer, "%d", &id_asiento) != 1) {
        cout << "ID no valido.\n";
        return;
    }

    string peticion = string(REQ_COMPRAR_VUELO) + "|" + to_string(u.getId()) + "|"
                    + to_string(id_vuelo) + "|" + to_string(id_asiento) + "|" + PRECIO_FIJO;
    cout << ">> Enviando reserva al servidor...\n";
    string r = enviarPeticion(peticion);

    if (r == RES_COMPRA_OK) {
        cout << ">> [EXITO] Reserva confirmada (vuelo " << id_vuelo
             << ", asiento " << id_asiento << ", " << PRECIO_FIJO << " EUR).\n";
    } else if (r == RES_COMPRA_LLENO) {
        cout << ">> [INFO] Ese asiento ya esta ocupado.\n";
    } else {
        cout << ">> No se pudo completar la reserva (" << r << ").\n";
    }
}

void verHistorial(Usuario& u) {
    cout << "\n--- MIS RESERVAS ---\n";

    string respuesta = enviarPeticion(string(REQ_CONSULTAR) + "|" + to_string(u.getId()));
    if (respuesta.rfind("ERROR", 0) == 0 || respuesta == RES_ERROR) {
        cout << ">> No se pudo obtener tu historial del servidor.\n";
        return;
    }

    vector<string> lineas = separar(respuesta, '\n');
    int n = 0;
    for (int i = 0; i < (int)lineas.size(); i++) {
        if (lineas[i].empty()) continue;
        vector<string> c = separar(lineas[i], '|');
        if (c.size() >= 5) {
            cout << "  Reserva " << c[0] << " | " << c[2]
                 << " | " << c[1] << " | Asiento " << c[3]
                 << " | " << c[4] << " EUR\n";
            n++;
        }
    }
    if (n == 0) cout << ">> No tienes reservas registradas.\n";
}

void menuPasajero(Usuario& u) {
    int running = 1;
    int seleccion;
    char buffer[10];

    while (running) {
        cout << "\n------------- MENU PASAJERO (" << u.getNombre() << ") -------------\n";
        cout << "1. Ver vuelos disponibles\n";
        cout << "2. Reservar un vuelo\n";
        cout << "3. Mis reservas\n";
        cout << "0. Cerrar sesion\n";
        cout << "Seleccione una opcion: ";

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &seleccion) != 1) {
                cout << "Error: introduce un numero.\n";
                continue;
            }
            switch (seleccion) {
                case 1: verVuelos(); break;
                case 2: reservarVuelo(u); break;
                case 3: verHistorial(u); break;
                case 0:
                    cout << "Cerrando sesion...\n";
                    running = 0;
                    break;
                default:
                    cout << "Opcion " << seleccion << " no valida.\n";
            }
        }
    }
}

// ----------------- MAIN -------------------
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << ">> [ERROR] Fallo al inicializar la red del cliente.\n";
        return -1;
    }

    cargarConfiguracion();

    int running = 1;
    int seleccion;
    char buffer[10];

    while (running) {
        cout << "\n=== MENU PRINCIPAL PASAJEROS ===\n";
        cout << "1. Iniciar Sesion (Login)\n";
        cout << "0. Salir de la Aplicacion\n";
        cout << "Seleccione una opcion: ";

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &seleccion) != 1) {
                cout << "Error: introduce un numero.\n";
                continue;
            }
            switch (seleccion) {
                case 1: {
                    Usuario u = hacerLogin();
                    if (u.logueado()) menuPasajero(u);
                    break;
                }
                case 0:
                    cout << "Cerrando la aplicacion cliente...\n";
                    running = 0;
                    break;
                default:
                    cout << "Opcion " << seleccion << " no valida.\n";
            }
        }
    }

    WSACleanup();
    return 0;
}
