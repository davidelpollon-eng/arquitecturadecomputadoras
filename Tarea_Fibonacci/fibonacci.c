#include <stdio.h>

int main(void) {
    int n, i;
    long long t1 = 0, t2 = 1, siguiente;

    printf("Ingrese el numero de terminos de Fibonacci a generar: ");
    scanf("%d", &n);

    printf("Serie de Fibonacci:\n");
    for (i = 1; i <= n; ++i) {
        printf("%lld ", t1);
        siguiente = t1 + t2;
        t1 = t2;
        t2 = siguiente;
    }
    printf("\n");

    return 0;
}
