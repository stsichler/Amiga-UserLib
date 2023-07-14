/* User.lib - 13-1-94 ®SSi   -  TimeHandle.c -------------------------------- */

/* verwenden den Datentyp "TIME" */

#include <user/user.h>
#include <proto/exec.h>
#include <devices/timer.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

/* Private Prototypen ------------------------------------------------------- */

void BreakRequest(TIME *);
TIME *_OpenTime(void __asm (*code)(register __a1 void *),void *data, ULONG unit);

/* Programm ----------------------------------------------------------------- */

TIME *OpenTimeInterrupt(void __asm (*code)(register __a1 void *),void *data)
{
	if (!code) return(0);
	return(_OpenTime(code,data,UNIT_VBLANK));
}

TIME *OpenTime(void)
{
	return(_OpenTime(0,0,UNIT_VBLANK));
}

TIME *OpenTimeExactly(void)
{
	return(_OpenTime(0,0,UNIT_MICROHZ));
}

TIME *_OpenTime(void __asm (*code)(register __a1 void *),void *data, ULONG unit)
{								
	TIME *th;			/* öffnet einen Handler */
	UBYTE SigBit;	/* RETURN: TH oder 0 bei Fehler */
	struct Interrupt *is=0;
	
	if ((SigBit=AllocSignal(-1l))==~0) return(0);
	if (code)
	{
		if (!(is=AllocMem(sizeof(struct Interrupt),MEMF_CLEAR)))
		{
			FreeSignal(SigBit);
			return(0);
		}
		is->is_Code=(void(*)())code;
		is->is_Data=data;
	}
	
	if (!(th=(TIME *)AllocMem(sizeof(TIME),MEMF_CLEAR)))
	{
		if (code) FreeMem(is,sizeof(struct Interrupt));
		FreeSignal(SigBit);
		return(0);
	}
	th->ReplyPort.mp_Node.ln_Type=NT_MSGPORT;
	th->ReplyPort.mp_Flags=PA_SIGNAL;
	th->ReplyPort.mp_SigBit=SigBit;
	th->ReplyPort.mp_SigTask=(struct Task *)FindTask(0);
	NewList(&th->ReplyPort.mp_MsgList);
	th->ReplyPort.mp_MsgList.lh_Type=NT_MESSAGE;
	
	if (OpenDevice(TIMERNAME,unit,(struct IORequest *)&th->Request,0))
	{
		if (code) FreeMem(is,sizeof(struct Interrupt));
		FreeSignal(SigBit);
		FreeMem(th,sizeof(TIME));
		return(0);
	}

	th->Request.tr_node.io_Message.mn_Node.ln_Type=NT_MESSAGE;
	th->Request.tr_node.io_Message.mn_Length=sizeof(struct timerequest)-sizeof(struct Message);
	th->Request.tr_node.io_Message.mn_ReplyPort=&th->ReplyPort;
	
	PutMsg(&th->ReplyPort,&th->Request.tr_node.io_Message);
	WaitPort(&th->ReplyPort);
	if (code)
	{
		Forbid();
		th->ReplyPort.mp_SigTask=(struct Task *)is;
		th->ReplyPort.mp_Flags=PA_SOFTINT;
		Permit();
	}
	return(th);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void CloseTime(TIME *th)	/* schließt einen Handler */
{
	if (!th) return;

	BreakRequest(th);

	GetMsg(&th->ReplyPort);	
	CloseDevice((struct IORequest *)&th->Request);
	FreeSignal(th->ReplyPort.mp_SigBit);
	if (th->ReplyPort.mp_Flags==PA_SOFTINT) FreeMem(th->ReplyPort.mp_SigTask,sizeof(struct Interrupt));
	FreeMem(th,sizeof(TIME));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void StartTime(TIME *th,ULONG secs) { StartTimeExactly(th,secs,0); }

void StartTimeExactly(TIME *th,ULONG secs,ULONG mics)	/* startet den Timer */
{												/* mit der angeg. Zeit */
	if (!th) return;

	BreakRequest(th);

	th->Request.tr_node.io_Command=TR_ADDREQUEST;
	th->Request.tr_time.tv_secs=secs;
	th->Request.tr_time.tv_micro=mics;

	GetMsg(&th->ReplyPort);	
	SendIO((struct IORequest *)&th->Request);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int CheckTime(TIME *th)	/* überprüft auf Abgelaufen */
{									/* RETURN: 0 - nein / !=0 ja */
	if (!th) return(-1);
	
	if (IsListEmpty(&th->ReplyPort.mp_MsgList)) return(0);
	else return(1);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void WaitTime(TIME *th)	/* wartet auf Abgelaufen */
{											/* bei Interrupt-Timer sofortige Rückkehr */
	if (!th) return;
	if (th->ReplyPort.mp_Flags==PA_SIGNAL) WaitPort(&th->ReplyPort);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void StopTime(TIME *th)	/* bricht den Timer ab */
{
	if (!th) return;

	BreakRequest(th);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void BreakRequest(TIME *th)	/* bricht den derz. Timer ab */
{	
	if (IsListEmpty(&th->ReplyPort.mp_MsgList))/* Nachricht noch nicht zurück ? */
	{
		if (th->ReplyPort.mp_Flags==PA_SOFTINT)
		{
			void *is;
			is=(void *)th->ReplyPort.mp_SigTask;
			Forbid();
			th->ReplyPort.mp_Flags=PA_SIGNAL;
			th->ReplyPort.mp_SigTask=(struct Task *)FindTask(0);
			Permit();
			AbortIO((struct IORequest *)&th->Request);
			WaitPort(&th->ReplyPort);
			Forbid();
			th->ReplyPort.mp_SigTask=(struct Task *)is;
			th->ReplyPort.mp_Flags=PA_SOFTINT;
			Permit();
		}
		else
		{
			AbortIO((struct IORequest *)&th->Request);
			WaitPort(&th->ReplyPort);
		}
	}
}
