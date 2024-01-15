#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <syscall.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <limits.h>


enum Resources {
    R1,
    R2,
    R3
};

struct R {

    enum Resources Res;

};

enum Tasks {
    X,
    Y,
    Z
};

enum State {
    Ready,
    Waiting,
    Running
};

struct T {

    enum Tasks Task;

    int priority;

    int num_R_needed;

    enum Resources R_needed[2];

    enum State state;

    int CpuTime;

};







int main()
{


}