#include"service_hook.h"
#include"include/stdafx.h"
#include"include/vector.hpp"
#include"include/exclusivity.h"
#include"include/write_protect.h"
#include"kernel-hook/khook/hde/hde.h"
#include"include/handle.h"
#include"include/PDBSDK.h"
#include"common.h"

extern "C"
{
#include"kernel-hook/khook/khook/hk.h"
extern ULONG_PTR KernelBase;
extern ULONG_PTR PspCidTable;
extern ULONG_PTR Win32kfullBase;
extern ULONG Win32kfullSize;
}

struct myUnicodeString : public UNICODE_STRING
{
	~myUnicodeString() {}
};

//
// ��Ҫ���صĴ�������
// ��ServiceHook::Construct��ʹ��
//
myUnicodeString ux64dbg = RTL_CONSTANT_STRING(L"x64dbg");
myUnicodeString uxdbg64 = RTL_CONSTANT_STRING(L"xdbg64");
myUnicodeString ux32dbg = RTL_CONSTANT_STRING(L"x32dbg");
myUnicodeString uxdbg32 = RTL_CONSTANT_STRING(L"xdbg32");
myUnicodeString uCheatEngine73 = RTL_CONSTANT_STRING(L"Cheat Engine 7.3");

//
// ��Ҫ���صĴ�������
//
//




using std::vector; 
vector<ServiceHook> vServcieHook;
hde64s gIns;

static bool init = false;
vector<MM_SESSION_SPACE*> vSesstionSpace;

static vector<myUnicodeString> vHideWindow;

LARGE_INTEGER MmOneSecond = { (ULONG)(-1 * 1000 * 1000 * 10), -1 };
LARGE_INTEGER MmTwentySeconds = { (ULONG)(-20 * 1000 * 1000 * 10), -1 };
LARGE_INTEGER MmShortTime = { (ULONG)(-10 * 1000 * 10), -1 }; // 10 milliseconds
LARGE_INTEGER MmHalfSecond = { (ULONG)(-5 * 100 * 1000 * 10), -1 };
LARGE_INTEGER Mm30Milliseconds = { (ULONG)(-30 * 1000 * 10), -1 };


