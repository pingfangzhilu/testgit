#ifndef _NETINTTER_H
#define _NETINTTER_H


#define SEND_MSG_FAILED		0x01	//发送数据失败
#define CONNECT_FAILED		0x02	//连接服务器失败


extern void updateNetwork(void);

extern int initSystem(void networkEvent(int type,char *msg,int size));
extern void cleanSystem(void);

extern int openHostTime(char *time);
extern int closeHostTime(char *time);

extern int lockHost(void);
extern int unlockHost(void);

extern int SubVol(void);
extern int AddVol(void);
extern int mplayerPause(void);
extern int mplayerStop(void);
extern int mplayerPlay(void);
extern int mplayerPlayUrl(char *url);

#endif
