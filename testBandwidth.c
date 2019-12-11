#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <x86intrin.h>

#include "latency.h"
#include "common.h"

#include <linux/string.h>

#include <time.h>

#include <sys/file.h>

#define NUM_DIMM 6

#define   LOCK_SH   1    /* shared lock */
#define   LOCK_EX   2    /* exclusive lock */
#define   LOCK_NB   4    /* don't block when locking */
#define   LOCK_UN   8    /* unlock */

int sid = 0;
char *map1 = NULL, *map2 = NULL, *map3 = NULL, *map4 = NULL, *map5 = NULL, *map6 = NULL;
//pthread_mutex_t lock;

char *map_arr[6];

int fd1, fd2, fd3, fd4, fd5, fd6;

uint64_t ftotal=0; //for total cycle


void *log_append(void *num_of_log_to_write)
{

    double t0, t1, time;
    char msg[20];

    char *map; //for mapping
    int fd;
    int n=(int *)num_of_log_to_write;
    uint64_t ft=0;


    //pthread_mutex_lock(&lock);

    //int lock = flock(fd, LOCK_SH);
    //printf("I am sid %d and map %ld\n", sid, map);


    //memcpy(map, msg, strlen(msg));


    //t0 = sec();

    for(int i=0; i < n; i++){
        __sync_fetch_and_add(&sid, 1); //atomic increment
        map = map_arr[sid%6] + (sid/6) * 64;
        // ft=store_64byte_clflushopt(map)*;
        ft=store_64byte_clflushopt(map);
        //printf("hi");
        //ft=store_64byte_clflushopt(map);
        ftotal=ftotal+ft;

        //msync(map, strlen(msg) , MS_SYNC);
        //_mm_clflush(msg);
        //_mm_clflushopt(msg);
        //_mm_sfence();
    }

    return NULL;

}


int main()
{

    int dimm_id = 0;
    int num_thread = 30;
    int num_dimm = 6;
    double time;
    double t0, t1;
    double total=0.0; //total bytes of msg
////Init file and mmap
    char *filepath1, *filepath2, *filepath3, *filepath4, *filepath5, *filepath6;
    size_t total_log_count = 120000;
    int num_of_logs_per_thread = total_log_count / num_thread;

    filepath1 = "/mnt/mem1/log.txt";
    filepath2 = "/mnt/mem2/log.txt";
    filepath3 = "/mnt/mem3/log.txt";
    filepath4 = "/mnt/mem4/log.txt";
    filepath5 = "/mnt/mem5/log.txt";
    filepath6 = "/mnt/mem6/log.txt";



    int fd1 = open(filepath1, O_RDWR | O_CREAT | O_APPEND, (mode_t)0600);
    int fd2 = open(filepath2, O_RDWR | O_CREAT | O_APPEND, (mode_t)0600);
    int fd3 = open(filepath3, O_RDWR | O_CREAT | O_APPEND, (mode_t)0600);
    int fd4 = open(filepath4, O_RDWR | O_CREAT | O_APPEND, (mode_t)0600);
    int fd5 = open(filepath5, O_RDWR | O_CREAT | O_APPEND, (mode_t)0600);
    int fd6 = open(filepath6, O_RDWR | O_CREAT | O_APPEND, (mode_t)0600);


    if (fd1 == -1 || fd2 == -1 || fd3 == -1 || fd4 == -1 || fd5 == -1 || fd6 == -1)
    {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }


///mmap ing all 6 files

    map1 = mmap(0, 10485760, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    map2 = mmap(0, 10485760, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    map3 = mmap(0, 10485760, PROT_READ | PROT_WRITE, MAP_SHARED, fd3, 0);
    map4 = mmap(0, 10485760, PROT_READ | PROT_WRITE, MAP_SHARED, fd4, 0);
    map5 = mmap(0, 10485760, PROT_READ | PROT_WRITE, MAP_SHARED, fd5, 0);
    map6 = mmap(0, 10485760, PROT_READ | PROT_WRITE, MAP_SHARED, fd6, 0);

    if (map1 == MAP_FAILED || map2 == MAP_FAILED || map3 == MAP_FAILED || map4 == MAP_FAILED || map5 == MAP_FAILED || map6 == MAP_FAILED)
    {
        close(fd1);
        close(fd2);
        close(fd3);
        close(fd4);
        close(fd5);
        close(fd6);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }


  map_arr[0]=map1;
  map_arr[1]=map2;
  map_arr[2]=map3;
  map_arr[3]=map4;
  map_arr[4]=map5;
  map_arr[5]=map6;
   // if (pthread_mutex_init(&lock, NULL) != 0) {
   //      printf("\n mutex init has failed\n");
   //      return 1;
   //  }


    /* this variable is our reference to the second thread */
    pthread_t log_threads[num_thread];

    //t0 = sec();

    for (int i = 0; i < num_thread; ++i)
    {
        pthread_create( &log_threads[i], NULL, log_append,(void*)num_of_logs_per_thread);
        printf("thread created\n");
    }

    //t1 = sec();

    for (int i = 0; i < num_thread; ++i)
    {
        if(pthread_join( log_threads[i], NULL)){
            printf("Failed to join%d\n", i);
            exit(0);
        }
    }

  // printf("total cycle count: %llu\n", ftotal);
   time = ftotal / (1000.186 * 1000000); //sec
  // printf("Time (sec): %f\n", time);
   printf("Throughput is %f[MiB/s] for %d threads \n", (64 * num_of_logs_per_thread / (time * 1024 * 1024)), num_thread);
  // ftotal = ftotal / total_log_count;
  // printf("My average clock cycle  %llu\n", ftotal);


    //printf("Total %f\n", total);
    //printf("Throughput is %f[MiB/s] for %d threads \n", total / ((t1-t0) * 1024 * 1024), num_thread);

    //pthread_mutex_destroy(&lock);

    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);
    close(fd5);
    close(fd6);

    return 0;
}
