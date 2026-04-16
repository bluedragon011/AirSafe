#include <stdio.h>
#include <string.h>
#include "models.h"
//FUNCION AUXILIAR PARA LIMPIAR RASTRO DEL TECLADO
void limpiarBuffer(){
    int c;
    while ((c=getchar()) != '\n'&& c!=EOF);
}


void crearVuelo(){
    Vuelo nuevo;
    printf("\n--- formulario: crear nuevo vuelo----\n");

    printf("ID del avion a asignar: ");
    scanf("%d",&nuevo.id_avion);
    limpiarBuffer();

    printf("fecha de salida  (YYYY-MM-DD HH:MM:SS): ");
    fgets(nuevo.fecha_salida, sizeof(nuevo.fecha_salida), stdin);//para guardar datos y teniendo limite
    nuevo.fecha_salida[strcspn(nuevo.fecha_salida, "\n")] = 0; //borra salto linea y guardarlo mejor

    printf("fecha de llegada (YYYY-MM-DD HH:MM:SS): ");
    fgets(nuevo.fecha_llegada, sizeof(nuevo.fecha_llegada, "\n"),stdin);
    nuevo.fecha_llegada[strcspn(nuevo.fecha_llegada, "\n")]=0;

    printf("\n>> [SIMULACION] Insertando en tabla Vuelos: Avion %d, Salida %s...\n", 
            nuevo.id_avion, nuevo.fecha_salida);
    printf(">> Vuelo registrado con exito.\n");
    

}

void eliminarVuelo(){
    int id;
    printf("\n--- modo: eliminar vuelo---\n");
    printf("introduce el id del vuelo que deseas cancelar: ");
    scanf("%d",&id);
    limpiarBuffer();
    printf(">> [SIMULACION] El vuelo con ID %d ha sido marcado como CANCELADO en la DB.\n", id);

}

void modificarVuelo(){
    int id;
    char nueva_fecha[20];
    printf("\n----modo: modificar vuelo------\n");
    printf("introduce el id del vuelo a editar: ");
    scanf("%d",&id);
    limpiarBuffer();

    printf("Nueva Fecha de Salida (YYYY-MM-DD HH:MM:SS): ");
    fgets(nueva_fecha, sizeof(nueva_fecha), stdin);
    nueva_fecha[strcspn(nueva_fecha, "\n")] = 0;

    printf(">> [SIMULACION] Actualizando ID %d con nueva fecha: %s\n", id, nueva_fecha);
    printf(">> Cambio guardado correctamente.\n");

}

void gestionarVuelos(int opcion){
    switch (opcion) {
        case 3:
           crearVuelo();
           break;
        case 4:
           eliminarVuelo();
           break;
        case 5:
           modificarVuelo();
           break;
        default:
        printf("opcion no reconocida. \n");
    }
}