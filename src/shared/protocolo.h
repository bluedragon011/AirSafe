#ifndef PROTOCOLO_H
#define PROTOCOLO_H

// --- Peticiones que el Cliente le hace al Servidor ---
// Formato: "LOGIN|email|password"
#define REQ_LOGIN             "LOGIN"
// Formato: "COMPRAR_VUELO|id_usuario|id_vuelo"
#define REQ_COMPRAR_VUELO     "COMPRAR_VUELO"
// Formato: "CONSULTAR_HISTORIAL|id_usuario"
#define REQ_CONSULTAR         "CONSULTAR_HISTORIAL"

// --- Respuestas que el Servidor le devuelve al Cliente ---
#define RES_LOGIN_OK          "LOGIN_OK"
#define RES_LOGIN_DENEGADO    "LOGIN_DENEGADO"
#define RES_COMPRA_OK         "COMPRA_OK"
#define RES_COMPRA_LLENO      "COMPRA_COMPLETO"
#define RES_ERROR             "ERROR"

// Tamaño del búfer de texto para enviar datos por la red
#define BUFFER_SIZE 2048

#endif