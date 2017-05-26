#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define KEY_ENTER 10

int main() {

    char texto[141];
    char * puntero = texto;
    struct termios config_actual;
    struct termios config_nueva;


    tcgetattr(STDIN_FILENO, &config_actual); //leo configuracion actual
    config_nueva = config_actual;

    config_nueva.c_lflag &= (~(ICANON | ECHO)); //leemos caracter a caracter y desactivamos el echo 
    
    config_nueva.c_cc[VTIME] = 0; // tiempo para escribir, 0 infinito
    
    config_nueva.c_cc[VMIN]  = 1; // minimo numero de caracteres de uno en uno

    tcsetattr(STDIN_FILENO, TCSANOW, &config_nueva); //cambiamos el modo del terminal //TCSANOW inmediatamente

    printf("Escribe lo que quieras: ");

    do {
        *puntero = getc(stdin);
        puntero++;
    } while (*(puntero - 1) != KEY_ENTER);

	printf("\n");

    *puntero = '\0';

    printf("\nEl texto que has escrito es:  %s\n", texto);


    return (0);
}
