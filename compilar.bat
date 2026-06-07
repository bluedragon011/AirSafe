@echo off
echo "Iniciando compilacion de AirSafe..."

:: Crear carpeta build
if not exist build mkdir build

:: 1. Compilar objetos C
echo Compilando modulos...
gcc -c src/shared/sqlite3.c -Isrc/shared -D_WIN32_WINNT=0x0601 -o build/sqlite3.o
gcc -c src/shared/auth.c -Isrc/shared -o build/auth.o
gcc -c src/shared/logs.c -Isrc/shared -o build/logs.o
gcc -c src/admin/pasajeros_db.c -Isrc/shared -o build/pasajeros_db.o

:: 2. Enlazar Servidor
echo Enlazando Servidor...
g++ src/admin/server.cpp build/auth.o build/logs.o build/pasajeros_db.o build/sqlite3.o -Isrc/shared -lws2_32 -D_WIN32_WINNT=0x0601 -o build/server.exe

:: 3. Enlazar Cliente
echo Enlazando Cliente...
g++ src/client/client.cpp -Isrc/shared -lws2_32 -D_WIN32_WINNT=0x0601 -o build/client.exe

:: 4. Enlazar Admin
echo Enlazando Administrador...
gcc src/admin/main_admin.c src/admin/vuelos_logic.c build/auth.o build/logs.o build/sqlite3.o -Isrc/shared -D_WIN32_WINNT=0x0601 -o build/admin.exe

echo "------------------------------------------"
echo "PROCESO FINALIZADO. Archivos en /build"
pause