#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Prototipos de funciones
void buscarUsuario();
void gestionarVuelos(int opcion); // 3: Crear, 4: Eliminar, 5: Modificar
void buscarVuelos();

int main(void) {
    int running = 1; 
    char buffer[10];
    int seleccion;

    while (running) {
        printf("\n------------- MENÚ DE ADMINISTRADOR -------------\n");
        printf("1. Buscar usuario\n");
        printf("2. Buscar vuelo\n");
        printf("3. Crear vuelo\n");
        printf("4. Eliminar vuelo\n");
        printf("5. Modificar vuelo\n");
        printf("6. Cerrar sesión\n");
        printf("Seleccione una opción: ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            //Validar que se introduce un número
            if (sscanf(buffer, "%d", &seleccion) != 1) {
                printf("Error: Por favor, introduce un número.\n");
                continue;
            }

            switch (seleccion) {
                case 1:
                    buscarUsuario();
                    break;
                case 2: 
                    buscarVuelos();
                case 3: case 4: case 5:
                    gestionarVuelos(seleccion);
                    break;
                case 6:
                    printf("Cerrando sesión...\n");
                    running = 0; 
                    break;
                default:
                    fprintf(stderr, "Opcion %d no valida. Intentalo de nuevo.\n", seleccion);
            }
        }
    }
    return 0;
}

//IMPLEMENTACION TEMPORAL: BORRAR CUANDO ESTÉN IMPLEMENTADAS LAS VUESTRAS
void buscarUsuario() {
    printf("Buscando usuarios...\n");
}


void buscarVuelos(){
    printf("Buscando vuelos...");
}