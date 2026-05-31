#include <stdio.h>
#include "pasajeros_db.h"

//Archivo temporal para probar funcionalidades. 

char RUTA_DB[150] = "data/airsafe.db"; 

int main() {
    int id_usuario = 1;
    int id_vuelo_futuro = 1;

    printf("=== INICIANDO BATERIA DE PRUEBAS===\n\n");

    printf("1. HISTORIAL INICIAL:\n");
    consultarMisVuelosDB(id_usuario);

    printf("\n2. SIMULANDO COMPRAS SUCESIVAS:\n");
    
    printf("\nIntento 1 (Debería ser EXITO):\n");
    comprarVueloDB(id_usuario, id_vuelo_futuro);

    printf("\nIntento 2 (Debería ser EXITO):\n");
    comprarVueloDB(id_usuario, id_vuelo_futuro);

    printf("\nIntento 3 (Debería dar ERROR DE AFORO, VUELO LLENO):\n");
    comprarVueloDB(id_usuario, id_vuelo_futuro);

    printf("\n3. HISTORIAL ACTUALIZADO:\n");
    consultarMisVuelosDB(id_usuario);

    printf("\n=== PRUEBAS FINALIZADAS ===\n");
    return 0;
}