/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*

*********************************************************************************************************
*/
/*
for book:Ƕ��ʽʵʱ����ϵͳ��COSԭ����ʵ��
¬����
2011�ڳɶ����ӿƼ���ѧ
*/
#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE               512       /* Size of each task's stacks (# of WORDs)            */
#define TaskStart_Prio	1
#define Task1_Prio		2

typedef struct {
	INT32S c;	// ����ǰ�����ڻ���Ҫ�ļ���ʱ��
	INT32S C;	// ����ļ���ʱ��
	INT32S t;	// ����ǰ������ʣ���ʱ��
	INT32S T;	// ����ļ�������
	INT32S d;	// ����ľ��Խ�ֹʱ��
	INT32S D;	// �������Խ�ֹʱ��
}TaskExt;

OS_STK  TaskStk[OS_MAX_TASKS][TASK_STK_SIZE];    // Tasks stacks
HANDLE mainhandle;		//���߳̾��
CONTEXT Context;		//���߳��л�������
BOOLEAN FlagEn = 1;		//����һ��ȫ�ֱ�������Ϊ�Ƿ�ʱ�ӵ��ȵı�־

void TaskStart(void * pParam) ;
//void Task1(void * pParam) ;                            /* Function prototypes of tasks                  */

void VCInit(void);						//��ʼ����ر���,һ����Ҫ

void E11_task1(void* pParam);
void E11_task2(void* pParam);
void E11_task3(void* pParam);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

int main(int argc, char **argv)
{
	TaskExt ext1 = { 1, 1, 4, 4, 0, 4 };
	TaskExt ext2 = { 2, 2, 6, 6, 0, 6 };
	TaskExt ext3 = { 3, 3, 8, 8, 0, 8 };
	int p[2],Experiment ;
	p[0]=0;
	p[1]=100;
	VCInit();
    printf("0.û���û�����\n");
    printf("1.��һ�����ӣ�һ���û�����\n");
    printf("2.�ڶ������ӣ�����������CPU��������\n");
    printf("3.���������ӣ�����Ĺ���ͻָ�\n");
    printf("4.���ĸ����ӣ��ź�������\n");
    printf("5.��������ӣ������ź�������\n"); 
    printf("6.���������ӣ��¼���־��\n");
    printf("7.���߸����ӣ���Ϣ����\n");
    printf("8.�ڰ˸����ӣ���Ϣ����\n");
    printf("9.�ھŸ����ӣ��ڴ����\n"); 
	printf("10.��ʮ�����ӣ����ȼ���ת\n");
	printf("11.�ھŸ����ӣ�RM/EDF�㷨\n");

    printf("���������ѡ������:\n");
	scanf("%d",&Experiment);
    if ((Experiment<0)||(Experiment>12))
	{
		printf("��Ч������!");
        return(1); 	
	}
	OSInit();
	OSTaskCreate(TaskStart, 0, &TaskStk[1][TASK_STK_SIZE-1], TaskStart_Prio);
	switch(Experiment) 
	{
		case 1://һ����������
			OSTaskCreate(FirstTask, 0, &TaskStk[5][TASK_STK_SIZE-1], 5);
			break;
		case 2://����������CPU
			OSTaskCreate(E2_task1, 0, &TaskStk[5][TASK_STK_SIZE-1], 5);
			OSTaskCreate(E2_task2, 0, &TaskStk[6][TASK_STK_SIZE-1], 6);
            break;
		case 3://����Ĺ���ͻָ�
			OSTaskCreate(E3_task0, 0, &TaskStk[5][TASK_STK_SIZE-1], 5);
			OSTaskCreate(E3_task1, 0, &TaskStk[6][TASK_STK_SIZE-1], 6);
			OSTaskCreate(E3_task2, 0, &TaskStk[7][TASK_STK_SIZE-1], 7);
            break;
		case 4://�ź�����������
			OSTaskCreate(UserTaskSemA, 0, &TaskStk[5][TASK_STK_SIZE-1], 7);
			OSTaskCreate(UserTaskSemB, 0, &TaskStk[6][TASK_STK_SIZE-1], 6);
			OSTaskCreate(UserTaskSemC, 0, &TaskStk[7][TASK_STK_SIZE-1], 5);
            break;
		case 5://�����ź�����������
			OSTaskCreate(TaskMutex1, 0, &TaskStk[6][TASK_STK_SIZE-1], 6);
			OSTaskCreate(TaskMutex2, 0, &TaskStk[7][TASK_STK_SIZE-1], 50);
			OSTaskCreate(TaskPrint, 0, &TaskStk[8][TASK_STK_SIZE-1], 30);
            break;
		case 6://ʱ���־���������
			OSTaskCreate(TaskDataProcess, 0, &TaskStk[5][TASK_STK_SIZE-1],5);
			OSTaskCreate(TaskIO1, 0, &TaskStk[6][TASK_STK_SIZE-1], 6);
			OSTaskCreate(TaskIO2, 0, &TaskStk[7][TASK_STK_SIZE-1], 7);
			OSTaskCreate(TaskIO3, 0, &TaskStk[8][TASK_STK_SIZE-1], 8);
			OSTaskCreate(TaskIO4, 0, &TaskStk[9][TASK_STK_SIZE-1], 9);
            break;
		case 7://��Ϣ����
			OSTaskCreate(TaskMessageSen, 0, &TaskStk[6][TASK_STK_SIZE-1], 6);
			OSTaskCreate(TaskMessageRec, 0, &TaskStk[7][TASK_STK_SIZE-1], 7);
			break;
		case 8://��Ϣ����
			 OSTaskCreate(TaskQSen, 0, &TaskStk[7][TASK_STK_SIZE-1], 5);
			 OSTaskCreate(TaskQRec, 0, &TaskStk[8][TASK_STK_SIZE-1], 6);
			 OSTaskCreate(TaskQRec, 0, &TaskStk[9][TASK_STK_SIZE-1], 7);
			break;
		case 9://�ڴ����
			 OSTaskCreate(TaskM, 0, &TaskStk[8][TASK_STK_SIZE-1], 6);
			break;
		case 10://���ȼ���ת
			OSTaskCreate(mytestA, 0, &TaskStk[5][TASK_STK_SIZE - 1], 5);
			OSTaskCreate(mytestB, 0, &TaskStk[7][TASK_STK_SIZE - 1], 7);
			OSTaskCreate(mytestC, 0, &TaskStk[9][TASK_STK_SIZE - 1], 9);
			OSTaskCreate(mytestD, 0, &TaskStk[8][TASK_STK_SIZE - 1], 8);
			break;
		case 11://RM/EDF�㷨
			OSTaskCreateExt(E11_task1, 0, &TaskStk[5][TASK_STK_SIZE - 1], 5, 5, &TaskStk[5][0], TASK_STK_SIZE, (void*)&ext1, 0);
			OSTaskCreateExt(E11_task2, 0, &TaskStk[6][TASK_STK_SIZE - 1], 6, 6, &TaskStk[6][0], TASK_STK_SIZE, (void*)&ext2, 0);
			OSTaskCreateExt(E11_task3, 0, &TaskStk[7][TASK_STK_SIZE - 1], 7, 7, &TaskStk[7][0], TASK_STK_SIZE, (void*)&ext3, 0);
			break;
		default:           
			;
	}
 
	OSStart();	       
	return(0);
}

