#include <stdio.h>
#include "dummy_main.h"

int dummy_main(int argc, char **argv) {
    printf("Job is running...\n");
    for (int i = 0; i < 5; ++i) {
        printf("Job step %d\n", i);
        sleep(1);  // Simulate work
    }
    return 0;
}
