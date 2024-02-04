
#include <iostream>
#include <stdio.h>
#include <vector>
#include <queue>
#include <pthread.h>
#include <time.h>
#include <algorithm>
#include <string.h>
#include <string>
#include <bits/stdc++.h>

using namespace std;

using std::basic_string;

#define cpuCount 4

int TimeQ = 3;

#define WakeupTime 3

pthread_t cpuThreads[cpuCount];

pthread_t mainThread;

long long int CLOCK; //time

int RunNum;

pthread_mutex_t mainThread_mutex;

pthread_mutex_t readyMutex;

pthread_mutex_t readyMutex2;

pthread_mutex_t readyMutex3;

pthread_mutex_t waitingMutex;

pthread_mutex_t idleMutex;

pthread_mutex_t WR_mutex;

pthread_mutex_t R_mutex;

pthread_mutex_t level_mutex;

pthread_mutex_t RunNum_mutex;

pthread_mutex_t currentMutexs[4];

int terminated_Proccesses;

string algo;

bool RR;

int level;


enum Resources {
    R1,
    R2,
    R3
};

struct R {

    enum Resources Res;

};

enum Tasks {
    X = 3,
    Y = 2,
    Z = 1
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

    float HRRN;

    int Burst;

    int agetime;

};

struct CPU_data {

    T* currentTask;

    enum CPUState state;

    pthread_cond_t wake;

    int preemtionTimer;
};

vector<T> ReadyQueue;

vector<T> ReadyQueue1;

vector<T> ReadyQueue2;

vector<T> ReadyQueue3;


vector<T> WaitingQueue;

int R1num, R2num, R3num;

int n;

CPU_data cpu_datas[cpuCount];

struct arg {

    int id;

};

void* Main_thread(void* arguments);

void* CPU_thread(void* arguments);

void print();


pthread_cond_t idleCond;

class IRWL {
public:
    IRWL() {
        pthread_mutex_init(&mutex, nullptr);
        pthread_cond_init(&no_writers, nullptr);
        writers = 0;
    }

    void readerLock() {
        pthread_mutex_lock(&mutex);
        while (writers > 0) {
            pthread_cond_wait(&no_writers, &mutex);
        }
    }

    void readerUnlock() {
        pthread_mutex_unlock(&mutex);
    }

    void writerLock() {
        pthread_mutex_lock(&mutex);
        writers++;
        pthread_mutex_unlock(&mutex);
    }

    void writerUnlock() {
        pthread_mutex_lock(&mutex);
        writers--;
        if (writers == 0) {
            pthread_cond_signal(&no_writers);
        }
        pthread_mutex_unlock(&mutex);
    }

private:
    pthread_mutex_t mutex;
    pthread_cond_t no_writers;
    int writers;
};

IRWL RW_LOCK;

void WaitingToReady();

int tempReq(T task);



void QueueToReady() {
    pthread_mutex_lock(&readyMutex);
    pthread_mutex_lock(&level_mutex);
    if (level == 2) {
        TimeQ = TimeQ * 2;
        for (int i = ReadyQueue2.size() - 1; i >= 0; i--) {
            ReadyQueue.push_back(ReadyQueue2[i]);
        }
    }
    if (level == 3) {
        RR = false;
        algo = "FCFS";
        for (int i = ReadyQueue3.size() - 1; i >= 0; i--) {
            ReadyQueue.push_back(ReadyQueue3[i]);
        }
    }
    pthread_mutex_unlock(&readyMutex);
    pthread_mutex_unlock(&level_mutex);
}


void WaitingToReady() {

    pthread_mutex_lock(&readyMutex);
    pthread_mutex_lock(&waitingMutex);

    bool x = false;
    while (x == false && WaitingQueue.size() > 0)
    {
        if (tempReq(WaitingQueue.front()) == 0) {
            T temp = WaitingQueue.front();

            ReadyQueue.push_back(temp);

            WaitingQueue.erase(WaitingQueue.begin());

        }
        else {
            x = true;
        }
    }




    pthread_mutex_unlock(&waitingMutex);
    pthread_mutex_unlock(&readyMutex);
}

