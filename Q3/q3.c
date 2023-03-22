#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct pixel {
    int R;
    int G;
    int B;
};

int main(int argc, char *argv[]) {
    FILE input = fopen(argcv[1], 'r');
    FILE output = fopen(argv[2], 'w');

    return 0;
}