#include <iostream>
#include <vector>
#include <queue>
#include <pthread.h>
#include <time.h>


using namespace std;
//priority queue START


//priority queue END

#define cpuCount 4

pthread_t cpuThreads[cpuCount];

pthread_t mainThread;

long long int CLOCK; //time

pthread_mutex_t mainThread_mutex;



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

enum CPUState {
    CPU_Running,
    CPU_Waiting,
    CPU_Idle,
    CPU_Preempt,
    CPU_Terminate

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

    int agetime = 0;

};

struct CPU_data {
    T* currentTask;

    enum CPUState state;

    pthread_cond_t wake;
};

vector<T> ReadyQueue;

vector<T> WaitingQueue;

int R1num, R2num, R3num;

int n;

CPU_data cpu_datas[cpuCount];

struct arg {

    int id;

};

void* Main_thread(void* arguments);

void* CPU_thread(void* arguments);

void start() {

    for (int i = 0; i < cpuCount; i++) {

        cpu_datas[i] = *new CPU_data();

        cpu_datas[i].currentTask = NULL;

        cpu_datas[i].state = CPU_Idle;

        pthread_cond_init(&cpu_datas[i].wake, NULL);

    }

    pthread_mutex_init(&mainThread_mutex, NULL);

    pthread_create(&mainThread, NULL, Main_thread, NULL);



    for (int i = 0; i < cpuCount; i++) {

        struct arg* args = new struct arg();

        args->id = i;

        pthread_create(&cpuThreads[i], NULL, CPU_thread, (void*)args);

    }

}

void* CPU_thread(void* arguments) {

    struct arg* args = (struct arg*)arguments;

    int cpuID = args->id;

    pthread_mutex_lock(&mainThread_mutex);

    pthread_cond_signal(&cpu_datas[cpuID].wake);


    if (cpu_datas[cpuID].currentTask == NULL) {

        cpu_datas[cpuID].state = CPU_Idle;
    }
    else {

        cpu_datas[cpuID].state = CPU_Running;

        while (cpu_datas[cpuID].state == CPU_Running) {

            pthread_cond_wait(&cpu_datas[cpuID].wake, &mainThread_mutex);

        }



    }

    CPUState currentState = cpu_datas[cpuID].state;


    pthread_mutex_unlock(&mainThread_mutex);


    switch (currentState)
    {
    case CPU_Terminate:
        //terminate
        break;
    case CPU_Idle:
        //Idle
        break;
    case CPU_Preempt:
        //preempt
        break;
    case CPU_Waiting:
        //preempt
        break;


    default:
        break;
    }







}

void proccess(int cpuId, T* currentTask) {

    if (currentTask->Burst > 0) {

        currentTask->Burst--;

        currentTask->CpuTime++;

        //check for preemtion

        {
            cpu_datas[cpuId].state = CPU_Preempt;

            pthread_cond_signal(&cpu_datas[cpuId].wake);
        }

        //preempt


        pthread_cond_wait(&cpu_datas[cpuId].wake, &mainThread_mutex);


    }
    else {

        cpu_datas[cpuId].state = CPU_Terminate;

        pthread_cond_signal(&cpu_datas[cpuId].wake);

        //terminate

        //we use the cond variable as a semaphore in order to make sure that the main thread schedules a proccess to the cpu before we do all of this
        pthread_cond_wait(&cpu_datas[cpuId].wake, &mainThread_mutex);


    }


}

void print() {

    cout << " R1 : " << R1num << " R2 : " << R2num << " R3 : " << R3num << endl;

    cout << "Ready Queue :" << endl;

    cout << "[";

    for (int i = 0; i < ReadyQueue.size(); i++) {

        cout << ReadyQueue[i].name;

        if (i != ReadyQueue.size() - 1)
            cout << "-";
    }
    cout << "]" << endl;

    cout << "[";

    for (int i = 0; i < WaitingQueue.size(); i++) {

        cout << WaitingQueue[i].name;

        if (i != WaitingQueue.size() - 1)
            cout << "-";
    }
    cout << "]" << endl;

    for (int i = 0; i < cpuCount; i++) {

        cout << "CPU" << i + 1 << ":" << "[";

        if (cpu_datas[i].currentTask != nullptr) {
            cout << cpu_datas[i].currentTask->name;
        }
        else {
            cout << "Idle";
        }


        cout << "]";

    }


}

