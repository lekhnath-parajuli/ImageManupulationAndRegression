#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include "lodepng.h"

////////////////////////////////////////////////////////////////////////////////
struct Image {
    unsigned char ***oldImageIn3D;
    unsigned char ***newImageIn3D;
    unsigned char *oldImageIn1D;
    unsigned char *newImageIn1D;
    unsigned int Height;
    unsigned int Width;
};

////////////////////////////////////////////////////////////////////////////////
struct thread {
    pthread_t Thread;
    int ThreadXStart;
    int ThreadXEnd;
    int ThreadYStart;
    int ThreadYEnd;
    int axis;
};

////////////////////////////////////////////////////////////////////////////////
struct Queue {
    int Front;
    int Rear;
    int Capacity;
    struct thread *thread;
};


struct Image image;
// get all image related data
////////////////////////////////////////////////////////////////////////////////
void getImageData(struct Image *image) {
    lodepng_decode32_file(&image->oldImageIn1D, &image->Width, &image->Height, "./image/Original.png");
    image->newImageIn1D = (unsigned char*) malloc(sizeof(unsigned char)*(image->Height * image->Width * 4));
}

// generate 3D array to fit all 1d image into 3D array
////////////////////////////////////////////////////////////////////////////////
void get3DArray(struct Image *image) {
    unsigned char *D1;
    unsigned char **D2;

    image->oldImageIn3D = (unsigned char ***) malloc(sizeof(unsigned char *) * image->Height);
    image->newImageIn3D = (unsigned char ***) malloc(sizeof(unsigned char *) * image->Height);

    for(int i=0; i<image->Height; i++) { 
        D2 = (unsigned char **) malloc(sizeof(unsigned char *) * image->Width);
        for(int j=0; j<image->Width; j++) { 
            D1 = (unsigned char *) malloc(sizeof(unsigned char) * 4);
            D2[j] = D1;
        }
        image->oldImageIn3D[i] = D2; 
    }

    for(int i=0; i<image->Height; i++) { 
        D2 = (unsigned char **) malloc(sizeof(unsigned char*) * image->Width);
        for(int j=0; j<image->Width; j++) { 
            D1 = (unsigned char *) malloc(sizeof(unsigned char) * 4);
            D2[j] = D1;
        }
        image->newImageIn3D[i] = D2; 
    }
}

// represent 1D image in 3D array
////////////////////////////////////////////////////////////////////////////////
void representImageIn3D(struct Image *image) {
    long long currentIndex = 0;
    for(int y=0; y<image->Height; y++) {
        for(int x=0; x<image->Width; x++) {
            for(int k=0; k<4; k++) {
                image->oldImageIn3D[y][x][k] = image->oldImageIn1D[currentIndex];
                currentIndex += 1;
            }
        }
    }
}

// find mean of R, G and B values
////////////////////////////////////////////////////////////////////////////////
void findMean(int *noOfNeighbors, unsigned char *Pixel, int *rMean, int *gMean, int *bMean) {
    *rMean += Pixel[0];
    *gMean += Pixel[1];
    *bMean += Pixel[2];
    *noOfNeighbors += 1;
}

