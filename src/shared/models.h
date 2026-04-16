#ifndef MODELS_H_
#define MODELS_H_

// Estructura Aerolineas
typedef struct {
    int id_aerolinea; //PK
    char nombre[50];
} Aerolinea;

// Estructura Aviones
typedef struct {
    int id_avion; //PK
    char matricula[15];
    char modelo[50];
    int id_aerolinea; //FK a Aerolineas
} Avion;

// Estructura Config_cabina
typedef struct {
    int id_avion;      //FK a Aviones
    int num_filas;
    int asientos_por_fila;
} ConfigCabina;

// Estructura Vuelo
typedef struct {
    int id_vuelo;
    char fecha_salida[20];
    char fecha_llegada[20];
    int id_avion;
    char ruta[100];
} Vuelo;


// Estructura Aeropuerto
typedef struct {
    int id_aeropuerto;
    char nombre[100];
    char ciudad[100];
    char pais[100];
    double latitud;
    double longitud;
} Aeropuerto;

// Estructura Ruta
typedef struct {
    int id_ruta;
    int id_origen;
    int id_destino;
} Ruta;

// Estructura Asiento
typedef struct {
    int id_asiento;
    char num_asiento[10];
    int id_avion; // FK a Avion
} Asiento;

// Estructura Reserva
typedef struct {
    int id_reserva;
    int id_usuario;   // FK a Usuario
    char fecha_reserva[20];
    float precio;
    int id_vuelo;     // FK a Vuelo
    int id_asiento;   // FK a Asiento
} Reserva;

 /* Estructura Usuario*/
typedef struct {
    int  id_usuario; //PK
    char nombre[100];
    char email[100];
    char pasaporte[40];
    char password_hash[64];
    char telefono[20];
    char tipo_usuario[10]; /* "ADMIN" o "CLIENTE" */
} Usuario;

#endif // MODELS_H_