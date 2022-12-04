


#include <stdlib.h>
#include <stdio.h>

void change(int* truc, int val) {
    *truc = val;
}
void printTab(int* truc, int len) {
    for (int i = 0; i < len; i++) {
        printf("%d ", truc[i]);
    }
    printf("\n");
}

int main(int argc, char* argv) {
    int* test = malloc(sizeof(int) * 5);
    int prime = 0;
    change(&test[0], 1);
    change(&test[3], 1);
    change(&prime, 1);
    printTab(test, 5);
    printf("%d", prime);
    return 0;
}

/*1 1 0 0 0 */