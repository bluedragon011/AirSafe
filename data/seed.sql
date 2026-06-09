-- Datos de prueba para AirSafe (una instancia por tabla, con claves foraneas validas)
-- Ejecutar sobre data/airsafe.db. password_hash guarda la contrasena tal cual (asi la compara auth.c).
PRAGMA foreign_keys = ON;

-- 1. Aerolineas
INSERT OR IGNORE INTO Aerolineas (id_aerolinea, nombre) VALUES (1, 'Iberia');
INSERT OR IGNORE INTO Aerolineas (id_aerolinea, nombre) VALUES (2, 'Vueling');

-- 2. Aviones
INSERT OR IGNORE INTO Aviones (id_avion, matricula, modelo, id_aerolinea) VALUES (1, 'EC-AAA', 'Airbus A320', 1);
INSERT OR IGNORE INTO Aviones (id_avion, matricula, modelo, id_aerolinea) VALUES (2, 'EC-BBB', 'Airbus A321', 2);

-- 3. Config_cabina
INSERT OR IGNORE INTO Config_cabina (id_avion, num_filas, asientos_por_fila) VALUES (1, 10, 6);
INSERT OR IGNORE INTO Config_cabina (id_avion, num_filas, asientos_por_fila) VALUES (2, 12, 6);

-- 4. Aeropuerto
INSERT OR IGNORE INTO Aeropuerto (id_aeropuerto, nombre, ciudad, pais, latitud, longitud) VALUES (1, 'Adolfo Suarez Barajas', 'Madrid', 'Espana', 40.4719, -3.5626);
INSERT OR IGNORE INTO Aeropuerto (id_aeropuerto, nombre, ciudad, pais, latitud, longitud) VALUES (2, 'Josep Tarradellas El Prat', 'Barcelona', 'Espana', 41.2974, 2.0833);
INSERT OR IGNORE INTO Aeropuerto (id_aeropuerto, nombre, ciudad, pais, latitud, longitud) VALUES (3, 'Bilbao', 'Bilbao', 'Espana', 43.3011, -2.9106);

-- 5. Ruta
INSERT OR IGNORE INTO Ruta (id_ruta, id_origen, id_destino) VALUES (1, 1, 2);
INSERT OR IGNORE INTO Ruta (id_ruta, id_origen, id_destino) VALUES (2, 1, 3);

-- 6. Vuelos
INSERT OR IGNORE INTO Vuelos (id_vuelo, fecha_salida, fecha_llegada, id_avion, ruta) VALUES (1, '2025-08-01 08:00', '2025-08-01 10:00', 1, 'Madrid-Barcelona');
INSERT OR IGNORE INTO Vuelos (id_vuelo, fecha_salida, fecha_llegada, id_avion, ruta) VALUES (2, '2025-08-02 14:00', '2025-08-02 16:30', 2, 'Madrid-Bilbao');

-- 7. Asiento (cada asiento pertenece al avion del vuelo)
INSERT OR IGNORE INTO Asiento (id_asiento, num_asiento, id_avion) VALUES (1, '12A', 1);
INSERT OR IGNORE INTO Asiento (id_asiento, num_asiento, id_avion) VALUES (2, '12B', 1);
INSERT OR IGNORE INTO Asiento (id_asiento, num_asiento, id_avion) VALUES (3, '1A', 2);

-- 8. Usuarios (un CLIENTE para probar el login del cliente y un ADMIN)
-- Sin id fijo: el email es UNIQUE, asi no choca si la tabla ya tiene usuarios.
INSERT OR IGNORE INTO Usuarios (nombre, email, pasaporte, password_hash, telefono, tipo_usuario) VALUES ('Diego Rodriguez', 'diego@airsafe.com', 'ABC123456', '1234', '600111222', 'CLIENTE');
INSERT OR IGNORE INTO Usuarios (nombre, email, pasaporte, password_hash, telefono, tipo_usuario) VALUES ('Admin', 'admin@airsafe.com', 'ADM000000', 'admin', '600000000', 'ADMIN');

-- 9. Reserva (una reserva para diego, solo si todavia no tiene ninguna)
INSERT INTO Reserva (id_usuario, fecha_reserva, precio, id_vuelo, id_asiento)
SELECT (SELECT id_usuario FROM Usuarios WHERE email='diego@airsafe.com'), '2025-07-01', 49.99, 2, 3
WHERE NOT EXISTS (SELECT 1 FROM Reserva WHERE id_usuario=(SELECT id_usuario FROM Usuarios WHERE email='diego@airsafe.com'));