// Find All the neighbors of the Pilex
////////////////////////////////////////////////////////////////////////////////
void getNeighborsMean(struct Image *image, int x, int y) {  
    int noOfNeighbors = 0;

    int haveLeft = x - 1;
    int haveRight = x + 1;
    int haveTop = y - 1;
    int haveBottom = y + 1;
    
    int rMean= image->oldImageIn3D[y][x][0],
     gMean = image->oldImageIn3D[y][x][1],
     bMean = image->oldImageIn3D[y][x][2];
    
    image->newImageIn3D[y][x][3] = image->oldImageIn3D[y][x][3];

    // it finds out if different pixel positions exist or not
    if(haveLeft   > -1)            { haveLeft = 1;   findMean(&noOfNeighbors, image->oldImageIn3D[y][haveLeft],   &rMean, &gMean, &bMean);} else {haveLeft   = 0;}
    if(haveRight  < image->Width)  { haveRight = 1;  findMean(&noOfNeighbors, image->oldImageIn3D[y][haveRight],  &rMean, &gMean, &bMean);} else {haveRight  = 0;}
    if(haveTop    > -1)            { haveTop = 1;    findMean(&noOfNeighbors, image->oldImageIn3D[haveTop][x],    &rMean, &gMean, &bMean);} else {haveTop    = 0;}
    if(haveBottom < image->Height) { haveBottom = 1; findMean(&noOfNeighbors, image->oldImageIn3D[haveBottom][x], &rMean, &gMean, &bMean);} else {haveBottom = 0;}

    if(haveLeft  && haveTop)       { findMean(&noOfNeighbors, image->oldImageIn3D[y-1][x-1], &rMean, &gMean, &bMean);}
    if(haveLeft  && haveBottom)    { findMean(&noOfNeighbors, image->oldImageIn3D[y+1][x-1], &rMean, &gMean, &bMean);}
    if(haveRight && haveTop)       { findMean(&noOfNeighbors, image->oldImageIn3D[y-1][x+1], &rMean, &gMean, &bMean);}
    if(haveRight && haveBottom)    { findMean(&noOfNeighbors, image->oldImageIn3D[y+1][x+1], &rMean, &gMean, &bMean);}

    image->newImageIn3D[y][x][0] =  rMean / noOfNeighbors;
    image->newImageIn3D[y][x][1] =  gMean / noOfNeighbors;
    image->newImageIn3D[y][x][2] =  bMean / noOfNeighbors;
}

// it apply gussian Blur filter to all image pilex by pilex
////////////////////////////////////////////////////////////////////////////////
void* applyGussianBlurFilter(void *thd) {
    struct thread *T = (struct thread *) thd;

    for(int y=T->ThreadYStart ;y<T->ThreadYEnd; y++) {
        for(int x=T->ThreadXStart; x<T->ThreadXEnd; x++) {
            getNeighborsMean(&image, x, y);
        }
    }
}

// convert the created image from 3D to 1D again
////////////////////////////////////////////////////////////////////////////////
void convertNewImageFrom3DTo1D(struct Image *image) {
    long long currentIndex = 0;
    for(int y=0; y<image->Height; y++) {
        for(int x=0; x<image->Width; x++) {
            for(int z=0; z<4; z++) {
                image->newImageIn1D[currentIndex] = image->newImageIn3D[y][x][z];
                currentIndex += 1;
            }
        }
    }
}

// check if the Queue is full or not
////////////////////////////////////////////////////////////////////////////////
int isFull(struct Queue *Q) {
    return ((Q->Rear + 1) % Q->Capacity == Q->Front);
}

// check if the Queue is empty or not
////////////////////////////////////////////////////////////////////////////////
int isEmpty(struct Queue *Q) {
    return (Q->Front == -1);
}

// put a thread in Queue
////////////////////////////////////////////////////////////////////////////////
void enQueue(struct Queue *Q, struct thread t) {
    if(isFull(Q)) {
        printf("\nQueue IsFull");
    } else {

        Q->Rear = ((Q->Rear + 1) % Q->Capacity);
        Q->thread[Q->Rear] = t;

        if(Q->Front == -1) Q->Front = Q->Rear;
    }
}

// get a thread from Queue
////////////////////////////////////////////////////////////////////////////////
struct thread deQueue(struct Queue *Q) {
    struct thread t;
    if(isEmpty(Q)) {
        printf("\nQuee is Empty");
        exit(1);
    } else {
        t = Q->thread[Q->Front];
        
        if(Q->Front == Q->Rear) Q->Front = Q->Rear = -1;
        else Q->Front = ((Q->Front + 1) % Q->Capacity);
    }
    return t;
}

// Delete Queue and free all memory location
////////////////////////////////////////////////////////////////////////////////
void deleteQueue(struct Queue *Q) {
    if(Q){
        if(Q->thread) {
            free(Q->thread);
        }
        free(Q);
    }
}

////////////////////////////////////////////////////////////////////////////////
struct Queue* CreateQueue(int noOfThreads) {
    struct Queue *Q = malloc(sizeof(struct Queue));
    if(!Q){printf("\nQueue not created !"); return NULL;}

    Q->Capacity = noOfThreads;
    Q->Front = Q->Rear = -1;
    Q->thread = (struct thread *)malloc(sizeof(struct thread)*noOfThreads);
    
