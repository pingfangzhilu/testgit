

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


#include "base/tools.h"
#include "netInter.h"

void pasreInputCmd(const char *inputCmd)
{
	if(!strcmp(inputCmd,"bro")){
		updateNetwork();
	}else if(!strcmp(inputCmd,"url")){
		mplayerPlayUrl("http://fdfs.xmcdn.com/group9/M08/A1/4A/wKgDZldzNWzRoXyzACZofeFKKKc093.mp3");
	}else if(!strcmp(inputCmd,"pause")){
		mplayerPause();
	}else if(!strcmp(inputCmd,"stop")){
		mplayerStop();
	}else if(!strcmp(inputCmd,"play")){
		mplayerPlay();
	}else if(!strcmp(inputCmd,"add")){
		AddVol();
	}else if(!strcmp(inputCmd,"sub")){
		SubVol();
	}else if(!strcmp(inputCmd,"open")){
		openHostTime("08:00");
	}else if(!strcmp(inputCmd,"close")){
		closeHostTime("18:00");
	}else if(!strcmp(inputCmd,"lock")){
		lockHost();
	}else if(!strcmp(inputCmd,"unlock")){
		unlockHost();
	}else if(!strcmp(inputCmd,"q")){
		cleanSystem();
		exit(0);
	}
}

void test_networkEvent(int type,char *msg,int size)
{
	printf("recv msg =%s\n",msg);
}

int main(void)
{
	initSystem(test_networkEvent);
	init_interface(pasreInputCmd);
	while(1)sleep(10);
	return 0;
}