void VCInit(void)
{
	HANDLE cp,ct;
	Context.ContextFlags = CONTEXT_CONTROL;
	cp = GetCurrentProcess();	//�õ���ǰ���̾��
	ct = GetCurrentThread();	//�õ���ǰ�߳�α���
	DuplicateHandle(cp, ct, cp, &mainhandle, 0, TRUE, 2);	//α���ת��,�õ��߳�����
		
}

void TaskStart(void * pParam) 
{	
	char err;	
	OS_EVENT *sem1;
    
	/*ģ�����ö�ʱ���жϡ�����һ����ʱ���߳�,ÿ���ж�100��,�жϷ������OSTickISRuser*/
	timeSetEvent(1000/OS_TICKS_PER_SEC, 0, OSTickISRuser, 0, TIME_PERIODIC);
	OSStatInit(); /*ͳ�������ʼ��*/
	sem1 = OSSemCreate(0); 
	OSSemPend(sem1, 0, &err);   //�ȴ��¼���������������
}

void E11_task1(void* pParam)
{
	printf("����1 c:%d C:%d t:%d T:%d d:%d D:%d\n", ((TaskExt*)OSTCBCur->OSTCBExtPtr)->c, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->C, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->t, 
		((TaskExt*)OSTCBCur->OSTCBExtPtr)->T, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->d, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->D);
	OSTimeDly(10);
	int i = 0, j = 0;
	for (;;)
	{
		for (i = 0; i < 99999999; i++);
		//printf("����1");
	}
}

void E11_task2(void* pParam)
{
	printf("����2 c:%d C:%d t:%d T:%d d:%d D:%d\n", ((TaskExt*)OSTCBCur->OSTCBExtPtr)->c, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->C, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->t,
		((TaskExt*)OSTCBCur->OSTCBExtPtr)->T, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->d, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->D);
	OSTimeDly(10);
	int i = 0, j = 0;
	for (;;)
	{
		for (i = 0; i < 99999999; i++);
		//printf("����2");
	}
}

void E11_task3(void* pParam)
{
	printf("����3 c:%d C:%d t:%d T:%d d:%d D:%d\n", ((TaskExt*)OSTCBCur->OSTCBExtPtr)->c, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->C, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->t,
		((TaskExt*)OSTCBCur->OSTCBExtPtr)->T, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->d, ((TaskExt*)OSTCBCur->OSTCBExtPtr)->D);
	OSTimeDly(10);
	int i = 0, j = 0;
	for (;;)
	{
		for (i = 0; i < 99999999; i++);
		//printf("����3");
	}
}