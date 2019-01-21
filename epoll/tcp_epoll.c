/**
 * 基于TCP高并发简单服务端聊天程序(epoll)
 * epoll_create 创建 
 * epoll_ctl 添加/删除事件
 * epoll_wait 开始监控等待事件是否就绪
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

int recv_data(int sockfd, char buf, int len)
{
  int total_len = 0;
  int read_len = 0;
  while(1)
  {
    read_len = recv(sockfd, buf + total_len, len - total_len, 0);
    if(read_len <= 0){
      if(errno == EINTR || errno == EAGAIN)
        continue;
      return -1;
    }
    if(read_len < (len -total_len)){
      break;
    }
    total_len += read_len;
  }
}

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    printf("Usage ./tcp_epoll ip port");
    return 0;
  }
  //TCP服务端程序
  int lis_fd,cli_fd,epfd;
  int ret,i,nfds;
  socklen_t len;
  struct sockaddr_in lis_addr;
  struct sockaddr_in cli_addr;
  struct epoll_event evs[1024],ev;

  lis_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(lis_fd < 0){
    perror("socket error");
    return -1;
  }
  lis_addr.sin_family = AF_INET;
  lis_addr.sin_port = htons(atoi(argv[2]));
  lis_addr.sin_addr.s_addr = inet_addr(argv[1]);
  len = sizeof(struct sockaddr_in);
  ret = bind(lis_fd, (struct sockaddr*)&lis_fd, len);

  if(listen(lis_fd, 5) < 0){
    perror("listen error");
    return -1;
  }

  //1.创建epoll
  epfd = epoll_create(1024);
  if(epfd < 0){
    perror("epoll_create error");
    return -1;
  }
  //2.定义一个事件结点
  ev.events = EPOLLIN;//可读事件
  ev.data.fd = lis_fd;
  //3.向内核直接添加事件
  epoll_ctl(epfd, EPOLL_CTL_ADD, lis_fd, &ev);
  while(1){
    //4.开始epoll监控
    nfds = epoll_wait(epfd, evs, 1024, 3000);//3s
    if(nfds < 0){
      perror("epoll_wait error");
      continue;
    }else if(nfds == 0){
      printf("timeout!\n");
      continue;
    }
    for(i = 0; i< nfds; i++)
    {
      //监听描述符事件,这个有新的连接可以接受
      if(evs[i].data.fd == lis_fd)
      {
        cli_fd = accept(lis_fd, (struct sockaddr*)&cli_addr, &len);
        if(cli_fd <0){
          continue;
        }
        printf("new connect\n");
        //接收新连接，给新连接定义事件，然后添加到epoll的监控集合中
        ev.events = EPOLLIN;//可读事件
        //EPOLLET代表边缘触发
        ev.data.fd = lis_fd | EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_ADD, cli_fd, &ev);
      }
      //如果就绪的事件结点的描述符不是监听描述符
      //并且这个事件是可读事件，说明存在数据可读
      else if(evs[i].events & EPOLLIN)
      {
        char buf[1024]={0};
        ret = recv(evs[i].data.fd, buf, 1023, 0 );
        if(ret <= 0){
          //描述符出错，关闭
          //从epoll集合中移除一个事件结点
          epoll_ctl(epfd, EPOLL_CTL_DEL, evs[i].data.fd, &ev);
          close(evs[i].data.fd);
        }
        printf("clinet say:%s\n", buf);
        send(evs[i].data.fd, "??", 6,0);
      }
    }
  }
  return 0;
}
