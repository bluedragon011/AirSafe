# Makefile de AirSafe
# 'make all' compila los 3 ejecutables (admin, server, client) dentro de build/

CC       = gcc
CXX      = g++
CFLAGS   = -Wall -O2
CXXFLAGS = -std=c++11 -Wall -O2
STATIC   = -static

BUILD  = build
SHARED = src/shared
ADMIN  = src/admin
CLIENT = src/client

all: $(BUILD)/admin.exe $(BUILD)/server.exe $(BUILD)/client.exe
	@echo Listo. Los 3 ejecutables estan en build/

# Carpeta de salida
$(BUILD):
	@if not exist $(BUILD) mkdir $(BUILD)

# --- Objetos compartidos (codigo C) ---
$(BUILD)/sqlite3.o: $(SHARED)/sqlite3.c | $(BUILD)
	$(CC) $(CFLAGS) -c $(SHARED)/sqlite3.c -o $(BUILD)/sqlite3.o

$(BUILD)/auth.o: $(SHARED)/auth.c | $(BUILD)
	$(CC) $(CFLAGS) -c $(SHARED)/auth.c -o $(BUILD)/auth.o

$(BUILD)/logs.o: $(SHARED)/logs.c | $(BUILD)
	$(CC) $(CFLAGS) -c $(SHARED)/logs.c -o $(BUILD)/logs.o

# --- Aplicacion de administrador (consola, C) ---
$(BUILD)/admin.exe: $(ADMIN)/main_admin.c $(ADMIN)/vuelos_logic.c $(ADMIN)/pasajeros_db.c $(BUILD)/auth.o $(BUILD)/logs.o $(BUILD)/sqlite3.o | $(BUILD)
	$(CC) $(CFLAGS) $(ADMIN)/main_admin.c $(ADMIN)/vuelos_logic.c $(ADMIN)/pasajeros_db.c $(BUILD)/auth.o $(BUILD)/logs.o $(BUILD)/sqlite3.o -o $(BUILD)/admin.exe $(STATIC)

# --- Servidor de sockets (C++) ---
$(BUILD)/server.exe: $(ADMIN)/server.cpp $(BUILD)/auth.o $(BUILD)/logs.o $(BUILD)/sqlite3.o | $(BUILD)
	$(CXX) $(CXXFLAGS) $(ADMIN)/server.cpp $(BUILD)/auth.o $(BUILD)/logs.o $(BUILD)/sqlite3.o -o $(BUILD)/server.exe $(STATIC) -lws2_32

# --- Cliente de pasajeros (C++) ---
$(BUILD)/client.exe: $(CLIENT)/client.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) $(CLIENT)/client.cpp -o $(BUILD)/client.exe $(STATIC) -lws2_32

clean:
	@if exist $(BUILD) rmdir /s /q $(BUILD)