int tempReq(T task) {//reverse nums for sort

    pthread_mutex_lock(&R_mutex);

    if (task.Task == X) {
        if (R1num == 0 || R2num == 0) {
            pthread_mutex_unlock(&R_mutex);
            return 1;
        }
        else {
            pthread_mutex_unlock(&R_mutex);
            return 0;
        }
    }
    else if (task.Task == Y) {
        if (R3num == 0 || R2num == 0) {
            pthread_mutex_unlock(&R_mutex);
            return 1;
        }
        else {
            pthread_mutex_unlock(&R_mutex);
            return 0;
        }
    }
    else if (task.Task == Z) {
        if (R1num == 0 || R3num == 0) {
            pthread_mutex_unlock(&R_mutex);
            return 1;
        }
        else {
            pthread_mutex_unlock(&R_mutex);
            return 0;
        }
    }
    else {
        pthread_mutex_unlock(&R_mutex);
        exit(0);
    }



}


struct MyEntry {
    int first;
    int second;
    T temp;
};

bool compare_entry(const MyEntry& e1, const MyEntry& e2) {
    if (e1.first != e2.first)
        return (e1.first < e2.first);
    return (e1.second < e2.second);
}


//sort
void WaitingQueueprioritysort() {//check

    pthread_mutex_lock(&waitingMutex);

    vector<MyEntry> Entries;

    for (int i = 0; i < WaitingQueue.size(); i++) {

        struct MyEntry m = *new struct MyEntry();

        m.first = tempReq(WaitingQueue[i]);


        m.second = WaitingQueue[i].priority;

        m.temp = WaitingQueue[i];

        Entries.push_back(m);

    }

    sort(Entries.begin(), Entries.end(), compare_entry);

    for (int i = 0; i < WaitingQueue.size(); i++)
        WaitingQueue[i] = Entries[i].temp;


    pthread_mutex_unlock(&waitingMutex);
}

void ReadyQueueprioritysort() {

    pthread_mutex_lock(&readyMutex);

    for (int i = 1; i < ReadyQueue.size(); i++) {
        T var1 = ReadyQueue[i];
        int j = i - 1;
        while (j >= 0 && ReadyQueue[j].priority > var1.priority) {
            ReadyQueue[j + 1] = ReadyQueue[j];
            j--;
        }
        ReadyQueue[j + 1] = var1;
    }

    pthread_mutex_unlock(&readyMutex);
}

void ReadyQueueBurstsort() {

    pthread_mutex_lock(&readyMutex);

    for (int i = 1; i < ReadyQueue.size(); i++) {
        T var1 = ReadyQueue[i];
        int j = i - 1;
        while (j >= 0 && ReadyQueue[j].Burst > var1.Burst) {
            ReadyQueue[j + 1] = ReadyQueue[j];
            j--;
        }
        ReadyQueue[j + 1] = var1;
    }

    pthread_mutex_unlock(&readyMutex);
}

void WaitingQueueBurstsort() {

    pthread_mutex_lock(&waitingMutex);

    vector<MyEntry> Entries;


    for (int i = 0; i < WaitingQueue.size(); i++) {

        struct MyEntry m = *new struct MyEntry();

        m.first = tempReq(WaitingQueue[i]);

        m.second = WaitingQueue[i].Burst;

        m.temp = WaitingQueue[i];

        Entries.push_back(m);

    }

    sort(Entries.begin(), Entries.end(), compare_entry);

    for (int i = 0; i < WaitingQueue.size(); i++)
        WaitingQueue[i] = Entries[i].temp;

    pthread_mutex_unlock(&waitingMutex);
}
void ReadyQueueHRRRN() {

    pthread_mutex_lock(&readyMutex);

    for (int i = 1; i < ReadyQueue.size(); i++) {
        T var1 = ReadyQueue[i];
        int j = i - 1;
        while (j >= 0 && ReadyQueue[j].HRRN > var1.HRRN) {
            ReadyQueue[j + 1] = ReadyQueue[j];
            j--;
        }
        ReadyQueue[j + 1] = var1;
    }

    pthread_mutex_unlock(&readyMutex);
}


//request
bool request(T task) {



    pthread_mutex_lock(&R_mutex);

    if (task.Task == X) {
        if (R1num == 0 || R2num == 0) {
            ReadyQueue.erase(ReadyQueue.begin());
            WaitingQueue.push_back(task);
            pthread_cond_broadcast(&idleCond);
            pthread_mutex_unlock(&R_mutex);
            return false;
        }
        else {
            R1num--;
            R2num--;
            pthread_mutex_unlock(&R_mutex);
            return true;
        }
    }
    else if (task.Task == Y) {
        if (R3num == 0 || R2num == 0) {
            ReadyQueue.erase(ReadyQueue.begin());
            WaitingQueue.push_back(task);
            pthread_cond_broadcast(&idleCond);
            pthread_mutex_unlock(&R_mutex);
            return false;
        }
        else {
            R2num--;
            R3num--;
            pthread_mutex_unlock(&R_mutex);
            return true;
        }
    }
    else { //else if (task.Task == Z) {
        if (R1num == 0 || R3num == 0) {
            ReadyQueue.erase(ReadyQueue.begin());
            WaitingQueue.push_back(task);
            pthread_cond_broadcast(&idleCond);
            pthread_mutex_unlock(&R_mutex);
            return false;
        }
        else {
            R1num--;
            R3num--;
            pthread_mutex_unlock(&R_mutex);
            return true;
        }
    }



}

