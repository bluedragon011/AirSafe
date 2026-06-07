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
    #include "pasajeros_db.h"
}


string RUTA_DB = "data/airsafe.db";
extern "C" char RUTA_DB_C[150] = "data/airsafe.db";
int PUERTO_SERVER = 8080; 


void cargarConfiguracion() {
    ifstream f("data/config.dat");
    if (f.is_open()) {
        string linea;
        while (getline(f, linea)) {
            
            if (linea.rfind("db_path=", 0) == 0) {
                RUTA_DB = linea.substr(8);
                if (!RUTA_DB.empty() && RUTA_DB.back() == '\r') RUTA_DB.pop_back();
                strncpy(RUTA_DB_C, RUTA_DB.c_str(), sizeof(RUTA_DB_C));
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

//Función encargada de procesar los datos de red y interactuar con auth.c
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
                    char c_email[100], c_password[64];
                    strncpy(c_email, email.c_str(), sizeof(c_email));
                    strncpy(c_password, password.c_str(), sizeof(c_password));

                    if (validarCredenciales(c_email, c_password)) {
                        char tipo[15]; obtenerTipoUsuario(c_email, tipo);
                        if (strcmp(tipo, "CLIENTE") == 0) {
                            int id_usr = obtenerIdUsuario(c_email);
                            string respuesta = string(RES_LOGIN_OK) + "|" + to_string(id_usr) + "|Pasajero";
                            send(socket_cliente, respuesta.c_str(), respuesta.length(), 0);
                            registrar_log(("Login RED EXITOSO: " + email).c_str());
                        } else {
                            send(socket_cliente, RES_LOGIN_DENEGADO, strlen(RES_LOGIN_DENEGADO), 0);
                        }
                    } else {
                        send(socket_cliente, RES_LOGIN_DENEGADO, strlen(RES_LOGIN_DENEGADO), 0);
                    }
                }
            }
            else if (comando == "LISTAR_VUELOS") {
                char buffer_respuesta[BUFFER_SIZE];
                obtenerVuelosRed(buffer_respuesta);
                send(socket_cliente, buffer_respuesta, strlen(buffer_respuesta), 0);
            }
            else if (comando == "LISTAR_ASIENTOS") {
                int id_vuelo = stoi(mensaje.substr(pos1 + 1));
                char buffer_respuesta[BUFFER_SIZE];
                obtenerAsientosLibresRed(id_vuelo, buffer_respuesta);
                send(socket_cliente, buffer_respuesta, strlen(buffer_respuesta), 0);
            }
            else if (comando == "COMPRAR_VUELO") {
                //Peticion: COMPRAR_VUELO|id_usr|id_vuelo|id_asiento|precio
                int pos2 = mensaje.find('|', pos1 + 1);
                int pos3 = mensaje.find('|', pos2 + 1);
                int id_usr = stoi(mensaje.substr(pos1 + 1, pos2 - pos1 - 1));
                int id_vuelo = stoi(mensaje.substr(pos2 + 1, pos3 - pos2 - 1));
                
                int res = comprarVueloDB(id_usr, id_vuelo);
                if (res == 1) {
                    send(socket_cliente, "COMPRA_OK", 9, 0);
                    registrar_log("Compra de billete realizada por red.");
                } else if (res == -1) {
                    send(socket_cliente, "COMPRA_COMPLETO", 15, 0);
                } else {
                    send(socket_cliente, "ERROR", 5, 0);
                }
            }
            else if (comando == "CONSULTAR_HISTORIAL") {
                int id_usr = stoi(mensaje.substr(pos1 + 1));
                char buffer_respuesta[BUFFER_SIZE];
                obtenerHistorialRed(id_usr, buffer_respuesta);
                send(socket_cliente, buffer_respuesta, strlen(buffer_respuesta), 0);
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

    //Enlazar al puerto leído de la configuración
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

    //Bucle continuo para aceptar las conexiones de los clientes
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
