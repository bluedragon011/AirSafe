#include <stdio.h>
#include <time.h>
#include "logs.h" 

void registrar_log(const char *mensaje) {
    FILE *f = fopen("data/server.log", "a");
    if (f == NULL) return;

    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char cadena_fecha[20];
    
    // Formato estándar: YYYY-MM-DD HH:MM:SS
   
    strftime(cadena_fecha, sizeof(cadena_fecha), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(f, "[%s] %s\n", cadena_fecha, mensaje);
    fclose(f);
}