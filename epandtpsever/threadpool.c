#include"threadpool.h"

//创建线程池  参数 最小线程数 最大线程数  最大队列数
thread_pool_t *threadpool_create(int min_thr_num,int max_thr_num,int queue_max_size) {
    int i;
    thread_pool_t *pool = NULL;
    do {
        if((pool = (thread_pool_t *)malloc(sizeof(thread_pool_t)))==NULL) {
            printf("malloc threadpool fail\n");
            break;
        }
        pool->min_thr_num = min_thr_num;
        pool->max_thr_num = max_thr_num;
        pool->busy_thr_num = 0;
        pool->live_thr_num = min_thr_num;
        pool->queue_max = queue_max_size;
        pool->queue_size = 0;
        pool ->queue_front =0 ;
        pool ->queue_rear = 0;
        pool ->shutdown = false;

        //根据线程数开辟空间并清零
        pool->threads = (pthread_t*)malloc(sizeof(pthread_t)*max_thr_num);
        if(pool->threads == NULL) {
            printf("malloc threads fail\n");
        }
        memset (pool->threads,0,sizeof(pthread_t)*max_thr_num);

        //初始化任务队列
        pool->task_queue = (threadpool_task_t*)malloc(sizeof(threadpool_task_t)*queue_max_size);
        if(pool->task_queue == NULL) {
            printf("malloc task_queue fail\n");
            break;
        }
        memset (pool->task_queue,0,sizeof(threadpool_task_t)*queue_max_size);

        //初始化锁和条件变量
        if(pthread_mutex_init(&(pool->lock),NULL)!=0
            || pthread_mutex_init(&(pool->thread_counter),NULL)!=0
            || pthread_cond_init(&(pool->queue_not_empty),NULL) !=0
            || pthread_cond_init(&(pool->queue_not_full),NULL)!=0) {
                printf("init the lock or cond fail\n");
                break;
            }
            //创建最小个数工作线程
            for(int i=0;i<min_thr_num;i++) {
                pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void *)pool);
                printf("start thread 0x%x...\n", (unsigned int) pool -> threads[i]);
            }
            pthread_create(&(pool->adjust_tid),NULL,adjust_thread,(void *)pool);

            return pool;

    }while(0);
    //如果有调用失败，释放线程池空间
    threadpool_destroy(pool);
}

void *threadpool_thread(void *threadpool) {
    thread_pool_t *pool = (thread_pool_t *) threadpool;
    threadpool_task_t task;
    while(1) {
        //上锁等条件变量
        pthread_mutex_lock(&(pool->lock));

        //队列为空表示没有任务，调用wait等条件变量
        while((pool->queue_size==0) && (!pool->shutdown)) {
            printf("thread 0x%x is waiting\n",(unsigned int)pthread_self());
            pthread_cond_wait(&(pool->queue_not_empty),&(pool->lock));

            //清除空闲线程
            if(pool->wait_exit_thr_num>0) {
                    pool->wait_exit_thr_num--;
                    //线程池里个数大于最小值时可以结束当前线程
                    if(pool->live_thr_num > pool->min_thr_num) {
                        printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
                        pool->live_thr_num--;
                        pthread_mutex_unlock(&(pool->lock));
                        pthread_exit(NULL);
                    }
            }
        }
        //shutdown 为true 关闭线程池子
        if(pool->shutdown) {
            printf("thread 0x%x is exiting\n",(unsigned int)pthread_self());
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }
        //获取任务
        task.function = pool->task_queue[pool->queue_front].function;
        task.arg = pool->task_queue[pool->queue_front].arg;
        pool->queue_front = (pool->queue_front+1) % pool->queue_max;
        pool -> queue_size--;
        //唤醒队列不满条件变量
        pthread_cond_signal(&(pool->queue_not_full));
        //任务获取后释放线程锁
        pthread_mutex_unlock(&(pool->lock));
        printf("thread 0x%x start work\n",(unsigned int )pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num ++;
        pthread_mutex_unlock(&(pool->thread_counter));
        //执行函数
        task.function(task.arg);
        //完成任务
        printf("thread 0x%x end work\n",(unsigned int )pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num --;
        pthread_mutex_unlock(&(pool->thread_counter));
    }
    pthread_exit(NULL);
}

int threadpool_add(thread_pool_t *pool,void*(*function)(void*arg),void *arg) {
    pthread_mutex_lock(&(pool->lock));
    // 任务队列满时 调用wait阻塞
    while((pool->queue_size ==pool->queue_max)&&(!pool->shutdown))
        pthread_cond_wait(&(pool->queue_not_full),&(pool->lock));
    if(pool->shutdown)
        pthread_mutex_unlock(&(pool->lock));

    //添加任务
    pool->task_queue[pool->queue_rear].function =function;
    pool->task_queue[pool->queue_rear].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max;
    pool->queue_size++;
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));
    //    printf("add suc\n");
    return 0;
}

void *adjust_thread(void *threadpool) {
    int i;
    thread_pool_t *pool = (thread_pool_t *)threadpool;
    while(!pool->shutdown) {
        sleep(DEFAULT_TIME);    //每隔指定时间检测一次
        //取出相关数据
        pthread_mutex_lock(&(pool->lock));
        int queue_size = pool->queue_size;
        int live_thr_num =pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        int busy_thr_num= pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));
        if(queue_size >= pool->min_thr_num &&  live_thr_num < pool->max_thr_num) {
        //printf("adjust add thread\n\n\n\n");
            pthread_mutex_lock(&(pool->lock));
            int add=0;
            for(i=0;i<pool->max_thr_num && add<DEFAULT_THREAD_VARY
                &&pool->live_thr_num<pool->max_thr_num;i++) {
                    if(pool->threads[i]==0) {
                        pthread_create(&(pool->threads[i]),NULL,threadpool_thread,(void*)pool);
                        pool->live_thr_num++;
                        add++;
                    }
                }
                pthread_mutex_unlock(&(pool->lock));
        }

        if((busy_thr_num*2) < live_thr_num && live_thr_num>pool->min_thr_num) {
            //printf("adjust close thread\n\n\n\n");
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;
            pthread_mutex_unlock(&(pool->lock));
            for(i=0;i<DEFAULT_THREAD_VARY;i++) {
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }
    return NULL;
}

int threadpool_destroy(thread_pool_t *pool){
    int i;
    if (pool==NULL) {
        return -1;
    }
    pool->shutdown = true;
    pthread_join(pool->adjust_tid,NULL) ;
    for(i = 0 ; i<pool->live_thr_num;i++) {
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    for(i=0;i<pool->live_thr_num;i++) {
        pthread_join(pool->threads[i],NULL);
    }
    threadpool_free(pool);
}

int threadpool_free(thread_pool_t *pool) {
    if(pool == NULL)
        return -1;
    if(pool->task_queue) {
        free(pool->task_queue);
    }
    if(pool->threads) {
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));
        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool = NULL;
    return 0;
}

// void *process(void *arg) {
//     int* num = (int *) arg;
//     printf("thread 0x%x working on task %d\n",(unsigned int)pthread_self(),*num );
//     sleep(1);
//     printf("task %d is end\n", *num);
//     return NULL;
// }

// int main(){
//     thread_pool_t *thp = threadpool_create(3,10,10);
//     printf("pool inied\n");
//     int num[20],i;
//     for(int i=0;i<20;i++) {
//         num[i] =i;
//         printf("add task %d\n",i);
//         threadpool_add(thp,process,(void*)&num[i]);
//     }
//     sleep(5);
//     threadpool_destroy(thp);
//     return 0;
// }
