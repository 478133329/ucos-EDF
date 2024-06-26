/*
*********************************************************************************************************
*                                               uC/OS-II
*                                         The Real-Time Kernel
*
*                         (c) Copyright 1992-2001, Jean J. Labrosse, Weston, FL
*                                          All Rights Reserved
*
*
*                                       80x86/80x88 Specific code
*                                          PROTECTED MEMORY MODEL
*
*                                          vc++6.0
*
* File         : OS_CPU_C.C
* By           : Jean J. Labrosse
*********************************************************************************************************
*/

#define  OS_CPU_GLOBALS
#include "includes.h"

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either OSTaskCreate() or OSTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*			   在console下没有中断调用，应此也没有中断返回地址，vc编译后的保护模式下的段地址没有变的
*				应此也不用压栈，简单起见，我也没有把浮点寄存器压栈
*				
* Arguments  : task          is a pointer to the task code
*
*              pdata         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If OS_STK_GROWTH is set to 1 then 
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            OS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of OSTaskStkInit().
*                            (see uCOS_II.H for OS_TASK_OPT_???).
*
* Returns    : Always returns the location of the new top-of-stack' once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : Interrupts are enabled when your task starts executing. You can change this by setting the
*              PSW to 0x0002 instead.  In this case, interrupts would be disabled upon task startup.  The
*              application code would be responsible for enabling interrupts at the beginning of the task
*              code.  You will need to modify OSTaskIdle() and OSTaskStat() so that they enable 
*              interrupts.  Failure to do this will make your system crash!
*********************************************************************************************************
*/

OS_STK *OSTaskStkInit (void (*task)(void *pd), void *pdata, OS_STK *ptos, INT16U opt)
{
    INT32U *stk;							//console 下寄存器为32位宽


    opt    = opt;                           /* 'opt' is not used, prevent warning                      */
    stk    = (INT32U *)ptos;                /* Load stack pointer                                      */
    *--stk = (INT32U)pdata;         /* Simulate call to function with argument                 */                                    
	*--stk = (INT32U)0X00000000;	
	*--stk = (INT32U)task;          /* Put pointer to task   on top of stack                   */
    *--stk = (INT32U)0x00000202;				/* EFL = 0X00000202												*/
	*--stk = (INT32U)0xAAAAAAAA;                /* EAX = 0xAAAAAAAA                                              */
    *--stk = (INT32U)0xCCCCCCCC;                /* ECX = 0xCCCCCCCC                                             */
    *--stk = (INT32U)0xDDDDDDDD;                /* EDX = 0xDDDDDDDD                                             */
    *--stk = (INT32U)0xBBBBBBBB;                /* EBX = 0xBBBBBBBB                                             */
    *--stk = (INT32U)0x00000000;                /* ESP = 0x00000000  esp可以任意，因为                                           */
    *--stk = (INT32U)0x11111111;                /* EBP = 0x11111111                                             */
    *--stk = (INT32U)0x22222222;                /* ESI = 0x22222222                                             */
    *--stk = (INT32U)0x33333333;                /* EDI = 0x33333333                                             */
                             
    return ((OS_STK *)stk);
}



/**********************************************************************************************************
;                                          START MULTITASKING
;                                       void OSStartHighRdy(void)
;
; The stack frame is assumed to look as follows:
;
; OSTCBHighRdy->OSTCBStkPtr --> EDI                               (Low memory)                           
;                               ESI
;                               EBP
;                               ESP
;                               EBX
;                               EDX
;                               ECX
;                               EAX
;                               Flags to load in PSW
;                               OFFSET  of task code address
;								4 empty byte
;                               OFFSET  of 'pdata'					(High memory)
;                                              
;
; Note : OSStartHighRdy() MUST:
;           a) Call OSTaskSwHook() then,
;           b) Set OSRunning to TRUE,
;           c) Switch to the highest priority task.
;*********************************************************************************************************/

void OSStartHighRdy(void)
{
	OSTaskSwHook();
	OSRunning = TRUE;
	_asm{
		mov ebx, [OSTCBCur]	;OSTCBCur结构的第一个参数就是esp
		mov esp, [ebx]		;恢复堆栈

		popad		;恢复所有通用寄存器，共8个
		popfd		;恢复标志寄存器
		ret			;ret 指令相当于pop eip 但保护模式下不容许使用eip
		;永远都不返回
	}
}

 /*$PAGE*/
/*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From task level)
;                                           void OSCtxSw(void)
;
; Note(s): 1) 此函数为手动任务切换函数
;
;          2) The stack frame of the task to suspend looks as follows:
;
;                 SP -> OFFSET  of task to suspend    (Low memory)
;                       PSW     of task to suspend    (High memory)
;
;          3) The stack frame of the task to resume looks as follows:
;
; OSTCBHighRdy->OSTCBStkPtr --> EDI                               (Low memory)                           
;                               ESI
;                               EBP
;                               ESP
;                               EBX
;                               EDX
;                               ECX
;                               EAX
;                               Flags to load in PSW
;                               OFFSET  of task code address				(High memory)
;                                    
;*********************************************************************************************************/

