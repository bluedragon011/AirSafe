# Nombre del archivo ejecutable final
TARGET = airsafe_app

# Compiladores para C y C++
CC = gcc
CXX = g++

# Banderas de optimización y errores
CFLAGS = -Wall -Wextra -g
CXXFLAGS = -Wall -Wextra -std=c++11 -g

# Carpetas del proyecto
ADMIN_DIR = src/admin
SHARED_DIR = src/shared

# 1. ARCHIVOS FUENTE (.c) de tu bloque, admin y compartidos
SRCS_C = $(ADMIN_DIR)/main_admin.c \
         $(ADMIN_DIR)/vuelos_logic.c \
         $(ADMIN_DIR)/pasajeros_db.c \
         $(SHARED_DIR)/auth.c \
         $(SHARED_DIR)/logs.c \
         $(SHARED_DIR)/sqlite3.c

# 2. ARCHIVOS FUENTE (.cpp) - Reservado para cuando el Bloque 2 cree el cliente C++
# Nota: Si tu compañero los nombra distinto, solo habrá que actualizar esta línea
SRCS_CPP = 

# Transformación de archivos .c y .cpp en archivos objeto .o
OBJS = $(SRCS_C:.c=.o) $(SRCS_CPP:.cpp=.o)

# Regla principal de compilación
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) -lpthread -ldl
	@echo ">> [EXITO] AirSafe compilado correctamente como './$(TARGET)'"

# Cómo compilar los archivos individuales de C
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Cómo compilar los archivos individuales de C++
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para limpiar los archivos temporales de la carpeta
clean:
	rm -f $(ADMIN_DIR)/*.o $(SHARED_DIR)/*.o *.o $(TARGET)
	@echo ">> [LIMPIEZA] Archivos temporales (.o) eliminados con éxito."