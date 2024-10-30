#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>


void handle_sigcont(int signo) {
    // Do nothing
}
void sigint(int signo){
    return;
}
int dummy_main(int argc, char **argv);
int main(int argc, char **argv) {
    signal(SIGCONT, handle_sigcont);
    signal(SIGINT, sigint);
    // printf("%d\n", getpid());
    pause();
/* You can add any code here you want to support your SimpleScheduler implementation*/
    int ret = dummy_main(argc, argv);
    return ret;
}
#define main dummy_main