void ServiceHook::Construct()
{

	if (!pfMiGetSystemRegionType)
		pfMiGetSystemRegionType = (MiGetSystemRegionTypeType)(KernelBase + OffsetMiGetSystemRegionType);

	if (!pfMmGetSessionById)
		pfMmGetSessionById = (MmGetSessionByIdType)(KernelBase + OffsetMmGetSessionById);
	
	//����һ��bool init������ֻ��Ҫһ�εĳ�ʼ������
	if (!init) { //bool init start

		vHideWindow.push_back(ux64dbg);
		vHideWindow.push_back(uxdbg64);
		vHideWindow.push_back(ux32dbg);
		vHideWindow.push_back(uxdbg32);
		vHideWindow.push_back(uCheatEngine73);


#ifndef WIN11
		pfMiAttachSession = (MiAttachSessionType)(KernelBase + OffsetMiAttachSession);
#else
		pfMiAttachSession = (MiAttachSessionType)(KernelBase + OffsetMiAttachSessionGlobal);
#endif
		pfMiDetachProcessFromSession = (MiDetachProcessFromSessionType)(KernelBase + OffsetMiDetachProcessFromSession);

		//
		for (int i = 0; i < 1000; i++)
		{
			PEPROCESS Process = pfMmGetSessionById(i);
			if (Process)
			{
				//EPROCESS
				///*0x400*/     struct _MM_SESSION_SPACE* Session;
				//
				SystemSesstionSpace = *(MM_SESSION_SPACE**)((ULONG_PTR)Process + SessionOffsetFromEprocess);
				vSesstionSpace.push_back(SystemSesstionSpace);

			}
		}
		//



#ifdef DBG
		for (auto session : vSesstionSpace)
		{
			Log("Session ID %d\n", session->SessionId);

			//��ʼ��ַ�ͽ�����ַ������һ��
			Log("PagedPoolStart %p\n", session->PagedPoolStart);
			Log("PagedPoolEnd %p\n", session->PagedPoolEnd);
		}
#endif // DBG








		init = true;
	}// bool init end


#if 0
	for (int i = 8;;)
	{
		//�ų�system���̺�idle����
		PEPROCESS process = (PEPROCESS)GetObject10((PHANDLE_TABLE10)PspCidTable, i);
		//����session
		SystemSesstionSpace = *(MM_SESSION_SPACE**)((ULONG_PTR)process + 0x400);

		//�������session
		if (SystemSesstionSpace) {
			ULONG32 SystemSesstionID = SystemSesstionSpace->SessionId;
			PVOID SystemSesstionPoolStart = SystemSesstionSpace->PagedPoolStart;
			PVOID SystemSesstionPoolEnd = SystemSesstionSpace->PagedPoolEnd;
		}
	}
#endif

	for (auto Session : vSesstionSpace)
	{
		//if (this->fp.GuestVA == PVOID(Win32kfullBase + OffsetNtUserFindWindowEx))
		//{
			//this->isWin32Hook = true;
		//}
		if (this->fp.GuestVA > (PVOID)Win32kfullBase && this->fp.GuestVA < (PVOID)(Win32kfullBase + Win32kfullSize))
		{
			this->isWin32Hook = true;
		}

	}

	if (!this->DetourFunc || !this->TrampolineFunc || !this->fp.GuestVA)
	{
		Log("DetourFunc or TrampolineFunc or fp.GuestVA is null!\n");
		Log("DetourFunc %p\nTrampolineFunc %p\nfp.GuestVA %p\n",
			this->DetourFunc, this->TrampolineFunc, this->fp.GuestVA);
		return;
	}


#if 1
	/**
	* Q:MmGetPhysicalAddress(NtCreateThread)���ص������ַΪ0
	*   MmGetPhysicalAddress(NtCreateThreadEx)���ص������ַ��Ϊ0
	* 
	* A:
	* 1: kd> u ntcreatethread
	nt!NtCreateThread:
	fffff807`1a6948f0 ??              ???


0: kd> !pte fffff807`1a6948f0
										   VA fffff8071a6948f0
PXE at FFFFFCFE7F3F9F80    PPE at FFFFFCFE7F3F00E0    PDE at FFFFFCFE7E01C698    PTE at FFFFFCFC038D34A0
contains 0000000001208063  contains 0000000001209063  contains 0000000001217063  contains 0000FEF300002064
pfn 1208      ---DA--KWEV  pfn 1209      ---DA--KWEV  pfn 1217      ---DA--KWEV  not valid
																				  PageFile:  2
																				  Offset: def3
																				  Protect: 3 - ExecuteRead
	��ʵ����ķ�ҳ�����ִ�����ϵͳ���ڴ�ѹ����

	*/


#endif

	//���ָ����������ҳ�Ŀ�ʼ��
	auto tmp = (PVOID)(((ULONG_PTR)(this->fp).GuestVA >> 12) << 12);
	//GuestPAΪGuestVA���ҳ����ʼ�������ַ
	//GuestVA�����ʼ�����ܸı�
#if 0
	if(this->fp.GuestVA == (PVOID)0xfffff80725d288f0)
	DbgBreakPoint();
#endif
	//
	//���pte.vaildΪ0��MmGetPhysicalAddress����0
	//
	this->fp.GuestPA = MmGetPhysicalAddress(tmp);
#if 1 //�ṩ��ҳ����֧��
	if (!pfMmAccessFault)
		pfMmAccessFault = (MmAccessFaultType)(KernelBase + OffsetMmAccessFault);

	if (!this->fp.GuestPA.QuadPart)
	{

#if 0  //��õ�ǰ��ַ���ڵ�POOL_TYPE
		auto RegionType = pfMiGetSystemRegionType((ULONG_PTR)this->fp.GuestVA);
#endif
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

		//
		//MmAccessFault��ҳ������������������ҳ�濽����ʱ��ϵͳ������ǻ�����
		//MmAccessFault��ʱ�򻹻�����,���绻NtUserFindWindowEx����ҳ
		//
		
		//pfMmAccessFault(PAGE_FAULT_READ, nullptr, this->fp.GuestVA, KernelMode);
		
		//���ṩһ�λ���
		//this->fp.GuestPA = MmGetPhysicalAddress(tmp);
	}
#endif
	this->fp.PageContent = ExAllocatePoolWithQuotaTag(NonPagedPool, PAGE_SIZE,'sbb');

	//����ԭ��ҳ��,������ʱ��ÿ��ҳ������ݶ�һ����д��ʱ���Ҫ�ֱ�д�ˡ�
	if (this->isWin32Hook) {
		pfMiAttachSession(vSesstionSpace[0]);
	}
	memcpy(this->fp.PageContent, tmp, PAGE_SIZE);


	

	this->fp.PageContentPA = MmGetPhysicalAddress(this->fp.PageContent);

	//�ȴ�����ϵͳ�����ǻ�ҳ�꣬�����ڻ�ȡ
	this->fp.GuestPA = MmGetPhysicalAddress(tmp);

	if (!fp.GuestPA.QuadPart || !fp.PageContentPA.QuadPart)
	{
		HYPERPLATFORM_COMMON_DBG_BREAK();
		Log("MmGetPhysicalAddress error %s %d\n",__func__,__LINE__);
		return;
	}

	auto exclusivity = ExclGainExclusivity();
	//
	//
	//mov rax,xx
	//jmp rax
	//
	//
	static char hook[] = { 0x48,0xB8,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0xFF,0xE0 };
	size_t CodeLength = 0;
	while (CodeLength < 12)
	{
		HdeDisassemble((void*)((ULONG_PTR)(this->fp.GuestVA) + CodeLength), &gIns);
		CodeLength += gIns.len;
	}
	this->HookCodeLen = (ULONG)CodeLength;
	/*
	* 1.����һ����̬�ڴ�(Orixxxxx)���溯����ͷ����12���ֽ�,���ü���һ��jmpxxx���ֽڣ���ΪOri����jmp��ȥ
	* 2.Ȼ���޸ĺ�����ͷΪmove rax,xx jump rax,xx
	* 3.
	*/

	*(this->TrampolineFunc) = ExAllocatePoolWithTag(NonPagedPool, CodeLength + 14, 'a');
	if (!*(this->TrampolineFunc))
	{
		Log("ExAllocatePoolWithTag failed ,no memory!\n");
		return;
	}
	
	memcpy(*(this->TrampolineFunc), this->fp.GuestVA, CodeLength);
	static char hook2[] = { 0xff,0x25,0,0,0,0,1,1,1,1,1,1,1,1 };
	ULONG_PTR jmp_return = (ULONG_PTR)this->fp.GuestVA + CodeLength;
	memcpy(hook2 + 6, &jmp_return, 8);
	memcpy((void*)((ULONG_PTR)(*(this->TrampolineFunc)) + CodeLength), hook2, 14);
	auto irql = WPOFFx64();

	PVOID* Ptr = &this->DetourFunc;
	memcpy(hook + 2, Ptr, 8);
	memcpy((PVOID)this->fp.GuestVA, hook, sizeof(hook));

	WPONx64(irql);
	

	ExclReleaseExclusivity(exclusivity);

	if (this->isWin32Hook)
		pfMiDetachProcessFromSession(1);

	this->isEverythignSuc = true;








}

