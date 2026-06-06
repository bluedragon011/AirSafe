#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../shared/protocolo.h" 


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

// Función encargada de procesar los datos de red (Protocolo) e interactuar con auth.c
void atenderCliente(SOCKET socket_cliente) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_recibidos = recv(socket_cliente, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_recibidos > 0) {
        string mensaje(buffer);
        cout << "[SERVIDOR] Peticion entrante: " << mensaje << "\n";

        
        size_t pos1 = mensaje.find('|');
        if (pos1 != string::npos) {
            string comando = mensaje.substr(0, pos1);
            
            
            if (comando == REQ_LOGIN) {
                size_t pos2 = mensaje.find('|', pos1 + 1);
                if (pos2 != string::npos) {
                    string email = mensaje.substr(pos1 + 1, pos2 - (pos1 + 1));
                    string password = mensaje.substr(pos2 + 1);

                    
                    char c_email[100];
                    char c_password[64];
                    strncpy(c_email, email.c_str(), sizeof(c_email));
                    strncpy(c_password, password.c_str(), sizeof(c_password));

                    
                    if (validarCredenciales(c_email, c_password)) {
                        char tipo[15];
                        obtenerTipoUsuario(c_email, tipo);
                        
                        // Restricción: Por sockets (App Cliente) solo entran pasajeros ("CLIENTE")
                        if (strcmp(tipo, "CLIENTE") == 0) {
                            send(socket_cliente, RES_LOGIN_OK, strlen(RES_LOGIN_OK), 0);
                            registrar_log(("Login RED EXITOSO - Cuenta Cliente: " + email).c_str());
                        } else {
                            send(socket_cliente, RES_LOGIN_DENEGADO, strlen(RES_LOGIN_DENEGADO), 0);
                            registrar_log(("Login RED RECHAZADO (Es Admin): " + email).c_str());
                        }
                    } else {
                        send(socket_cliente, RES_LOGIN_DENEGADO, strlen(RES_LOGIN_DENEGADO), 0);
                        registrar_log(("Login RED ERRONEAS - Credenciales invalidas para: " + email).c_str());
                    }
                }
            }
        } else {
            send(socket_cliente, RES_ERROR, strlen(RES_ERROR), 0);
        }
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