/*
for book:Ƕ��ʽʵʱ����ϵͳ��COS������ʵ��
¬����
2011�ڳɶ����ӿƼ���ѧ


*/
//#define ERRORCOMMAND    255

//#define MaxLenComBuf	100

//INT8U CommandAnalys(char *Buf);
#include    "ucos_ii.h"
OS_EVENT  *MyEventSem;

int FirstTask(void *pParam);
void E2_task1(void *pParam);
void E2_task2(void *pParam);
void E3_task0(void *pParam);
void E3_task1(void *pParam);
void E3_task2(void *pParam);


void usertask(void *pParam);
void usertask2(void *pParam);
void usertask1(void *pParam);

void UserTaskSemA(void *pParam);

void UserTaskSemB(void *pParam);

void UserTaskSemC(void *pParam);

void TaskDataProcess(void *pParam);
void TaskIO1(void *pParam);
void TaskIO2(void *pParam);
void TaskIO3(void *pParam);
void TaskIO4(void *pParam);

/* */
void TaskMutex1(void *pParam);
void TaskMutex2(void *pParam);
void TaskPrint(void *pParam);

/*��Ϣ*/
void TaskMessageSen(void *pParam);
void TaskMessageRec(void *pParam);

void TaskQSen(void *pParam);
void TaskQRec(void *pParam);

void TaskM(void *pParam);

void mytestA(void* pParam);
void mytestB(void* pParam);
void mytestC(void* pParam);
void mytestD(void* pParam);