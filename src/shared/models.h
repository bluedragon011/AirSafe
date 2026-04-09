#ifndef MODELS_H
#define MODELS_H

// --- Estructura para Aerolineas ---
typedef struct {
    int id_aerolinea; //PK
    char nombre[50];
} Aerolinea;

// --- Estructura para Aviones ---
typedef struct {
    int id_avion; //PK
    char matricula[15];
    char modelo[50];
    int id_aerolinea; //FK a Aerolineas
} Avion;

// --- Estructura para Config_cabina ---
typedef struct {
    int id_avion;      //FK a Aviones
    int num_filas;
    int asientos_por_fila;
} ConfigCabina;

// --- Estructura para Vuelos ---
typedef struct {
    int id_vuelo;
    char fecha_salida[20];  //Formato "YYYY-MM-DD HH:MM:SS"
    char fecha_llegada[20]; //Formato "YYYY-MM-DD HH:MM:SS"
    int id_avion;           //FK a Aviones
} Vuelo;

#endif // MODELS_H