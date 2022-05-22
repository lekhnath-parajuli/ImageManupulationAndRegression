#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

////////////////////////////////////////////////////////////////////////////////
struct Prime {
    long long totalNumbers;
    long long currentIndex;
    long long totalPrimeNumbers;
	long long *IsPrime;
};

////////////////////////////////////////////////////////////////////////////////
struct thread {
    pthread_t Thread;
    int ThreadStart;
    int ThreadEnd;
};

////////////////////////////////////////////////////////////////////////////////
FILE *fp[5];
struct Prime PrimeData;
    
////////////////////////////////////////////////////////////////////////////////
void ReloadData() {
    fp [0] = fopen("./PrimeNumbers/IsPrime1.txt", "r");
	fp [1] = fopen("./PrimeNumbers/IsPrime2.txt", "r");
	fp [2] = fopen("./PrimeNumbers/IsPrime3.txt", "r");	
    fp [3] = fopen("./PrimeNumbers/PrimeNumbers.txt", "a");	
    fp [4] = fopen("./PrimeNumbers/PrimeNumbers.txt", "r");									
}

////////////////////////////////////////////////////////////////////////////////
long long FindLength(void *file) {
    FILE *Fp = (FILE *) file;
    long long length = 0;
    long long temp = 0;
    while(!feof(Fp)) {	 
        fscanf(Fp, "%lld\n",&temp);
        length += 1;
    }fclose(Fp);
    return length;													
}

////////////////////////////////////////////////////////////////////////////////
void LodeData(void *file) {
    FILE *Fp = (FILE *) file;
    while(!feof(Fp)) {			   
        fscanf(Fp,"%lld\n", &PrimeData.IsPrime[PrimeData.currentIndex]);
        PrimeData.currentIndex += 1;
    } fclose(Fp);													
}

////////////////////////////////////////////////////////////////////////////////
void InitilizeData() {
    ReloadData();
    for(int i=0; i<3; i++) {
        PrimeData.totalNumbers += FindLength(fp[i]);
    } 

    PrimeData.IsPrime = (long long *) malloc(sizeof(long long) * PrimeData.totalNumbers);
    ReloadData();

    for(int i=0; i<3; i++) {
        LodeData(fp[i]);
    }
}

////////////////////////////////////////////////////////////////////////////////
void IsPrime(int *N) {
    int n = *N;
    int IsIt = 0;

    for(int i=2; i<PrimeData.IsPrime[n] / 2; i++) {
        if (PrimeData.IsPrime[n] % i == 0) {
            IsIt = 1;
            break;
        }
    }

    if((IsIt == 0) && (PrimeData.IsPrime[n] != 0) && (PrimeData.IsPrime[n] != 1) && (PrimeData.IsPrime[n] != 2)) {
        PrimeData.totalPrimeNumbers += 1;
        fprintf(fp[3], "%lld\n", PrimeData.IsPrime[n]);
    }
}

////////////////////////////////////////////////////////////////////////////////
void *Prime(void *thd) {
    struct thread *T = (struct thread *) thd;
    for(int n= T->ThreadStart; n<= T->ThreadEnd; n++) {
        IsPrime(&n);
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
        pthread_create(&threads[i].Thread, NULL,Prime, &threads[i]);
    }

    for(int i=0; i<noOfThreads; i++) {
        pthread_join(threads[i].Thread, NULL);
    }
    fclose(fp[3]);
    free(PrimeData.IsPrime);
}

////////////////////////////////////////////////////////////////////////////////
void main() {
    remove("./PrimeNumbers/PrimeNumbers.txt");
    PrimeData.totalNumbers = 0;
    PrimeData.currentIndex = 0;
    long long noOfThreads;

    InitilizeData();
    long long noOfIterations = PrimeData.totalNumbers;

    printf("\nEnter How Many Threads :: ");
    scanf("%lld", &noOfThreads);

    struct thread Threads[noOfThreads];
    threadSlicing(noOfIterations, noOfThreads, Threads);
    runThread(noOfThreads, Threads);

    ReloadData();
    printf("\nTotal %lld Prime Numbers Sucessfully added to the PrimeNumber.txt File\n", FindLength(fp[4]));
}
