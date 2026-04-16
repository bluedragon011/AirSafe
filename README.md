# AirSafe
Compañía encargada de la gestión de aeropuertos, aerolíneas, vuelos y todo lo relacionado al respecto. Creada tanto para administradores como para clientes ordinarios. 
----------------
# ¡IMPORTANTE!
Al querer loguearse como admin y tener acceso a la BDD:
- Solo se contempla la creacion de usuarios Admin. desde la base de datos (INSERT INTO USUARIOS...).
- Hay un usuario administrador creado para testear/comprobar las funcionalidades:
  <img width="702" height="42" alt="image" src="https://github.com/user-attachments/assets/6d35f62e-6f6e-48e0-9296-fff443c1e4b7" />
----------------
# COMANDOS A USAR (Windows/Linux)
gcc src/admin/main_admin.c src/admin/vuelos_logic.c src/shared/sqlite3.c src/shared/auth.c -Isrc/shared -D_WIN32_WINNT=0x0601 -o admin_app -lkernel32
----------------
Razones:
- Isrc/shared: estamos diciendole al compilador que busque los .h en shared, no solo en la misma carpeta donde estan los .c
- D_WIN32_WINNT=0x0601: especificacion de dispositivo en el que se esta trabajando (Windows en este caso). En caso de trabajar en Linux, mirar apartado Linux.
- lkernel32: enlazado de los archivos a compilar con Windows (permisos a SQLite).
-----------------------------
*EN CASO DE USAR LINUX*:
gcc src/admin/main_admin.c src/admin/vuelos_logic.c src/shared/sqlite3.c src/shared/auth.c -Isrc/shared -o admin_app -lpthread -ldl
- lpthread: enlaza libreria estandar de linux de hilos (para que varios procesos puedan acceder a la vez en la Bd).
- ldl: carga de extensiones si fuera necesaria (dynamic linker). 
