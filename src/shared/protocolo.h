#ifndef PROTOCOLO_H
#define PROTOCOLO_H

// --- Peticiones que el Cliente le hace al Servidor ---
// Formato: "LOGIN|email|password"
#define REQ_LOGIN             "LOGIN"
// Formato: "LISTAR_VUELOS"   (respuesta: lineas "id|salida|llegada|id_avion|ruta")
#define REQ_LISTAR_VUELOS     "LISTAR_VUELOS"
// Formato: "LISTAR_ASIENTOS|id_vuelo"   (respuesta: lineas "id_asiento|num_asiento")
#define REQ_LISTAR_ASIENTOS   "LISTAR_ASIENTOS"
// Formato: "COMPRAR_VUELO|id_usuario|id_vuelo|id_asiento|precio"
#define REQ_COMPRAR_VUELO     "COMPRAR_VUELO"
// Formato: "CONSULTAR_HISTORIAL|id_usuario"   (respuesta: lineas "id_reserva|fecha|ruta|asiento|precio")
#define REQ_CONSULTAR         "CONSULTAR_HISTORIAL"

// --- Respuestas que el Servidor le devuelve al Cliente ---
// El login OK responde "LOGIN_OK|id_usuario|nombre"
#define RES_LOGIN_OK          "LOGIN_OK"
#define RES_LOGIN_DENEGADO    "LOGIN_DENEGADO"
#define RES_COMPRA_OK         "COMPRA_OK"
#define RES_COMPRA_LLENO      "COMPRA_COMPLETO"
#define RES_ERROR             "ERROR"

// Tamaño del búfer de texto para enviar datos por la red
#define BUFFER_SIZE 2048

#endif