void* Main_thread(void* arguments) {

    while (true) {

        pthread_mutex_lock(&mainThread_mutex);


        if (ReadyQueue.size() == 0 and WaitingQueue.size() == 0) {
            exit(0);
        }

        for (int i = 0; i < cpuCount; i++) {

            T* nextTask;//=az safe ready va algorithm ha

            proccess(i, nextTask);


        }

        CLOCK++;

        print();

        pthread_mutex_unlock(&mainThread_mutex);


        //sleep

        struct timespec tSpec;

        tSpec.tv_sec = 1 / 1000000;

        tSpec.tv_nsec = (1 % 1000000) * 1000;

        while (nanosleep(&tSpec, &tSpec) != 0);


    }


}




//request
bool request(T task) {
    if (task.Task == X) {
        if (R1num == 0 || R2num == 0) {
            WaitingQueue.push_back(task);
            return false;
        }
    }
    else if (task.Task == Y) {
        if (R3num == 0 || R2num == 0) {
            WaitingQueue.push_back(task);
            return false;
        }
    }
    else if (task.Task == Z) {
        if (R1num == 0 || R3num == 0) {
            WaitingQueue.push_back(task);
            return false;
        }
    }
    else {
        return true;
    }
}





//aging
void age(T &task)
{
    if (task.agetime >= 5) {
        task.agetime = 0;
        task.priority = task.priority - 1;
        }
}

void age_all() {
    for (int i = 0; i < WaitingQueue.size(); i++) {
        if (WaitingQueue[i].agetime < 5 && WaitingQueue[i].priority != 1){
            WaitingQueue[i].agetime = WaitingQueue[i].agetime + 1;
        }
        else if (WaitingQueue[i].agetime == 5) {
            WaitingQueue[i].agetime = 0;
            WaitingQueue[i].priority = WaitingQueue[i].priority - 1;
        }
    }
}



//RR pushback
void RRpushback(T task) {
    ReadyQueue.push_back(task);
}



//terminate
void terminate(T task) {
    for (int i = 0; i < ReadyQueue.size(); i++) {
        if (ReadyQueue[i].name == task.name)
        {
            ReadyQueue.erase(ReadyQueue.begin() + i);
        }
    }
    for(int i = 0; i < WaitingQueue.size(); i++){
        if (WaitingQueue[i].name == task.name)
        {
            WaitingQueue.erase(WaitingQueue.begin() + i);
        }
    }
}

//sort
void WaitingQueueprioritysort() {
    for (int i = 1; i < WaitingQueue.size(); i++) {
        T var1 = WaitingQueue[i];
        int j = i - 1;
        while (j >= 0 && WaitingQueue[j].priority > var1.priority) {
            WaitingQueue[j + 1] = WaitingQueue[j];
            j--;
        }
        WaitingQueue[j + 1] = var1;
    }
}

void ReadyQueueprioritysort() {
    for (int i = 1; i < ReadyQueue.size(); i++) {
        T var1 = ReadyQueue[i];
        int j = i - 1;
        while (j >= 0 && ReadyQueue[j].priority > var1.priority) {
            ReadyQueue[j + 1] = ReadyQueue[j];
            j--;
        }
        ReadyQueue[j + 1] = var1;
    }
}

void ReadyQueueBurstsort() {
    for (int i = 1; i < ReadyQueue.size(); i++) {
        T var1 = ReadyQueue[i];
        int j = i - 1;
        while (j >= 0 && ReadyQueue[j].Burst > var1.Burst) {
            ReadyQueue[j + 1] = ReadyQueue[j];
            j--;
        }
        ReadyQueue[j + 1] = var1;
    }
}







int main()
{


    cin >> R1num >> R2num >> R3num;


    cin >> n;

    for (int i = 0; i < n; i++) {

        T tempTask;

        char tempTasks;

        string tempName;

        int tempBurst;

        cin >> tempName >> tempTasks >> tempBurst;

        tempTask.name = tempName;

        tempTask.num_R_needed = 2;


        tempTask.state = Ready;

        tempTask.Burst = tempBurst;


        if (tempTasks == 'X') {

            tempTask.Task = X;

            tempTask.priority = 3;

            tempTask.R_needed[0] = R1;

            tempTask.R_needed[1] = R2;


        }
        else if (tempTasks == 'Y') {

            tempTask.Task = Y;

            tempTask.priority = 2;

            tempTask.R_needed[0] = R2;

            tempTask.R_needed[1] = R3;

        }
        else if (tempTasks == 'Z') {

            tempTask.Task = Z;

            tempTask.priority = 1;

            tempTask.R_needed[0] = R1;

            tempTask.R_needed[1] = R3;

        }

        ReadyQueue.push_back(tempTask);



    }

    start();

    for (int i = 0; i < 4; i++) {
        pthread_join(cpuThreads[i], NULL);
    }
    pthread_join(mainThread, NULL);



    return 0;

}