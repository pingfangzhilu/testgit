#ifndef _NETINTTER_H
#define _NETINTTER_H


extern int updateNetwork(void);
extern int initSystem(void networkEvent(int type,char *msg,int size));
extern void cleanSystem(void);

extern int openHostTime(char *time);
extern int closeHostTime(char *time);


extern int lockHost(void);
extern int unlockHost(void);

extern int SubVol(void);
extern int AddVol(void);
extern int SetVol_Data(int data);
extern int mplayerGetState(void);
extern int mplayerPause(void);
extern int mplayerStop(void);
extern int mplayerPlay(void);
extern int mplayerPlayUrl(char *url);

extern int versionUpdate(int state);
extern int testDevices(void);
extern int testHostQtts(char *text);
#endif
