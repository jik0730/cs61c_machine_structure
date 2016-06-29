#include <stdio.h>
#include <string.h>

int main() {
    char *a = "hello";
    printf("%lu\n%p\n", sizeof(a), a);

    int len = 0;
    while (NULL) {
        ++len;
        printf("%c %p\n", *a, a);
    }
    printf("%d\n", len);

    return 0;
}