void ServiceHook::Destruct()
{
	if (!this->isEverythignSuc)
		return;

#if 0
	NTSTATUS Status = HkRestoreFunction((this->fp).GuestVA, this->TrampolineFunc);
	if (!NT_SUCCESS(Status)) {
		Log("HkRestoreFunction Failed %x\n", Status);
		return;
	}
#endif

	if (this->isWin32Hook)
		pfMiAttachSession(vSesstionSpace[0]);



	//
	//�ⲿ�ִ���������÷�ҳ���ڴ滻��������������߳��л��ͻ�������
	//
	if(KeGetCurrentIrql()>= DISPATCH_LEVEL)
		KeLowerIrql(APC_LEVEL);
	char tmp[1];
	memcpy(tmp, this->fp.GuestVA, 1);
	//
	//û������ҳ,��������
	//
	if (!MmIsAddressValid(this->fp.GuestVA))
	{
		KeBugCheck(0x11111110);
	}
	//
	auto Exclu = ExclGainExclusivity();
	auto irql = WPOFFx64();
	memcpy(this->fp.GuestVA, *(this->TrampolineFunc), this->HookCodeLen);
	WPONx64(irql);
	ExclReleaseExclusivity(Exclu);

	if (this->isWin32Hook)
		pfMiDetachProcessFromSession(1);

	ExFreePool(*(this->TrampolineFunc));

}

//
// ��ʼhook
//
void AddServiceHook(PVOID HookFuncStart, PVOID Detour, PVOID *TramPoline,const char* funcName)
{
	ServiceHook tmp;
	tmp.DetourFunc = Detour;
	tmp.fp.GuestVA = HookFuncStart;
	tmp.TrampolineFunc = TramPoline;
	tmp.funcName = funcName;
	tmp.Construct();
	vServcieHook.push_back(tmp);
}

//
// ж��hook
//

