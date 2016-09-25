#ifndef _EVENT_H
#define _EVENT_H

#include "kernel_list.h"
typedef struct{
	int event;
	int time;
	struct list_head list;
}Event_t;

extern int AddEvent(int event,int time);
extern void removeEvent(int event);

extern int InitWorkEvent(void timeout_cb(Event_t *ev));
extern void destoryWorkEvent(void);

#endif
