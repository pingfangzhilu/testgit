#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

#include "base/pool.h"
#include "event.h"

typedef struct{
	unsigned char quit;
	int eventNum;
	Event_t head;
	void (*timeout_cb)(Event_t *ev);
	pthread_mutex_t Evmutex;
}MsgEv_t;
static MsgEv_t *msgEv=NULL;

int AddEvent(int event,int time){
	Event_t *ev = (Event_t *)calloc(1,sizeof(Event_t));
	if(ev==NULL){
		return -1;
	}
	ev->event = event;
	ev->time = time;
	
	pthread_mutex_lock(&msgEv->Evmutex);
	msgEv->eventNum++;
	list_add(&(ev->list),&(msgEv->head.list));
	pthread_mutex_unlock(&msgEv->Evmutex);
	
}

void removeEvent(int event)
{
	struct list_head *pos=NULL;
	Event_t *ev=NULL;
	pthread_mutex_lock(&msgEv->Evmutex);
	list_for_each(pos,&(msgEv->head.list))
	{
		ev=list_entry(pos,Event_t,list);
		if(ev->event==event){
			msgEv->eventNum--;
			list_del(&ev->list);
			free(ev);
			break;
		}
		printf("removeEvent  event=%d \n",ev->event);
		
	}
	pthread_mutex_unlock(&msgEv->Evmutex);

}

static void timeoutEvent(Event_t *ev)
{
	msgEv->timeout_cb(ev);
	list_del(&ev->list);
	free(ev);
}

static void destoryEvent(void){
	struct list_head *pos=NULL;
	Event_t *ev=NULL;
	pthread_mutex_lock(&msgEv->Evmutex);
	list_for_each(pos,&(msgEv->head.list))
	{
		ev=list_entry(pos,Event_t,list);
		list_del(&ev->list);
		printf("destoryEvent  event=%d \n",ev->event);
		free(ev);
	}
	pthread_mutex_unlock(&msgEv->Evmutex);

}
static void *WorkEvent(void *arg)
{
	int i=0;
	struct list_head *pos=NULL;
	Event_t *p=NULL;
	while(!msgEv->quit)
	{
		usleep(200000);
		pthread_mutex_lock(&msgEv->Evmutex);
		list_for_each(pos,&(msgEv->head.list))
		{
			p=list_entry(pos,Event_t,list);
			p->time--;
			printf("msgEv->eventNum =%d \n",msgEv->eventNum);
			if(p->time==0){
				timeoutEvent(p);
			}
			
		}
		pthread_mutex_unlock(&msgEv->Evmutex);
	}
	return NULL;
}

int InitWorkEvent(void timeout_cb(Event_t *ev))
{
	msgEv = (MsgEv_t *)calloc(1,sizeof(MsgEv_t));
	if(msgEv==NULL)
	{
		return -1;
	}
	INIT_LIST_HEAD(&(msgEv->head.list));
	pthread_mutex_init(&msgEv->Evmutex, NULL);
	msgEv->timeout_cb = timeout_cb;
	
	if(pthread_create_attr(WorkEvent,NULL))
	{
		printf("pthread_create_attr failed \n");
		return -1;
	}
	return 0;
}
void destoryWorkEvent(void)
{
	if(msgEv){
		msgEv->quit=0;
		destoryEvent();
		free(msgEv);
		msgEv=NULL;
	}
}
#if 0
void timeout_cb(Event_t *ev)
{
	printf("timeout_cb : ev->event %d\n",ev->event);
}
int main(void){
	InitWorkEvent(timeout_cb);
	AddEvent(1,1);
	AddEvent(2,1);
	sleep(10);
	destoryWorkEvent();
	return 0;
}
#endif
