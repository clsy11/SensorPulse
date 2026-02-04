#ifndef  _THREADPOOL_H
#define _THREADPOOL_H



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

typedef struct Task{
    void* (*function)(void* );
    void * arg;
}Task;

typedef struct ThreadPool{
    //任务队列
    struct Task* taskq;             //任务队列
    int taskqcap;                   //任务队列容量
    int taskqsize;                  //任务队列大小
    int taskqfront;                 //任务队列头
    int taskqrear;                  //任务队列尾

    pthread_t mangerid;             //管理者线程id
    pthread_t*  workid;             //消费者线程id
    int threadmax;                  //最大线程数
    int threadmin;                  //最小线程数
    int threadbusy;                 //忙线程
    int threadlive;                 //存活线程
    int threadexit;                 //销毁线程数

    pthread_mutex_t mutexpool;      //线程池所                                                                   
    pthread_mutex_t mutexthrbsy;    //忙线程数所      
    pthread_cond_t  nfull;          //任务队列非空条件变量
    pthread_cond_t  nempty;         //任务队列非满条件变量   

    int pooldelflags;               //线程池销毁标志


}ThreadPool;

//创建并初始化线程池
    ThreadPool* threadpool_create(int max, int min, int taskqcap);
//销毁注册线程池
    int threadpool_destory(ThreadPool* pool);
//给线程池任务队列添加任务
    void threadpool_add(ThreadPool* pool,void*(function)(void*),void*arg);
//获取此刻忙线程数量
    int threadpoolthrbsy(ThreadPool* pool);
//获取存活线程数量
    int threadpoolthrliv(ThreadPool* pool);
    
    
    
    void* worker(void* arg);
    void* manger(void* arg);
    void  threadexit(ThreadPool* arg);

#endif 