void context_switch(int cpuID) {



    T t = ReadyQueue.front();

    pthread_mutex_lock(&currentMutexs[cpuID]);

    cpu_datas[cpuID].currentTask = &t;

    cpu_datas[cpuID].preemtionTimer = TimeQ;

    pthread_mutex_unlock(&currentMutexs[cpuID]);

    ReadyQueue.erase(ReadyQueue.begin());

    //cout<<cpu_datas[cpuID].currentTask->name<<endl;



}

//schedule
void schedule(int cpuID) {//check

    if (algo == "SJF") {
        WaitingQueueBurstsort();
        ReadyQueueBurstsort();
    }
    else if (RR == true) {
        WaitingQueueprioritysort();
    }
    else {
        WaitingQueueprioritysort();
    }

    pthread_mutex_lock(&waitingMutex);
    pthread_mutex_lock(&readyMutex);



    if (WaitingQueue.size() == 0) {//waiting empty , ready ->

        bool x = false;
        while (x == false && ReadyQueue.size() > 0)
        {
            if (request(ReadyQueue.front()) == true) {
                context_switch(cpuID);
                x = true;
            }
        }
        if (x == false) {
            pthread_mutex_lock(&currentMutexs[cpuID]);

            cpu_datas[cpuID].currentTask = NULL;

            pthread_mutex_unlock(&currentMutexs[cpuID]);

        }



    }
    else if (ReadyQueue.size() == 0) {//ready empty ,waiting ->

        if (tempReq(WaitingQueue.front()) == 0) {//we have the resources

            T temp = WaitingQueue.front();

            ReadyQueue.insert(ReadyQueue.begin(), temp);

            if (request(ReadyQueue.front()) == true)
                context_switch(cpuID);

            WaitingQueue.erase(WaitingQueue.begin());

        }
        else {//nothing
            pthread_mutex_lock(&currentMutexs[cpuID]);

            cpu_datas[cpuID].currentTask = NULL;

            pthread_mutex_unlock(&currentMutexs[cpuID]);
        }

    }
    else {//both not empty

        if (tempReq(WaitingQueue.front()) == 1) {//ready goes through

            bool x = false;

            while (x == false && ReadyQueue.size() > 0)
            {
                if (request(ReadyQueue.front()) == true) {
                    context_switch(cpuID);
                    x = true;
                }
            }
            if (x == false) {
                pthread_mutex_lock(&currentMutexs[cpuID]);

                cpu_datas[cpuID].currentTask = NULL;

                pthread_mutex_unlock(&currentMutexs[cpuID]);

            }





        }

        else if (tempReq(ReadyQueue.front()) == 1 && tempReq(WaitingQueue.front()) == 0) {//waiting goes through


            T temp = WaitingQueue.front();


            ReadyQueue.insert(ReadyQueue.begin(), temp);



            if (request(ReadyQueue.front()) == true)
                context_switch(cpuID);

            WaitingQueue.erase(WaitingQueue.begin());



        }
        else if (tempReq(ReadyQueue.front()) == 0 && tempReq(WaitingQueue.front()) == 0) {//both should be checked 
            if (ReadyQueue.front().priority <= WaitingQueue.front().priority) {//ready



                if (request(ReadyQueue.front()) == true) {
                    context_switch(cpuID);
                }


            }


            else if (ReadyQueue.front().priority > WaitingQueue.front().priority) {//waiting



                T temp = WaitingQueue.front();



                ReadyQueue.insert(ReadyQueue.begin(), temp);




                if (request(ReadyQueue.front()) == true)
                    context_switch(cpuID);

                WaitingQueue.erase(WaitingQueue.begin());


            }
        }
    }


    pthread_mutex_unlock(&waitingMutex);
    pthread_mutex_unlock(&readyMutex);
}






