# MAKEFILE

#Compiladores
CC = gcc
CXX = g++

#Opciones de compilación (Banderas de Windows y rutas)
CFLAGS = -Isrc/shared -D_WIN32_WINNT=0x0601
CXXFLAGS = -Isrc/shared -D_WIN32_WINNT=0x0601
LDFLAGS = -lws2_32

#Carpeta de salida
BUILD_DIR = build

#Archivos Objeto pre-compilados (C puro)
OBJ_SQLITE = $(BUILD_DIR)/sqlite3.o
OBJ_AUTH = $(BUILD_DIR)/auth.o
OBJ_LOGS = $(BUILD_DIR)/logs.o
OBJ_PASAJEROS = $(BUILD_DIR)/pasajeros_db.o

#Ejecutables finales
TARGET_SERVER = $(BUILD_DIR)/server.exe
TARGET_CLIENT = $(BUILD_DIR)/client.exe
TARGET_ADMIN = $(BUILD_DIR)/admin.exe


#Reglas:
#Compilacion
all: directorios $(TARGET_SERVER) $(TARGET_CLIENT) $(TARGET_ADMIN)
	@echo ">> COMPILACION COMPLETADA CON EXITO <<"

#Crear carpeta build para meter los .exe
directorios:
	@mkdir -p $(BUILD_DIR)

#COMPILACION DE OBJETOS (C PURO)
$(OBJ_SQLITE): src/shared/sqlite3.c
	@echo "Compilando SQLite..."
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_AUTH): src/shared/auth.c
	@echo "Compilando Auth..."
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_LOGS): src/shared/logs.c
	@echo "Compilando Logs..."
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PASAJEROS): src/admin/pasajeros_db.c
	@echo "Compilando DB Pasajeros..."
	$(CC) $(CFLAGS) -c $< -o $@


#ENLACE DE EJECUTABLES

#Servidor (C++ con objetos C)
$(TARGET_SERVER): src/admin/server.cpp $(OBJ_AUTH) $(OBJ_LOGS) $(OBJ_PASAJEROS) $(OBJ_SQLITE)
	@echo "Enlazando Servidor..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

#Cliente (C++)
$(TARGET_CLIENT): src/client/client.cpp
	@echo "Enlazando Cliente..."
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

#Administrador (C)
$(TARGET_ADMIN): src/admin/main_admin.c src/admin/vuelos_logic.c $(OBJ_AUTH) $(OBJ_LOGS) $(OBJ_SQLITE)
	@echo "Enlazando Administrador..."
	$(CC) $(CFLAGS) $^ -o $@

#LIMPIEZA
clean:
	@echo "Limpiando binarios..."
	rm -rf $(BUILD_DIR)/*.o $(BUILD_DIR)/*.exe