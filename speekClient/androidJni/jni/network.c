#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <linux/tcp.h>

#include <assert.h>
#include <linux/netfilter_ipv4.h>

#include "base/udp_sock.h"
#include "base/cJSON.h"
#include "base/pool.h"
#include "base/demo_tcp.h"
#include "base/queWorkCond.h"

#define NET_DBG(fmt, args...)	printf("%s: "fmt,__func__, ## args)

typedef struct{
	unsigned char quit;
	char serverip[16];
	int port;
	int sockfd;
	int maxsock;
	void (*networkEvent)(int type,char *msg,int size);
	int broSock; 
	struct sockaddr_in broaddr;	
	WorkQueue *Nlist;
}NetServer_t;

static NetServer_t *Net;

static int __sendto(char *msg,int size,struct sockaddr_in *dest)
{	
	if(sendto(Net->broSock, msg,size , 0,(struct sockaddr*)dest, sizeof(struct sockaddr))<0)
	{
    	return -1;
	}
    return 0;
}
int SendBro(char *buf,int size)
{
        return __sendto(buf, size,&Net->broaddr);
}

int sendMsg(int *sockfd,char *msg,int size)
{
	int ret = send(*sockfd,msg,size,0);
	if(ret==-1){
		NET_DBG("send msg failed \n");
	}else if(ret ==0){
		if(*sockfd>0)
			close(*sockfd);
		*sockfd=-1;
	}else{
		ret=0;
	}
	return ret;

}
static int handler_CtrlMsg(int sockfd,char *recvdata,int size)
{
	cJSON * pJson = cJSON_Parse(recvdata);
	if(NULL == pJson){
		return -1;
	}
	cJSON * pSub = cJSON_GetObjectItem(pJson, "handler"); 
	if(NULL == pSub){
		printf("get json data  failed\n");
		goto exit;
	}
	if(!strcmp(pSub->valuestring,"brocast")){	  // --------------------> brocast 广播地址
		NET_DBG("brocast msg :%s \n",recvdata);
		char *status = cJSON_GetObjectItem(pJson, "status")->valuestring;
		if(!strcmp(status,"ok")){
			char *ip =cJSON_GetObjectItem(pJson, "ip")->valuestring;
			int port = cJSON_GetObjectItem(pJson, "port")->valueint;
			sprintf(Net->serverip,"%s",ip);
			Net->port = port;
			if(Net->sockfd>0){
				close(Net->sockfd);
			}
			Net->sockfd = create_client(Net->serverip,Net->port);
			if(Net->sockfd<0){
				NET_DBG("connect server failed \n");
			}
		}
	}else if(!strcmp(pSub->valuestring,"host")){     // --------------------> host 定时开关机
		Net->networkEvent(0x01,recvdata,size);
	}else if(!strcmp(pSub->valuestring,"setvol")){   // --------------------> setvol 设置声音
		Net->networkEvent(0x01,recvdata,size);
	}else if(!strcmp(pSub->valuestring,"setlock")){  // --------------------> setlock 设备锁
		Net->networkEvent(0x01,recvdata,size);
	}else if(!strcmp(pSub->valuestring,"mplayer")){  // --------------------> mplayer 播放器状态
		Net->networkEvent(0x01,recvdata,size);
	}else if(!strcmp(pSub->valuestring,"all")){      // --------------------> all  登录获取所有消息
		Net->networkEvent(0x01,recvdata,size);
	}
exit:
	cJSON_Delete(pJson);
	return 0;
}


typedef struct{
	char buf[1500];
	int sockfd;
}NetMsg_t;
static void AddNetMsg(int sockfd,char *rbuf,int size)
{
	NetMsg_t *msg = (NetMsg_t *)calloc(1,sizeof(NetMsg_t));
	if(msg==NULL)
	{
		return ;
	}
	msg->sockfd = sockfd;
	memcpy(msg->buf,rbuf,size);
	putMsgQueue(Net->Nlist,(const char *)msg,size);
}
static void handleMsg(const char * msg,int msgSize)
{
	NetMsg_t *rMsg = (NetMsg_t *)msg;
	handler_CtrlMsg(rMsg->sockfd,rMsg->buf, msgSize);
	free((void *)rMsg);
}

static void *Client(void *arg)
{
	NetServer_t *Net = (NetServer_t *)arg;
	struct timeval tv;
	fd_set fdsr;
	char rbuf[1500]={0};	
	int ret =0;
	struct sockaddr_in peer;
	int  len=sizeof(struct sockaddr),size;
	memset(&tv, 0, sizeof(struct timeval));
	tv.tv_sec = 2;		
	Net->quit=1;
	while(Net->quit){
		FD_ZERO(&fdsr);
		FD_SET(Net->broSock, &fdsr);
		Net->maxsock = Net->broSock;
		tv.tv_sec = 2;	
		tv.tv_usec = 0;  
		if(Net->sockfd!=0){
			FD_SET(Net->sockfd, &fdsr);  
			Net->maxsock = Net->sockfd;
		}
		ret = select(Net->maxsock + 1, &fdsr, NULL, NULL, &tv);  
		if (ret < 0){  
			perror("select error ");  
			break;	
		}
		else if (ret == 0){  
			continue;  
		}  
		if (FD_ISSET(Net->broSock, &fdsr)){  
			printf("select live broSock Net->broSock =%d\n",Net->broSock);
			if((size = recvfrom(Net->broSock, rbuf, 1500, 0, (struct sockaddr*)&peer, (socklen_t *)&len))<=0)
			{
				perror("recvfrom broadcast failed");
				usleep(100);
			}
			AddNetMsg(Net->broSock,rbuf, size);
		}
		if (FD_ISSET(Net->sockfd, &fdsr)){
			if((size = recv(Net->sockfd, rbuf, 1500, 0))<=0)
			{
				perror("recv failed");
				usleep(100);
			}
			AddNetMsg(Net->sockfd,rbuf, size);
		}
	}
	Net->quit=2;
	return  NULL;
}
int initSystem(void networkEvent(int type,char *msg,int size))
{
	Net= (NetServer_t *)calloc(1,sizeof(NetServer_t));
	if(Net==NULL)
	{
		NET_DBG("calloc failed \n");
		return -1;
	}
	Net->networkEvent = networkEvent;
	Net->broSock = create_client_brocast(&Net->broaddr,20001);
	if(Net->broSock==-1){
		NET_DBG("create_client_brocast failed \n");
		goto exit0;
	}
	Net->Nlist = InitCondWorkPthread(handleMsg);
	if(Net->Nlist==NULL)
	{
		goto exit1;
	}
	Net->networkEvent = networkEvent;
	if(pthread_create_attr(Client,(void *)Net))
	{
		NET_DBG("pthread_create_attr failed \n");
		goto exit2;
	}
	return 0;
exit2:
	free(Net->Nlist);
exit1:
	close(Net->broSock);	
exit0:
	free(Net);	
	return -1;
}

