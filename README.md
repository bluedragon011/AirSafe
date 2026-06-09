# AirSafe

Sistema para administrar una aerolínea: vuelos, pasajeros, reservas y aeropuertos. Hay dos mundos, el de los administradores (que tienen control total de la base de datos) y el de los pasajeros (que se conectan por red nada más para buscar y reservar vuelos). Los dos jalan contra la misma base de SQLite.

----------------
# ¿Cómo está armado?

Son tres ejecutables que trabajan contra la misma base `data/airsafe.db`:

- **admin** la consola del administrador. Maneja usuarios y vuelos, y puede consultar las reservas de un pasajero. Entra directo a la base.
- **server** el servidor de sockets. Se queda escuchando y atiende lo que le pide el cliente (login, ver vuelos, reservar, historial).
- **client** la app del pasajero. Esta no toca la base directo; todo se lo pide al server por red, y guarda en caché la lista de vuelos para no andar molestando al servidor cada rato.

Lo compartido (sqlite3, el protocolo, el login y los logs) vive en `src/shared/`.


# Correr todo (las 3 terminales)

Abre tres terminales, **todas paradas en la raíz del proyecto** (esto importa, si no, no encuentra la carpeta `data/`):

```
build/server.exe      ->  terminal 1: déjalo escuchando
build/client.exe      ->  terminal 2: el pasajero
build/admin.exe       ->  terminal 3: el administrador
```

El flujo de prueba es: entras al cliente, ves los vuelos, compras un boleto, y luego en el admin (opción 6) consultas las reservas de ese usuario para comprobar que sí se guardó.

----------------
# Usuarios de prueba

La base ya trae datos cargados. Para entrar:

- Pasajero (cliente): **diego@airsafe.com** / **1234**
- Administrador (admin): **admin@airsafe.com** / **admin**

Los usuarios admin solo se crean metiéndolos a mano en la base (INSERT INTO Usuarios...). El cliente por red nada más deja pasar a los de tipo CLIENTE.

----------------
# Dónde queda cada cosa

```
src/admin/    ->  admin (main_admin, vuelos_logic, pasajeros_db) y el server
src/client/   ->  el cliente
src/shared/   ->  lo compartido (sqlite3, protocolo, auth, logs, models)
data/         ->  airsafe.db, config.dat y server.log
build/        ->  aquí caen los .exe al compilar
```