/*

.text:00000000000066FF                 mov     rax, OriNtAllocateVirtualMemory
.text:0000000000006706                 mov     [rsp+68h+var_18], rax
.text:000000000000670B                 mov     rax, [rsp+68h+var_18]
.text:0000000000006710                 mov     [rsp+68h+var_10], rax
.text:0000000000006715                 mov     eax, [rsp+68h+Protect]
.text:000000000000671C                 mov     dword ptr [rsp+68h+HandleInformation], eax
.text:0000000000006720                 mov     eax, [rsp+68h+AllocationType]
.text:0000000000006727                 mov     dword ptr [rsp+68h+Object], eax
.text:000000000000672B                 mov     r9, [rsp+68h+arg_18]
.text:0000000000006733                 mov     r8, [rsp+68h+arg_10]
.text:000000000000673B                 mov     rdx, [rsp+68h+arg_8]
.text:0000000000006740                 mov     rcx, [rsp+68h+Handle]
.text:0000000000006745                 mov     rax, [rsp+68h+var_10]
.text:000000000000674A                 call    cs:__guard_dispatch_icall_fptr
.text:0000000000006750                 add     rsp, 68h  --->����ط�bugcheck
.text:0000000000006754                 retn

���������������Ϊִ��ԭ����ϵͳ�ĺ�����ʱ������ж���ˣ�������ʱ���Ҳ���������,ά�������ü����ܺõ�

00 ffffdd07`221dade8 fffff807`25732d72     nt!DbgBreakPointWithStatus
01 ffffdd07`221dadf0 fffff807`257324f7     nt!KiBugCheckDebugBreak+0x12
02 ffffdd07`221dae50 fffff807`25655837     nt!KeBugCheck2+0x957
03 ffffdd07`221db570 fffff807`256f97b6     nt!KeBugCheckEx+0x107
04 ffffdd07`221db5b0 fffff807`2556a3d7     nt!MiSystemFault+0x130186
05 ffffdd07`221db6f0 fffff807`25663183     nt!MmAccessFault+0x327
06 ffffdd07`221db890 fffff802`dfe06750     nt!KiPageFault+0x343
07 ffffdd07`221dba20 ffffffff`ffffffff     <Unloaded_HyperTool.sys>+0x6750
08 ffffdd07`221dba28 ffffa38f`068b7080     0xffffffff`ffffffff
09 ffffdd07`221dba30 00000000`00000000     0xffffa38f`068b7080
*/
void RemoveServiceHook()
{


	for (auto& hook : vServcieHook)
	{
		while (hook.refCount != 0)
		{
			Log("ref count is %d\n", hook.refCount);
			KeDelayExecutionThread(KernelMode, false, &MmOneSecond);
		}
		hook.Destruct();
		Log("unload %s success\n", hook.funcName.c_str());
	}
}











//
//hook example
//

NTSTATUS DetourNtOpenProcess(
	PHANDLE            ProcessHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PCLIENT_ID         ClientId
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[0].refCount, 1);
	//user code start














	//usercode end
	Status = OriNtOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
	InterlockedAdd(&vServcieHook[0].refCount, -1);
	return Status;
}

NTSTATUS DetourNtCreateFile(
	PHANDLE            FileHandle,
	ACCESS_MASK        DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK   IoStatusBlock,
	PLARGE_INTEGER     AllocationSize,
	ULONG              FileAttributes,
	ULONG              ShareAccess,
	ULONG              CreateDisposition,
	ULONG              CreateOptions,
	PVOID              EaBuffer,
	ULONG              EaLength
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG

	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[1].refCount, 1);
	//user code start















	//usercode end
	Status = OriNtCreateFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		AllocationSize,
		FileAttributes,
		ShareAccess,
		CreateDisposition,
		CreateOptions,
		EaBuffer,
		EaLength);
	InterlockedAdd(&vServcieHook[1].refCount, -1);
	return Status;
}

//
//�����û�̬�ڴ�д��
//
NTSTATUS DetourNtWriteVirtualMemory(
	IN HANDLE ProcessHandle,
	OUT PVOID BaseAddress,
	IN CONST VOID* Buffer,
	IN SIZE_T BufferSize,
	OUT PSIZE_T NumberOfBytesWritten OPTIONAL
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[2].refCount, 1);
	//user code start

























	//usercode end
	Status = OriNtWriteVirtualMemory(
		ProcessHandle,
		BaseAddress,
		Buffer,
		BufferSize,
		NumberOfBytesWritten);
	InterlockedAdd(&vServcieHook[2].refCount, -1);
	return Status;
}