    if(!Q->thread){printf("\nArray Inside Queue is not Created !"); return NULL;}
    return(Q);
}

// this thread slicer slices images int no of threads part 
// each image slice is a rectangular pieces having its
// own heihgt and width
////////////////////////////////////////////////////////////////////////////////
void threadSlicing(int height, int width, int noOfThreads, struct thread *threads) {
    struct Queue *Q = CreateQueue(noOfThreads);

    while(!isFull(Q)) {
        if(isEmpty(Q)) {
            struct thread t;
            t.ThreadXStart = 0;
            t.ThreadXEnd = width;
            t.ThreadYStart = 0;
            t.ThreadYEnd = height;
            t.axis = 0;
            enQueue(Q, t);
    } else {
        struct thread t = deQueue(Q);
        struct thread t2;
        int common = 0;
        int remaining = 0;
            if(t.axis == 0) {
                common = (t.ThreadYEnd - t.ThreadYStart) / 2;
                remaining = (t.ThreadYEnd - t.ThreadYStart) % 2;
                t2.axis = 1;

                if(t.ThreadYStart > t.ThreadYEnd) {
                    printf("not fixed");
                }
                t2.ThreadXStart = t.ThreadXStart;
                t2.ThreadXEnd = t.ThreadXEnd;
                t2.ThreadYStart = t.ThreadYStart;
                t2.ThreadYEnd =  t.ThreadYStart + common + remaining;
                enQueue(Q, t2);
                t2.ThreadYStart = t.ThreadYStart + common + remaining + 1;
                t2.ThreadYEnd = t.ThreadYEnd;
                enQueue(Q, t2);
            } else if(t.axis == 1) {
                common = (t.ThreadXEnd - t.ThreadXStart) / 2;
                remaining = (t.ThreadXEnd - t.ThreadXStart) % 2;
                t2.axis = 0;

                if(t.ThreadXStart > t.ThreadXEnd) {
                    printf("not fixed");
                }
                t2.ThreadXStart = t.ThreadXStart;
                t2.ThreadXEnd = t.ThreadXStart + common + remaining;
                t2.ThreadYStart = t.ThreadYStart;
                t2.ThreadYEnd = t.ThreadYEnd;
                enQueue(Q, t2);
                t2.ThreadXStart = t.ThreadXStart + common + remaining + 1;
                t2.ThreadXEnd = t.ThreadXEnd;
                enQueue(Q, t2);
            }
        }
    }
    
    for(int i=0; i<noOfThreads; i++) {
        threads[i] = deQueue(Q);
    }

    deleteQueue(Q);
}

// it runs each rectangular piece of image in different thread
////////////////////////////////////////////////////////////////////////////////
void runThread(int noOfThreads, struct thread *threads) {
    for(int i=0; i<noOfThreads; i++) {
        pthread_create(&threads[i].Thread, NULL,applyGussianBlurFilter, &threads[i]);
    }

    for(int i=0; i<noOfThreads; i++) {
        pthread_join(threads[i].Thread, NULL);
    }
}

// it recreate the image having GusseanBlurFilter
////////////////////////////////////////////////////////////////////////////////
void reCreateImage(struct Image *image) {
    lodepng_encode32_file("./image/GussianBluredImage.png", image->newImageIn1D, image->Width,image->Height);
    free(image->newImageIn1D);
    free(image->newImageIn3D);
    free(image->oldImageIn1D);
    free(image->oldImageIn3D);
}

////////////////////////////////////////////////////////////////////////////////
void main() {
    long long noOfThreads;
    printf("\n\nenter How Many Threads Yout Want to Run it On:: ");
    scanf("%lld", &noOfThreads);
    struct thread Threads[noOfThreads];

    // image is a globel variable
    getImageData(&image);
    get3DArray(&image);
    representImageIn3D(&image); 

    threadSlicing(image.Height, image.Width, noOfThreads, Threads);
    runThread(noOfThreads, Threads);

    convertNewImageFrom3DTo1D(&image);
    reCreateImage(&image);

    printf("\nsucessfully created ./GussianBluredImage.png Image !\n\n");
}