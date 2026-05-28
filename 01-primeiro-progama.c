#include<stdio.h>

void main() {
    int senha = 0;
    while (senha != 1234) {
        printf ("digite sua senha:\n");
        scanf("%d", &senha);
        if (senha == 1234) {
            printf("senha correta\n");
        } else {
            printf("senha incorreta, tente novamente\n");
        }
    }
}