//idle
void idle(int cpuID) {

    pthread_mutex_lock(&idleMutex);

    while (ReadyQueue.size() == 0) {
        pthread_cond_wait(&idleCond, &idleMutex);
    }
    pthread_mutex_unlock(&idleMutex);

    RW_LOCK.writerLock();
    schedule(cpuID);
    RW_LOCK.writerUnlock();



}



//RR pushback
void RRpushback(T& task) {

    pthread_mutex_lock(&readyMutex);

    ReadyQueue.push_back(task);

    pthread_mutex_unlock(&readyMutex);
}

void MFQPreempt(int cpuID) {
    pthread_mutex_lock(&currentMutexs[cpuID]);

    T& currentTask = *cpu_datas[cpuID].currentTask;

    pthread_mutex_lock(&R_mutex);

    if (currentTask.Task == X) {
        R1num++;
        R2num++;
    }
    else if (currentTask.Task == Y) {
        R2num++;
        R3num++;
    }
    else if (currentTask.Task == Z) {
        R1num++;
        R3num++;
    }

    pthread_mutex_unlock(&R_mutex);

    pthread_mutex_lock(&level_mutex);
    if (level == 1) {

        pthread_mutex_lock(&readyMutex2);

        ReadyQueue2.push_back(currentTask);

        pthread_mutex_unlock(&readyMutex2);


    }
    else if (level == 2) {
        pthread_mutex_lock(&readyMutex3);

        ReadyQueue3.push_back(currentTask);

        pthread_mutex_unlock(&readyMutex3);
    }
    pthread_mutex_unlock(&level_mutex);

    pthread_mutex_unlock(&currentMutexs[cpuID]);


    WaitingQueueprioritysort();


    WaitingToReady();


    pthread_mutex_lock(&RunNum_mutex);

    RunNum++;

    pthread_mutex_unlock(&RunNum_mutex);

    if (RunNum >= n) {
        bool check = true;
        for (int i = 0; i < cpuCount; i++) {
            pthread_mutex_lock(&currentMutexs[i]);
            if (cpu_datas[i].currentTask != NULL)
                check = false;
            pthread_mutex_unlock(&currentMutexs[i]);
        }
        if (check == true) {
            pthread_mutex_lock(&RunNum_mutex);

            RunNum = 0;

            pthread_mutex_unlock(&RunNum_mutex);
            RW_LOCK.writerLock();
            pthread_mutex_lock(&level_mutex);
            level++;
            pthread_mutex_unlock(&level_mutex);
            RW_LOCK.writerUnlock();
            QueueToReady();
        }

    }
    RW_LOCK.writerLock();
    schedule(cpuID);
    RW_LOCK.writerUnlock();
}


//preempt
void preempt(int cpuID) {

    pthread_mutex_lock(&currentMutexs[cpuID]);

    T& currentTask = *cpu_datas[cpuID].currentTask;

    pthread_mutex_lock(&R_mutex);

    if (currentTask.Task == X) {
        R1num++;
        R2num++;
    }
    else if (currentTask.Task == Y) {
        R2num++;
        R3num++;
    }
    else if (currentTask.Task == Z) {
        R1num++;
        R3num++;
    }

    pthread_mutex_unlock(&R_mutex);


    RRpushback(currentTask);

    pthread_mutex_unlock(&currentMutexs[cpuID]);

    if (algo == "SJF") {
        WaitingQueueBurstsort();
    }
    else if (RR == true) {
        WaitingQueueprioritysort();
    }
    else {
        WaitingQueueprioritysort();
    }


    WaitingToReady();


    RW_LOCK.writerLock();
    schedule(cpuID);
    RW_LOCK.writerUnlock();



}




//aging
void age(T& task)
{
    if (task.agetime >= 5) {
        task.agetime = 0;
        task.priority = task.priority - 1;
    }
}

void age_all() {

    pthread_mutex_lock(&waitingMutex);


    for (int i = 0; i < WaitingQueue.size(); i++) {

        if (WaitingQueue[i].agetime < WakeupTime && WaitingQueue[i].priority != 1) {

            WaitingQueue[i].agetime = WaitingQueue[i].agetime + 1;
        }
        else if (WaitingQueue[i].agetime == WakeupTime) {
            WaitingQueue[i].agetime = 0;
            WaitingQueue[i].priority = WaitingQueue[i].priority - 1;
        }
    }

    pthread_mutex_unlock(&waitingMutex);
}

