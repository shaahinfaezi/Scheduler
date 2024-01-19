#include <iostream>
#include <vector>
#include <queue>
#include <pthread.h>
 
using namespace std;
 
pthread_t cpuThreads[4];

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

    string name;

    enum Tasks Task;

    int priority;

    int num_R_needed;

    enum Resources R_needed[2];

    enum State state;

    int CpuTime;

    int Burst;

};

vector<T*> ReadyQueue;

vector<T*> WaitingQueue;

int R1num,R2num,R3num;

int n;

struct arg{

};

void *CPU(void * arguments){

    while(ReadyQueue.size()!=0 or WaitingQueue.size()!=0){

        
        //increment semaphore until burst

    }
}

void * scheduler(void * arguments){

    for (int i = 0; i < 4; i++) {
        pthread_create(&cpuThreads[i], NULL,CPU,NULL);
    }


}


int main()
{


    cin>>R1num>>R2num>>R3num;


    cin>>n;

    for(int i=0;i<n;i++){

        T *tempTask=new T();

        char tempTasks;

        string tempName;

        int tempBurst;

        cin>>tempName>>tempTasks>>tempBurst;

        tempTask->name=tempName;

        tempTask->num_R_needed=2;


        tempTask->state=Ready;

        tempTask->Burst=tempBurst;


        if(tempTasks=='X'){

            tempTask->Task=X;

            tempTask->priority=3;

            tempTask->R_needed[0]=R1;

            tempTask->R_needed[1]=R2;
            

        }
        else if(tempTasks=='Y'){

            tempTask->Task=Y;

            tempTask->priority=2;

            tempTask->R_needed[0]=R2;

            tempTask->R_needed[1]=R3;

        }
        else if(tempTasks=='Z'){

            tempTask->Task=Z;

            tempTask->priority=1;

            tempTask->R_needed[0]=R1;

            tempTask->R_needed[1]=R3;

        }

        ReadyQueue.push_back(tempTask);


        
    }

    pthread_t schedulerThread;

    pthread_create(&schedulerThread, NULL, scheduler, NULL);

    for (int i = 0; i < 4; i++) {
        pthread_join(cpuThreads[i], NULL);
    }
    pthread_join(schedulerThread, NULL);
    


    return 0;

}