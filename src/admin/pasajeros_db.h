int comprarVueloDB(int id_usuario, int id_vuelo, int id_asiento);
void consultarMisVuelosDB(int id_usuario);
//Funciones adaptadas para enviar datos por Sockets
void obtenerVuelosRed(char *buffer);
void obtenerHistorialRed(int id_usuario, char *buffer);
void obtenerAsientosLibresRed(int id_vuelo, char *buffer);