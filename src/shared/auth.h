#ifndef AUTH_H_
#define AUTH_H_


void mostrarPantallaInicio(void);

/* Comprueba email y contrasena contra la BD.Devuelve 1 si son correctos, 0 si no. */
int validarCredenciales(char email[], char password_hash[]);

/* Rellena tipo_out con el tipo_usuario del email dado ("ADMIN" o "CLIENTE").
 * Si no existe el usuario, tipo_out queda como cadena vacia. */
void obtenerTipoUsuario(char email[], char tipo_out[]);

/* Gestiona el flujo completo de inicio de sesion, especificos mencionados en auth.c*/
int iniciarSesion(void);


void registrarUsuario(void);

#endif