void HRRN_INIT() {
    pthread_mutex_lock(&waitingMutex);
    pthread_mutex_lock(&readyMutex);


    for (int i = 0; i < ReadyQueue.size(); i++) {
        ReadyQueue[i].HRRN = (float)(((float)CLOCK + (float)ReadyQueue[i].Burst) / (float)ReadyQueue[i].Burst);
    }

    for (int i = 0; i < WaitingQueue.size(); i++) {
        WaitingQueue[i].HRRN = (float)(((float)CLOCK + (float)WaitingQueue[i].Burst) / (float)WaitingQueue[i].Burst);

    }


    pthread_mutex_unlock(&readyMutex);
    pthread_mutex_unlock(&waitingMutex);
}


//terminate
void Terminate(int cpuID) {

    pthread_mutex_lock(&currentMutexs[cpuID]);

    T task = *cpu_datas[cpuID].currentTask;

    pthread_mutex_lock(&R_mutex);

    if (task.Task == X) {
        R1num++;
        R2num++;
    }
    else if (task.Task == Y) {
        R2num++;
        R3num++;
    }
    else if (task.Task == Z) {
        R1num++;
        R3num++;
    }

    pthread_mutex_unlock(&R_mutex);

    if (algo == "SJF") {
        WaitingQueueBurstsort();
    }
    else if (algo == "RR") {
        WaitingQueueprioritysort();
    }
    else if (algo == "FCFS") {
        WaitingQueueprioritysort();
    }
    else if (algo == "HRRN") {
        HRRN_INIT();
        WaitingQueueprioritysort();
        ReadyQueueHRRRN();
    }


    WaitingToReady();

    pthread_mutex_lock(&RunNum_mutex);

    RunNum++;

    pthread_mutex_unlock(&RunNum_mutex);
    if (RunNum >= n) {
        bool check = true;
        for (int i = 0; i < cpuCount; i++) {
            pthread_mutex_lock(&currentMutexs[i]);
            if (cpu_datas[i].currentTask != NULL)
                check = false;
            pthread_mutex_unlock(&currentMutexs[i]);
        }
        if (check == true) {
            pthread_mutex_lock(&RunNum_mutex);

            RunNum = 0;

            pthread_mutex_unlock(&RunNum_mutex);
            RW_LOCK.writerLock();
            pthread_mutex_lock(&level_mutex);
            level++;
            pthread_mutex_unlock(&level_mutex);
            RW_LOCK.writerUnlock();
            QueueToReady();
        }
    }





    cpu_datas[cpuID].currentTask = NULL;



    pthread_mutex_unlock(&currentMutexs[cpuID]);



}



void start() {

    for (int i = 0; i < cpuCount; i++) {

        cpu_datas[i] = *new CPU_data();

        cpu_datas[i].currentTask = NULL;

        cpu_datas[i].state = CPU_Idle;

        if (RR == true) {
            cpu_datas[i].preemtionTimer = TimeQ;
        }

        pthread_cond_init(&cpu_datas[i].wake, NULL);

        pthread_mutex_init(&currentMutexs[i], NULL);

    }

    RunNum = 0;


    pthread_mutex_init(&mainThread_mutex, NULL);
    pthread_mutex_init(&waitingMutex, NULL);
    pthread_mutex_init(&WR_mutex, NULL);

    pthread_mutex_init(&R_mutex, NULL);

    pthread_mutex_init(&idleMutex, NULL);

    pthread_mutex_init(&level_mutex, NULL);

    pthread_mutex_init(&RunNum_mutex, NULL);


    RW_LOCK = *new IRWL();



    for (int i = 0; i < cpuCount; i++) {

        struct arg* args = new struct arg();

        args->id = i;

        pthread_create(&cpuThreads[i], NULL, CPU_thread, (void*)args);

    }

    pthread_create(&mainThread, NULL, Main_thread, NULL);

}

void* CPU_thread(void* arguments) {

    struct arg* args = (struct arg*)arguments;

    int cpuID = args->id;

    while (true) {



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


            pthread_mutex_lock(&mainThread_mutex);

            terminated_Proccesses++;

            pthread_mutex_unlock(&mainThread_mutex);

            RW_LOCK.writerLock();

            Terminate(cpuID);

            RW_LOCK.writerUnlock();



            break;
        case CPU_Idle:

            idle(cpuID);


            break;
        case CPU_Preempt:
            RW_LOCK.writerLock();

            if (algo == "MFQ") {
                MFQPreempt(cpuID);
            }
            else {
                preempt(cpuID);
            }




            RW_LOCK.writerUnlock();
            break;


        default:
            break;
        }




    }


}

