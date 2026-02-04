#include <sys/epoll.h>    // epoll_create, epoll_ctl, epoll_wait 的核心头文件
#include <fcntl.h>        // 用于把文件描述符设为非阻塞模式（O_NONBLOCK）
#include <sys/types.h>
#include <sys/socket.h>   // 网络 IO
#include <netinet/in.h>   // 网络地址结构 sockaddr_in
#include <arpa/inet.h>    // IP 地址转换函数
#include "threadpool.h"
#include <errno.h>

struct INFO{
    int epfd;
    int fd;
};


void* comuni(void* arg){
    
    struct INFO* info= (struct INFO*)arg;
    
    short buffer[3];
    char data[15];
    int got_request = 1; // 增加一个标志

    while(1){
        int len = recv(info->fd,data,sizeof(data),0);
        if(len > 0){
            if(got_request){
                int s_fd = open("/dev/ap3216c", O_RDONLY);
                if (s_fd > 0) {
                    read(s_fd,buffer,sizeof(buffer));
                    close(s_fd);
                }
                if(s_fd < 0){
                    printf("open failed\n\r");
                }
            got_request = 0;
            printf("%x",buffer);
            ssize_t s_len = send(info->fd,buffer,sizeof(buffer),0);
            if (s_len < 0) {
                printf("Send failed! errno: %d (%s)\n", errno, strerror(errno));
            } else {
                printf("Send success! Actually sent: %zd bytes\n", s_len);
            }
            }
            
        }
        else if(len == -1){
            // 情况 A: 缓冲区读空了 (正常退出循环)
            if(errno == EAGAIN || errno == EWOULDBLOCK){
            
            
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
                ev.data.fd = info->fd;
                epoll_ctl(info->epfd, EPOLL_CTL_MOD, info->fd, &ev);
                // printf("read done\n"); 
                break;
            }
            // 情况 B: 被信号中断 (继续读，不应该 break)
            if(errno == EINTR){
                continue; 
            }
            // 情况 C: 真正的错误 (比如连接重置)
            perror("recv error"); // 使用 perror 会告诉你到底是 EINTR 还是 ECONNRESET
            close(info->fd);
            break;
        }
        else if(len == 0){
            
                printf("disconnect interrupt!%d\n\r",info->fd);
                epoll_ctl(info->epfd,EPOLL_CTL_DEL,info->fd,NULL);
            
    
            
            close(info->fd);
            break;
        }
        
    }
    
    free(info);
    
}

int main(){
    struct sockaddr_in caddr;
    int clen = sizeof(caddr);
    
    struct sockaddr_in saddr;

    //创建监听套接字
    int lfd = socket(AF_INET,SOCK_STREAM,0);
    if(lfd < 0){
        printf("socket lfd failed!\n\r");
        
    }
    
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(10000);
    
    bind(lfd,(struct sockaddr*)&saddr,sizeof(struct sockaddr_in));

    listen(lfd,128);


    int epfd = epoll_create(1);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;

    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);
    
    struct epoll_event evs[1024];
    int size = sizeof(evs)/sizeof(evs[0]);

    ThreadPool* pool = threadpool_create(10,3,10);
    while(1){
        int i = 0;
        int num = epoll_wait(epfd,evs,size,-1);
        for(i = 0; i < num; i++){
            
            int cfd;
            int fd = evs[i].data.fd;
            //printf("Main Loop: Event on fd %d\n", fd);
            struct INFO* info = malloc(sizeof(struct INFO));
            info->epfd = epfd;
            info->fd = fd;
            if(fd == lfd){
                    int lfd = info->fd;
                    int cfd = accept(lfd,NULL,NULL);
                    if(cfd < 0){
                        printf("connect failed!%d\n\r",lfd);
                    }
                   
                    int flags = fcntl(cfd, F_GETFL, 0);
                    fcntl(cfd, F_SETFL, flags | O_NONBLOCK);
                    struct epoll_event ev;
                    ev.data.fd = cfd;
                    ev.events = EPOLLIN | EPOLLET |EPOLLONESHOT;
                    epoll_ctl(info->epfd,EPOLL_CTL_ADD,cfd,&ev);
                    free(info);
            }
            else{
                threadpool_add(pool,comuni,info);
               
            }
        }
    }
    close(lfd);

    return 0;
}