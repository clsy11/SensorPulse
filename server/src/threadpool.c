#include "threadpool.h"

#define CHANGETHREAD 2

ThreadPool* threadpool_create(int max, int min, int taskqcap){
    int i =0;
    ThreadPool* pool = (ThreadPool*)malloc(sizeof(ThreadPool));
    do{
        if(pool == NULL){
            printf("malloc threadpool failed");
            break;
        }

        pool->workid = (pthread_t*)malloc(sizeof(pthread_t)*max);
        if(pool->workid == NULL){
            printf("malloc workid failed");
            break;
        }
        memset(pool->workid,0,sizeof(sizeof(pthread_t)*max));

        pool->threadmin = min;
        pool->threadmax = max;
        pool->threadbusy = 0;
        pool->threadlive = min;
        pool->threadexit = 0;

        if(pthread_mutex_init(&pool->mutexpool,NULL) != 0   ||
           pthread_mutex_init(&pool->mutexthrbsy,NULL) != 0 ||
           pthread_cond_init(&pool->nempty,NULL) != 0        ||
           pthread_cond_init(&pool->nfull,NULL) != 0)
        {
            printf("mutex cond init failed");
            break;
        }

        pool->taskq = (Task*)malloc(sizeof(Task)*pool->taskqcap);
        if(pool->taskq == NULL){
            printf("malloc taskq failed");

        }

        pool->taskqcap = taskqcap;
        pool->taskqsize = 0;
        pool->taskqfront = 0;
        pool->taskqrear = 0;

        pool->pooldelflags = 0;

        pthread_create(&pool->mangerid,NULL,manger,pool);

        for(i = 0; i < min; i++){
            pthread_create(&pool->workid[i],NULL,worker,pool);
        }

        return pool;

    }while(0);
    if(pool->workid) free(pool->workid);
    if(pool->taskq) free(pool->taskq);

} 

//添加任务
    void threadpool_add(ThreadPool* pool,void*(function)(void*),void*arg){
        pthread_mutex_lock(&pool->mutexpool);
        while(pool->taskqsize == pool->taskqcap && !pool->pooldelflags){
            pthread_cond_wait(&pool->nfull,&pool->mutexpool);
        }
        if(pool->pooldelflags){
            pthread_mutex_unlock(&pool->mutexpool);
            return;
        }

        pool->taskq[pool->taskqrear].function = function;
        pool->taskq[pool->taskqrear].arg = arg;
        pool->taskqrear = (pool->taskqrear+1)%(pool->taskqcap);
        pool->taskqsize++;
        pthread_cond_signal(&pool->nempty);

        pthread_mutex_unlock(&pool->mutexpool);
    }

//获取忙线程数
    int threadpoolthrbsy(ThreadPool* pool){
        int busynum = 0;
        pthread_mutex_lock(&pool->mutexthrbsy);
        busynum = pool->threadbusy;
        pthread_mutex_unlock(&pool->mutexthrbsy);
        return busynum;
    }
//获取存活线程数
    int threadpoolthrliv(ThreadPool* pool){
        int livenum = 0;
        pthread_mutex_lock(&pool->mutexpool);
        livenum = pool->threadlive;
        pthread_mutex_unlock(&pool->mutexpool);
        return livenum;
    }

//线程池销毁
int threadpool_destory(ThreadPool* pool){
    int i = 0;
    if(pool == NULL){
        return -1;
    }
    pool->pooldelflags = 1;
    pthread_join(pool->mangerid,NULL);
    pthread_cond_broadcast(&pool->nempty);
    for(i = 0; i < pool->threadlive; i++){
        if(pool->workid[i] != 0){
            pthread_join(pool->workid[i],NULL);
        }
    }

    if(pool->taskq) free(pool->taskq);
    if(pool->workid) free(pool->workid);
    

    pthread_mutex_destroy(&pool->mutexpool);
    pthread_mutex_destroy(&pool->mutexthrbsy);
    pthread_cond_destroy(&pool->nempty);
    pthread_cond_destroy(&pool->nfull);
    
    free(pool);
    pool = NULL;
    return 0;
    
}
void* worker(void* arg){
    ThreadPool* pool = (ThreadPool*)arg;
    while(1){
        pthread_mutex_lock(&pool->mutexpool);
        while(!pool->taskqsize  && !pool->pooldelflags){
            pthread_cond_wait(&pool->nempty,&pool->mutexpool);

            if(pool->threadexit > 0){
                pool->threadexit--;
                    if(pool->threadlive > pool->threadmin){
                        pool->threadlive--;
                        pthread_mutex_unlock(&pool->mutexpool);
                        threadexit(pool);
                    }
                
            }
        }
        if(pool->pooldelflags && pool->taskqsize == 0){
            pthread_mutex_unlock(&pool->mutexpool);
            threadexit(pool);
        }

        Task task;
        task.function = pool->taskq[pool->taskqfront].function;
        task.arg = pool->taskq[pool->taskqfront].arg;
        pool->taskqfront = (pool->taskqfront+1)%(pool->taskqcap);
        pool->taskqsize--;
        pthread_cond_signal(&pool->nfull);
        pthread_mutex_unlock(&pool->mutexpool);

        pthread_mutex_lock(&pool->mutexthrbsy);
        pool->threadbusy++;
        pthread_mutex_unlock(&pool->mutexthrbsy);

        task.function(task.arg);
        

        pthread_mutex_lock(&pool->mutexthrbsy);
        pool->threadbusy--;
        pthread_mutex_unlock(&pool->mutexthrbsy);
    }
}

void* manger(void* arg){
    ThreadPool* pool = (ThreadPool*)arg;
    int i = 0;
    
    while(1){
        sleep(3);
        int cnt = 0;
        pthread_mutex_lock(&pool->mutexpool);
        int threadbusy = pool->threadbusy;
        int taskqsize = pool->taskqsize;
        int threadlive = pool->threadlive;
        

        if(!pool->pooldelflags){
            //添加线程
            if(threadbusy < taskqsize && threadlive < pool->threadmax){
             
                for(i = 0; i < pool->threadmax && cnt < CHANGETHREAD && threadlive < pool->threadmax;i++){
                    if(pool->workid[i] == 0){
                        pthread_create(&pool->workid[i],NULL,worker,pool);
                        pool->threadlive++;
                        cnt++;
                    }
                }
            }

            //销毁线程
            if(threadbusy*2 < threadlive && threadlive > pool->threadmin){
                
                pool->threadexit = CHANGETHREAD;
               
                for(i = 0; i < CHANGETHREAD ;i++){
                    //不能在这里去销毁线程，不知道销毁哪一个，可以去唤醒那些阻塞的线程它是没事的，去自我销毁
                    pthread_cond_signal(&pool->nempty);
                }
            }
        }
     pthread_mutex_unlock(&pool->mutexpool);
    }
}

void threadexit(ThreadPool* arg){
    int i = 0;
    for(i = 0; i < arg->threadmax; i++){
        if(arg->workid[i] == pthread_self()){
            arg->workid[i] = 0;
            break;
        }
        
    }
    pthread_exit(NULL);
}