void proccess(int cpuId, T& currentTask) {

    if (currentTask.Burst > 0) {

        currentTask.Burst--;

        currentTask.CpuTime++;

        if (RR) {

            cpu_datas[cpuId].preemtionTimer--;

        }

        if (RR && cpu_datas[cpuId].preemtionTimer == 0)

        {
            cpu_datas[cpuId].state = CPU_Preempt;

            pthread_cond_signal(&cpu_datas[cpuId].wake);


            pthread_cond_wait(&cpu_datas[cpuId].wake, &mainThread_mutex);
        }

        //preempt




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

    RW_LOCK.readerLock();
    if (algo == "MFQ") {
        cout << "level : " << level << " " << RunNum << endl;

        cout << "Ready Queue2 :" << endl;

        cout << "[";

        for (int i = 0; i < ReadyQueue2.size(); i++) {

            cout << ReadyQueue2[i].name;

            if (i != ReadyQueue2.size() - 1)
                cout << "-";
        }

        cout << "]" << endl;


    }

    cout << " R1 : " << R1num << " R2 : " << R2num << " R3 : " << R3num << endl;

    cout << "Ready Queue :" << endl;

    cout << "[";

    for (int i = 0; i < ReadyQueue.size(); i++) {

        cout << ReadyQueue[i].name;

        if (i != ReadyQueue.size() - 1)
            cout << "-";
    }

    cout << "]" << endl;

    cout << "Waiting Queue :" << endl;

    cout << "[";


    for (int i = 0; i < WaitingQueue.size(); i++) {

        cout << WaitingQueue[i].name;

        if (i != WaitingQueue.size() - 1)
            cout << "-";
    }

    cout << "]" << endl;






    for (int i = 0; i < cpuCount; i++) {


        cout << "CPU" << i + 1 << ":" << "[";

        pthread_mutex_lock(&currentMutexs[i]);

        if (cpu_datas[i].currentTask != NULL) {

            cout << cpu_datas[i].currentTask->name << "     ";

        }
        else {
            cout << "Idle";
        }


        cout << "]";

        pthread_mutex_unlock(&currentMutexs[i]);


    }



    RW_LOCK.readerUnlock();


}

void* Main_thread(void* arguments) {

    while (true) {

        pthread_mutex_lock(&mainThread_mutex);


        if (terminated_Proccesses >= n) {

            exit(0);
        }

        for (int i = 0; i < cpuCount; i++) {

            if (cpu_datas[i].currentTask != NULL) {

                T& nextTask = *cpu_datas[i].currentTask;

                proccess(i, nextTask);

            }


        }

        CLOCK++;

        age_all();

        cout << endl << "Time :" << CLOCK << endl;

        print();

        pthread_mutex_unlock(&mainThread_mutex);


        //sleep

        long int time = 1 * 1000000;

        //usleep(time);

        struct timespec tSpec;

        tSpec.tv_sec = time / 1000000;

        tSpec.tv_nsec = (time % 1000000) * 1000;

        while (nanosleep(&tSpec, &tSpec) != 0);



    }


}






int main()
{

    string tempNames[1000];

    char tempTasks_[100000];

    int tempBursts[10000];

    //cin.ignore();

    cin >> algo;

    cin >> R1num >> R2num >> R3num;


    cin >> n;


    int i;

    for (i = 0; i < n; i++) {

        cin >> tempNames[i] >> tempTasks_[i] >> tempBursts[i];
    }



    for (int i = 0; i < n; i++) {

        T tempTask = *new T();

        char tempTasks = tempTasks_[i];

        string tempName = tempNames[i];

        int tempBurst = tempBursts[i];

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
        else {
            exit(1);
        }

        ReadyQueue.push_back(tempTask);





    }


    RR = false;
    pthread_mutex_init(&readyMutex, NULL);

    pthread_mutex_init(&readyMutex2, NULL);

    pthread_mutex_init(&readyMutex3, NULL);

    if (algo == "RR") {
        RR = true;
    }
    else if (algo == "FCFS") {

    }
    else if (algo == "SJF") {
        ReadyQueueBurstsort();
    }
    else if (algo == "HRRN") {

    }
    else if (algo == "MFQ") {
        level = 1;
        RR = true;
        for (int i = 0; i < ReadyQueue.size(); i++) {
            ReadyQueue1.push_back(ReadyQueue[i]);
        }
    }
    else {
        exit(0);
    }

    start();


    pthread_join(mainThread, NULL);

    for (int i = 0; i < 4; i++) {
        pthread_join(cpuThreads[i], NULL);
    }




    return 0;

}