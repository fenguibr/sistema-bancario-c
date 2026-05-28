#include <stdio.h>

int main() {

    // caracteres
    char fenguizadaa = 'a';
    char fenguizada = 1;
    unsigned char fenguizada1 = 1;

    printf("Valor de fenguizadaa: %c\n", fenguizadaa);

    // numeros inteiros
    int fenguizada2 = 1;
    printf("valor de fenguizada2: %i\n", fenguizada2);
    long int fenguizada5 = 1;
    printf("valor de fenguizada5: %li\n", fenguizada5);
    short int fenguizada4 = 1;
    printf("valor de fenguizada4: %i\n", fenguizada4);
    unsigned int fenguizada8 = 1;
    printf("valor de fenguizada8: %u\n", fenguizada8);

    // numeros reais
    float fenguizada3 = 1.0f;
    printf("valor de fenguizada3: %f\n", fenguizada3);
    double fenguizada6 = 1.0;
    printf("valor de fenguizada6: %lf\n", fenguizada6);
    long double fenguizada7 = 3.9e-23l;
    printf("valor de fenguizada7: %Lf\n", fenguizada7);

    // constantes nomeadas
    const int fenguizada9 = 10;
    printf("valor de fenguizada9: %i\n", fenguizada9);

    // fenguizada9 = 100; // erro

    /* comentarios */

    return 0;
}