CREATE TABLE Aeropuerto (
    id_aeropuerto INT PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL,
    ciudad VARCHAR(100),
    pais VARCHAR(100),
    latitud DECIMAL(10,6),
    longitud DECIMAL(10,6)
);
CREATE TABLE Ruta (
    id_ruta INT PRIMARY KEY,
    id_origen INT,
    id_destino INT
);
CREATE TABLE Asiento (
    id_asiento INT PRIMARY KEY,
    num_asiento VARCHAR(10) NOT NULL,
    id_avion INT,
    
    FOREIGN KEY (id_avion) REFERENCES Avion(id_avion)
);
CREATE TABLE Reserva (
    id_reserva INT PRIMARY KEY,
    id_usuario INT,
    fecha_reserva DATE,
    precio DECIMAL(10,2),
    id_vuelo INT,
    id_asiento INT,
    
    FOREIGN KEY (id_usuario) REFERENCES Usuario(id_usuario),
    FOREIGN KEY (id_vuelo) REFERENCES Vuelo(id_vuelo),
    FOREIGN KEY (id_asiento) REFERENCES Asiento(id_asiento)
);