NTSTATUS DetourNtCreateThreadEx(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID lpStartAddress,
	IN PVOID lpParameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID lpBytesBuffer)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[3].refCount, 1);
	//user code start




















	//usercode end
	Status = OriNtCreateThreadEx(
		hThread,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		lpStartAddress,
		lpParameter,
		Flags,
		StackZeroBits,
		SizeOfStackCommit,
		SizeOfStackReserve,
		lpBytesBuffer);
	InterlockedAdd(&vServcieHook[3].refCount, -1);
	return Status;
}

//����ڴ����
NTSTATUS DetourNtAllocateVirtualMemory(
	HANDLE    ProcessHandle,
	PVOID* BaseAddress,
	ULONG_PTR ZeroBits,
	PSIZE_T   RegionSize,
	ULONG     AllocationType,
	ULONG     Protect
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[4].refCount, 1);
	//user code start













	//usercode end
	Status = OriNtAllocateVirtualMemory(
		ProcessHandle,
		BaseAddress,
		ZeroBits,
		RegionSize,
		AllocationType,
		Protect);
	InterlockedAdd(&vServcieHook[4].refCount, -1);
	return Status;
}


NTSTATUS DetourNtCreateThread(
	OUT PHANDLE ThreadHandle,
	IN  ACCESS_MASK DesiredAccess,
	IN  POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
	IN  HANDLE ProcessHandle,
	OUT PCLIENT_ID ClientId,
	IN  PCONTEXT ThreadContext,
	IN  PVOID InitialTeb,
	IN  BOOLEAN CreateSuspended
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[5].refCount, 1);
	//user code start















	//usercode end
	Status = OriNtCreateThread(
		ThreadHandle,
		DesiredAccess,
		ObjectAttributes,
		ProcessHandle,
		ClientId,
		ThreadContext,
		InitialTeb,
		CreateSuspended
	);
	InterlockedAdd(&vServcieHook[5].refCount,-1);
	return Status;
}
/*
hook win32k ������ע������

00 ffffdd07`2186d780 fffff807`255490fb     nt!MiAttachSession+0x6c
01 ffffdd07`2186d7d0 fffff807`255c7675     nt!MiTrimOrAgeWorkingSet+0x77b
02 ffffdd07`2186d8b0 fffff807`255c6056     nt!MiProcessWorkingSets+0x255
03 ffffdd07`2186da60 fffff807`25620c17     nt!MiWorkingSetManager+0xaa
04 ffffdd07`2186db20 fffff807`254dfa45     nt!KeBalanceSetManager+0x147
05 ffffdd07`2186dc10 fffff807`2565cb8c     nt!PspSystemThreadStartup+0x55
06 ffffdd07`2186dc60 00000000`00000000     nt!KiStartSystemThread+0x1c


*/

//
//����Ҫ��ClassName��һ�¹��ˣ��������ò��������̫����
//
HWND DetourNtUserFindWindowEx(  // API FindWindowA/W, FindWindowExA/W
	IN HWND hwndParent,
	IN HWND hwndChild,
	IN PUNICODE_STRING pstrClassName,
	IN PUNICODE_STRING pstrWindowName)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	HWND hwnd;

	//user code start

	for (auto window : vHideWindow) {
		if (!RtlCompareUnicodeString(pstrWindowName, &window, 1))
			return 0;
	}



	//user code end


	hwnd = OriNtUserFindWindowEx(hwndParent, hwndChild, pstrClassName, pstrWindowName);
	return hwnd;
}

/*
2022.1.29
������������ж�ز��ˣ������û��,��ʱ�������������



*/
NTSTATUS DetourNtDeviceIoControlFile(
	_In_ HANDLE FileHandle,
	_In_opt_ HANDLE Event,
	_In_opt_ PIO_APC_ROUTINE ApcRoutine,
	_In_opt_ PVOID ApcContext,
	_Out_ PIO_STATUS_BLOCK IoStatusBlock,
	_In_ ULONG IoControlCode,
	_In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_writes_bytes_opt_(OutputBufferLength) PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength
)
{
#ifdef DBG
	static int once = 0;
	if (!(once++))
		Log("[HOOK_LOG]%s\n", __func__);
#endif // DBG
	NTSTATUS Status;
	InterlockedAdd(&vServcieHook[6].refCount, 1);
	//user code start





















	//usercode end
	Status = OriNtDeviceIoControlFile(FileHandle, Event, ApcRoutine, ApcContext, IoStatusBlock, IoControlCode, InputBuffer,
		InputBufferLength, OutputBuffer, OutputBufferLength);
	InterlockedAdd(&vServcieHook[6].refCount, -1);
	return Status;

}