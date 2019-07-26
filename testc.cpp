// testc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
#include "time.h"
#include <process.h>
#define MaxTask   100

typedef struct _task {
	void * task;
	int taskid;
} TaskItem;


typedef struct _p_c {
	int maxTask;	
	CRITICAL_SECTION   taskCS;
	TaskItem tasklist[MaxTask];
	int fullindex;
	int emptyindex;
	int fullNum;
	int testGenTaskNum;
} Pr_Co;

int GetRandom(int min, int max);
void ProFun(void *task, Pr_Co *pc, int randint);
//EnterCriticalSection（& _critical）
//LeaveCriticalSection(& _critical)
//DeleteCriticalSection(& _critical)
void ConsumThread(Pr_Co *pc);
UINT __stdcall ConsumThreadRun(LPVOID lParam)
{
	ConsumThread((Pr_Co *)lParam);
	return 0;
}


 void StartComsumThread(Pr_Co *prc)
{
	HANDLE   hth1;
	unsigned  uiThread1ID;

	hth1 = (HANDLE)_beginthreadex(NULL,         // security  
		0,            // stack size  
		ConsumThreadRun,
		prc,           // arg list  
		CREATE_SUSPENDED,  // so we can later call ResumeThread()  
		&uiThread1ID);
	::ResumeThread(hth1);
}

 UINT __stdcall GenTaskThreadRun(LPVOID lParam)
 {
	 Pr_Co *prc = (Pr_Co*)lParam;
	 if (prc == 0)
	 {
		 return 0;
	 }
	 printf("Gen %d task \n", prc->testGenTaskNum);
	 for (int i = 0; i < prc->testGenTaskNum; i++)
	 {
		 int taskid = GetRandom(1, prc->testGenTaskNum * 10);
		 void *tmptask = (void*)&taskid;//todo no use;
		 ProFun(tmptask, prc, taskid);
	 }

	 return 0;
 }


 void StartGenTaskThread(Pr_Co *prc)
 {
	 HANDLE   hth1;
	 unsigned  uiThread1ID;

	 hth1 = (HANDLE)_beginthreadex(NULL,         // security  
		 0,            // stack size  
		 GenTaskThreadRun,
		 prc,           // arg list  
		 CREATE_SUSPENDED,  // so we can later call ResumeThread()  
		 &uiThread1ID);
	 ::ResumeThread(hth1);
 }
int GetRandom(int min, int max)
{
	int randint = 0;
	randint = min + rand()%(max-min);
	return randint;
}
void InitTaskPrCo(Pr_Co * pc, int testTaskNum)
{
	memset(pc, 0, sizeof(Pr_Co));
	pc->maxTask = MaxTask;
	pc->testGenTaskNum = testTaskNum;
	InitializeCriticalSection(&pc->taskCS);
}

int PreTaskDone(Pr_Co *pc,bool c_p,void **task,int &randint)
{
	int i = 0;
	EnterCriticalSection(&pc->taskCS);
	i = pc->fullNum;
	if (c_p)
	{
		if (i < pc->maxTask)
		{
			pc->fullNum++;
			pc->tasklist[pc->emptyindex].task = *task;
			pc->tasklist[pc->emptyindex].taskid = randint;
			printf("gen the task,taskid:%d\n", randint);
			pc->emptyindex = (pc->emptyindex + 1) % pc->maxTask;
		}
	}
	else
	{
		if (i > 0)
		{
			pc->fullNum--;
			*task = pc->tasklist[pc->fullindex].task;
			randint = pc->tasklist[pc->fullindex].taskid;
			printf("run the task,taskid:%d\n", randint);
			pc->fullindex = (pc->fullindex + 1) % pc->maxTask;
		}
		
	}
	LeaveCriticalSection(&pc->taskCS);
	return i;
}

void ConsumThread(Pr_Co *pc)
{
	while (true)
	{
		void *task;
		int taskid = 0;
		while (PreTaskDone(pc, false, &task, taskid) <= 0)
		{
			Sleep(1);
		}
		//run the task;		
	}
}

void ProFun(void *task, Pr_Co *pc,int randint)
{
	void *ptask = task;
	int trand = randint;
	while (PreTaskDone(pc, true, &ptask, trand) >= pc->maxTask)
	{
		Sleep(1);
	}
	
}
int _tmain(int argc, _TCHAR* argv[])
{
	Pr_Co temppc;
	srand(time(0));
	InitTaskPrCo(&temppc, 1000);		
	StartComsumThread(&temppc);
	//int genThreadNum = temppc.testGenTaskNum / temppc.maxTask;
	//for (int i = 0; i < genThreadNum;i++)
	//	StartGenTaskThread(&temppc);
	StartGenTaskThread(&temppc);
	while (1)
	{
		Sleep(100000);
	}
	return 0;
}