void OSCtxSw(void)
{

	_asm{
		lea	 eax, nextstart	;任务切换回来后从nextstart开始
		push eax
		pushfd				;标志寄存器的值
		pushad				;保存EAX -- EDI		
		mov ebx, [OSTCBCur]
		mov [ebx], esp		;把堆栈入口的地址保存到当前TCB结构中
	}

	OSTaskSwHook();
	OSTCBCur = OSTCBHighRdy;	
	OSPrioCur = OSPrioHighRdy;

	_asm{
		mov ebx, [OSTCBCur]
		mov esp, [ebx]		;得到OSTCBHighRdy的esp
		
		popad				;恢复所有通用寄存器，共8个
		popfd				;恢复标志寄存器
		ret					;跳转到指定任务运行
	}
nextstart:			//任务切换回来的运行地址
		return;
}

/*********************************************************************************************************
;                                PERFORM A CONTEXT SWITCH (From an ISR)
;                                        void OSIntCtxSw(void)
;
; Note(s): 1) Context保存了主线程所以的上下文，因此切换起来比较简单，先保存相应寄存器
			  然后把要运行的任务的上下文填入Context结构并保存，完成切换，堆栈内容如下	：
;
; OSTCBHighRdy->OSTCBStkPtr --> EDI                               (Low memory)                           
;                               ESI
;                               EBP
;                               ESP
;                               EBX
;                               EDX
;                               ECX
;                               EAX
;                               Flags to load in PSW
;                               OFFSET  of task code address				(High memory)
;*********************************************************************************************************/
extern CONTEXT Context;
extern HANDLE mainhandle;

void OSIntCtxSw(void)
{
	OS_STK *sp;
	OSTaskSwHook();

	
	sp = (OS_STK *)Context.Esp;	//得到主线程当前堆栈指针
	//在堆栈中保存相应寄存器。
	*--sp = Context.Eip;	//先保存eip
	*--sp = Context.EFlags;	//保存efl
	*--sp = Context.Eax;
	*--sp = Context.Ecx;
	*--sp = Context.Edx;
	*--sp = Context.Ebx;
	*--sp = Context.Esp;	//此时保存的esp是错误的，但OSTCBCur保存了正确的
	*--sp = Context.Ebp;
	*--sp = Context.Esi;
	*--sp = Context.Edi;	
	OSTCBCur->OSTCBStkPtr = (OS_STK *)sp;	//保存当前esp
	
	OSTCBCur = OSTCBHighRdy;		//得到当前就绪最高优先级任务的tcb
	OSPrioCur = OSPrioHighRdy;		//得到当前就绪任务最高优先级
	sp = OSTCBHighRdy->OSTCBStkPtr;	//得到重新执行的任务的堆栈指针
	
	//恢复所有处理器的寄存器
	Context.Edi = *sp++;
	Context.Esi = *sp++;
	Context.Ebp = *sp++;
	Context.Esp = *sp++;		//此时上下文中得到的esp是不正确的
	Context.Ebx = *sp++;
	Context.Edx = *sp++;
	Context.Ecx = *sp++;
	Context.Eax = *sp++;
	Context.EFlags = *sp++; 
	Context.Eip = *sp++;
	
	Context.Esp = (unsigned long)sp;		//得到正确的esp
	
	SetThreadContext(mainhandle, &Context);	//保存主线程上下文
}

/*********************************************************************************************************
;                                            HANDLE TICK ISR
;
; Description:  此函数为时钟调度关键程序，通过timeSetEvent函数来产生时钟节拍，timeSetEvent将产生一个
				线程调用此函数，此函数将与主任务同步运行，应此并不是中断主程序才来运行此函数的
				但此函数将模拟中断的产生，如果发现中断处于关闭状态，则退出
;
; Arguments  : none
;
; Returns    : none
;
; Note(s)    :  此函数与中断函数最不同的是没有立即保存寄存器，而且不能用iret指令，所以OSIntCtxSw()实现也不一样
				它是要返回调用函数的
;*********************************************************************************************************/

void __stdcall OSTickISRuser(unsigned int a,unsigned int b,unsigned long c,unsigned long d,unsigned long e)
{
	if(!FlagEn)
		return;	//如果当前中断被屏蔽则返回

	SuspendThread(mainhandle);		//中止主线程的运行，模拟中断产生.但没有保存寄存器
	GetThreadContext(mainhandle, &Context);	//得到主线程上下文，为切换任务做准备

	OSIntNesting++;
	if (OSIntNesting == 1) {
		OSTCBCur->OSTCBStkPtr = (OS_STK *)Context.Esp;	//保存当前esp
	}	
	OSTimeTick();		//ucos内部定时
	OSIntExit();		//由于不能使用中断返回指令，所以此函数是要返回的

	ResumeThread(mainhandle);	//模拟中断返回，主线程得以继续执行

}
/*$PAGE*/
#if OS_CPU_HOOKS_EN
/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookBegin (void)
{
}
#endif

/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookEnd (void)
{
}
#endif


/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the 
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void OSTaskSwHook (void)
{
}

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your 
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
void OSTaskStatHook (void)
{
	//printf("CPU利用率=%d\n",OSCPUUsage);
}
void   OSTaskReturnHook(OS_TCB          *ptcb)
{

}
#if OS_DEBUG_EN > 0u
void          OSDebugInit             (void)
{
}
#endif
/*
*********************************************************************************************************
*                                           OSTCBInit() HOOK
*
* Description: This function is called by OSTCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.打印定时信息
				
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
void OSTimeTickHook (void)
{

//	_log("   tasktick\n");

}


/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do  
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none 
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION >= 251
void OSTaskIdleHook (void)
{
	//Sleep(1);		
}
#endif
#endif
