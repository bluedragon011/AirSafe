#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include "../shared/protocolo.h"

using namespace std;

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

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        return "ERROR_SOCKET";
    }

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PUERTO_SERVER);
    
    // Cambiado por inet_addr para evitar incompatibilidades en Windows
    servidor.sin_addr.s_addr = inet_addr(IP_SERVER.c_str());
    if (servidor.sin_addr.s_addr == INADDR_NONE) {
        closesocket(sock);
        return "ERROR_IP_INVALIDA";
    }

    if (connect(sock, (struct sockaddr*)&servidor, sizeof(servidor)) < 0) {
        closesocket(sock);
        return "ERROR_CONEXION";
    }

    send(sock, peticion.c_str(), peticion.length(), 0);

    int bytes_recibidos = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    string respuesta = (bytes_recibidos > 0) ? string(buffer) : "ERROR_NO_RESPUESTA";

    closesocket(sock);
    return respuesta;
}

void menuLogin() {
    string email, password;
    
    cout << "\n==================================================\n";
    cout << "          AIRSAFE APP - LOGIN DE PASAJEROS        \n";
    cout << "==================================================\n";
    cout << "Introduce tu Email: ";
    cin >> email;
    cout << "Introduce tu Contrasena: ";
    cin >> password;

    string peticion = string(REQ_LOGIN) + "|" + email + "|" + password;
    
    cout << ">> Enviando credenciales al servidor seguro...\n";
    string respuesta = enviarPeticion(peticion);

    if (respuesta == RES_LOGIN_OK) {
        cout << "\n>> [ACCESO CONCEDIDO] ¡Bienvenido a AirSafe! Has iniciado sesion correctamente.\n";
    } else if (respuesta == RES_LOGIN_DENEGADO) {
        cout << "\n>> [ERROR] Credenciales incorrectas o el acceso no es de tipo CLIENTE.\n";
    } else {
        cout << "\n>> [ERROR DE RED] No se pudo comunicar con el servidor (" << respuesta << ").\n";
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << ">> [ERROR] Fallo al inicializar la red del cliente.\n";
        return -1;
    }

    cargarConfiguracion();
    
    int opcion = -1;
    while (opcion != 0) {
        cout << "\n=== MENU PRINCIPAL PASAJEROS ===\n";
        cout << "1. Iniciar Sesion (Login por Sockets)\n";
        cout << "0. Salir de la Aplicacion\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion;

        if (opcion == 1) {
            menuLogin();
        } else if (opcion != 0) {
            cout << "Opcion no valida.\n";
        }
    }

    cout << "Cerrando la aplicacion cliente...\n";
    WSACleanup();
    return 0;
}