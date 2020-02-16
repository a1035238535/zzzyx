#ifndef THREAD_POOL
#define THREAD_POOL

#include<pthread.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#define true  1
#define false 0
#define DEFAULT_TIME 1
#define DEFAULT_THREAD_VARY 3

typedef struct {
    void *(*function) (void*) ;
    void *arg;
} threadpool_task_t;

typedef struct {
    pthread_mutex_t lock;   //锁住本结构体
    pthread_mutex_t thread_counter;     //记录忙状态的锁
    pthread_cond_t queue_not_full;  //任务队列满时
    pthread_cond_t queue_not_empty; //任务队列空时

    pthread_t *threads;   //线程id
    pthread_t  adjust_tid; //管理线程的线程pid
    threadpool_task_t *task_queue; //任务队列

    int min_thr_num;    //线程最小线程数
    int max_thr_num;   //线程最大线程数
    int live_thr_num;     //当前存活线程数
    int busy_thr_num;  //忙状态线程数
    int wait_exit_thr_num;  //要销毁的线程数

    int queue_front;
    int queue_rear;
    int queue_size;
    int queue_max;

    int shutdown;
}thread_pool_t;

thread_pool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size);
void *threadpool_thread(void *threadpool);
void *adjust_thread(void *threadpool);
int threadpool_add(thread_pool_t *pool,void*(*function)(void*arg),void *arg);
int threadpool_destroy(thread_pool_t *pool);
int threadpool_free(thread_pool_t *pool);

#endif