void cleanSystem(void)
{
	int timeout=0;
	if(Net)
	{
		Net->quit=0;
		close(Net->broSock);
		Net->broSock=-1;
		while(++timeout<20)
		{
			usleep(10000);
			if(Net->quit!=0)
				break;
		}
		CleanCondWorkPthread(Net->Nlist,handleMsg);
		free(Net);
	}
}

/*-----------------------------------------------------------------------------------*/
static void getServerIp(void)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int wsize=0;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "brocast");
	cJSON_AddStringToObject(pItem, "ip", "null");
	cJSON_AddNumberToObject(pItem, "port", 0);
	cJSON_AddStringToObject(pItem, "status","unkown");
	
	szJSON = cJSON_Print(pItem);		
	wsize = strlen(szJSON);
	printf("%s \nwsize =%d\n",szJSON,wsize);
	SendBro(szJSON,wsize);
	cJSON_Delete(pItem);
}

/*
static int GetDevicesState(void)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int wsize=0,ret=-1;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "state");
	cJSON_AddStringToObject(pItem, "status","unkown");
	szJSON = cJSON_Print(pItem);		
	wsize = strlen(szJSON);
	printf("%s \nwsize =%d\n",szJSON,wsize);
	ret =sendMsg(&Net->sockfd,szJSON,wsize);
	cJSON_Delete(pItem);
	return ret;
}
*/
void updateNetwork(void)
{
	//GetDevicesState();
	getServerIp();
}
static  int SetDevicesTime(char *time,char *type)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int wsize=0,ret=-1;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "host");	
	cJSON_AddStringToObject(pItem, "type", type);
	cJSON_AddStringToObject(pItem, "time", time);
	cJSON_AddStringToObject(pItem, "status","unkown");
		
	szJSON = cJSON_Print(pItem);		
	wsize = strlen(szJSON);
	NET_DBG("%s \nwsize =%d\n",szJSON,wsize);
	ret =sendMsg(&Net->sockfd,szJSON,wsize);
	cJSON_Delete(pItem);
	return ret;
}
int openHostTime(char *time)
{
	return SetDevicesTime(time,"open");
}
int closeHostTime(char *time)
{
	return SetDevicesTime(time,"close");
}

static int __setlock(int lock)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int wsize=0,ret=-1;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "lock");	
	cJSON_AddNumberToObject(pItem, "state", lock);
	cJSON_AddStringToObject(pItem, "status","unkown");
	
	szJSON = cJSON_Print(pItem);		
	wsize = strlen(szJSON);
	NET_DBG("%s \nwsize =%d\n",szJSON,wsize);
	ret =sendMsg(&Net->sockfd,szJSON,wsize);
	cJSON_Delete(pItem);
	return ret;

}
int lockHost(void)
{
	return __setlock(1);
}
int unlockHost(void)
{
	return __setlock(0);
}
/*-----------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------------*/

static int setVol(char *set)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int wsize=0,ret=-1;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "setvol");	
	cJSON_AddStringToObject(pItem, "vol", set);
	cJSON_AddStringToObject(pItem, "status","unkown");
	
	szJSON = cJSON_Print(pItem);		
	wsize = strlen(szJSON);
	printf("%s \nwsize =%d\n",szJSON,wsize);
	ret =sendMsg(&Net->sockfd,szJSON,wsize);
	cJSON_Delete(pItem);
	return ret;
}

int AddVol(void)
{
	return setVol("add");
}
int SubVol(void)
{
	return setVol("sub");
}

static int setMplayerState(char *state,char *url)
{
	char* szJSON = NULL;
	cJSON* pItem = NULL;
	int wsize=0,ret=-1;
	pItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pItem, "handler", "mplayer");	
	cJSON_AddStringToObject(pItem, "state", state);
	cJSON_AddStringToObject(pItem, "url", url);
	cJSON_AddStringToObject(pItem, "status","unkown");
	
	szJSON = cJSON_Print(pItem);		
	wsize = strlen(szJSON);
	NET_DBG("%s \nwsize =%d\n",szJSON,wsize);
	ret =sendMsg(&Net->sockfd,szJSON,wsize);
	cJSON_Delete(pItem);
	return ret;
}

int mplayerPause(void)
{
	return setMplayerState("pause","null");
}
int mplayerStop(void)
{
	return setMplayerState("stop","null");
}
int mplayerPlay(void)
{
	return setMplayerState("play","null");
}
int mplayerPlayUrl(char *url)
{
	return setMplayerState("next",url);
}


/*-----------------------------------------------------------------------------------*/











