#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<math.h>

////////////////////////////////////////////////////////////////////////////////
double PI;
double sum = 0.0;

////////////////////////////////////////////////////////////////////////////////
struct thread {
    pthread_t Thread;
    int ThreadStart;
    int ThreadEnd;
};

////////////////////////////////////////////////////////////////////////////////
void* formula(void *thd) {
    struct thread *T = (struct thread *) thd;
    for(int n= T->ThreadStart; n<= T->ThreadEnd; n++) {
        double Num = pow(-1, n);
        double Denm = (2*n)+ 1;
        sum += (Num/Denm);
    }
pthread_exit(NULL);
}

////////////////////////////////////////////////////////////////////////////////
void threadSlicing(int noOfIterations, int noOfThreads, struct thread *threads) {
    int commonWorkCount;
    int remainingWork;

    if(noOfThreads > noOfIterations) {
        commonWorkCount = noOfThreads / noOfIterations;
        remainingWork = noOfThreads % noOfIterations;
    } else {
        commonWorkCount = noOfIterations / noOfThreads;
        remainingWork = noOfIterations % noOfThreads;
    }

    int PreviousEnd = -1;
    for(int i=0; i<noOfThreads; i++) {
        struct thread T;
        T.Thread = 0;
        T.ThreadStart = PreviousEnd + 1;
        T.ThreadEnd = PreviousEnd + commonWorkCount;
        threads[i] = T;
        PreviousEnd += commonWorkCount;
    }

    for(int i=0; i<remainingWork; i++) {
        threads[i].ThreadEnd += 1;
        for(int j=i+1; j<noOfThreads; j++) {
            threads[j].ThreadStart += 1;
            threads[j].ThreadEnd += 1;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void runThread(int noOfThreads, struct thread *threads) {
    for(int i=0; i<noOfThreads; i++) {
        pthread_create(&threads[i].Thread, NULL,formula, &threads[i]);
    }

    for(int i=0; i<noOfThreads; i++) {
        pthread_join(threads[i].Thread, NULL);
    }
}

////////////////////////////////////////////////////////////////////////////////
void main() {
    int noOfIterations;
    int noOfThreads;
    
    printf("\nEnter How Many Iteration :: ");
    scanf("%d", &noOfIterations);
    printf("\nEnter How Many Threads :: ");
    scanf("%d", &noOfThreads);

    struct thread Threads[noOfThreads];
    threadSlicing(noOfIterations, noOfThreads, Threads);
    runThread(noOfThreads, Threads);
    
    PI = sum*4;

    printf("Value of PI == %lf\n\n", PI);
}