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
    #include "pasajeros_db.h"
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
        }
        f.close();
        cout << ">> [SERVIDOR] Configuracion cargada. BD en: " << RUTA_DB << "\n";
    } else {
        cout << ">> [SERVIDOR] No se encontro config.dat, usando ruta por defecto.\n";
    }
}

void atenderCliente(SOCKET socket_cliente) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int bytes_recibidos = recv(socket_cliente, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_recibidos > 0) {
        string mensaje(buffer);
        cout << "[SERVIDOR] Peticion entrante de red: " << mensaje << "\n";

        string respuesta = "OK|Servidor C++ conectado correctamente";
        send(socket_cliente, respuesta.c_str(), respuesta.length(), 0);
        
        registrar_log("Cliente atendido a traves de Sockets C++ Windows");
    }
    
    closesocket(socket_cliente); 
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << ">> [ERROR] No se pudo inicializar Winsock.\n";
        return -1;
    }

    cargarConfiguracion();

    SOCKET server_fd, nuevo_socket;
    struct sockaddr_in direccion;
    int addrlen = sizeof(direccion);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cerr << ">> [ERROR] No se pudo crear el socket del servidor.\n";
        WSACleanup();
        return -1;
    }

    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PUERTO_SERVER);

    if (bind(server_fd, (struct sockaddr*)&direccion, sizeof(direccion)) == SOCKET_ERROR) {
        cerr << ">> [ERROR] Fallo el bind. Puerto " << PUERTO_SERVER << " ocupado.\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        cerr << ">> [ERROR] Fallo el listen.\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    cout << ">> [SERVIDOR AIRSAFE WINDOWS] Escuchando en el puerto " << PUERTO_SERVER << "...\n";

    while (true) {
        if ((nuevo_socket = accept(server_fd, (struct sockaddr*)&direccion, &addrlen)) == INVALID_SOCKET) {
            cerr << ">> [ERROR] Fallo al aceptar la conexion.\n";
            break;
        }
        atenderCliente(nuevo_socket);
    }

    closesocket(server_fd);
    WSACleanup();
    return 0;
}