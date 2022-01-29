#include"include/exclusivity.h"
#include"ia32_type.h"
#include"systemcall.h"
#include"include/write_protect.h"
#include "settings.h"

extern "C"
{
#include"kernel-hook/khook/khook/hk.h"
extern "C" void DetourKiSystemServiceStart();
NTSYSAPI const char* PsGetProcessImageFileName(PEPROCESS Process);
}


fpSystemCall SystemCallFake;

char SystemCallRecoverCode[15] = {};
NTSTATUS HookStatus = STATUS_UNSUCCESSFUL;


const char* GetSyscallProcess()
{
	return PsGetProcessImageFileName(IoGetCurrentProcess());
}


//copy from blackbone
PKLDR_DATA_TABLE_ENTRY GetSystemModule(IN PUNICODE_STRING pName, IN PVOID pAddress)
{
	if ((pName == NULL && pAddress == NULL) || PsLoadedModuleList == NULL)
		return NULL;

	// No images
	if (IsListEmpty(PsLoadedModuleList))
		return NULL;

	// Search in PsLoadedModuleList
	for (PLIST_ENTRY pListEntry = PsLoadedModuleList->Flink; pListEntry != PsLoadedModuleList; pListEntry = pListEntry->Flink)
	{
		PKLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(pListEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		// Check by name or by address
		if ((pName && RtlCompareUnicodeString(&pEntry->BaseDllName, pName, TRUE) == 0) ||
			(pAddress && pAddress >= pEntry->DllBase && (PUCHAR)pAddress < (PUCHAR)pEntry->DllBase + pEntry->SizeOfImage))
		{
			return pEntry;
		}
	}

	return NULL;
}


NTSTATUS InitSystemVar()
{
	/*
	* �������������vmlaunch֮ǰ��ʼ��һЩȫ�ֱ���
	* 
	* 1.����ں˻�ַ
	* 
	* 2.���KiSystemServiceStart�ĵ�ַ
	* 
	* 3.���SSDT Table�ĵ�ַ
	* 
	* 4.��ʼ��һ��α���guestҳ��
	*/


	KernelBase = GetKernelBase();

	PsLoadedModuleList = (decltype(PsLoadedModuleList))(KernelBase + OffsetPsLoadedModuleList);

	auto tmpa = GetSystemModule(&Win32kfullBaseString,0);
	if (tmpa)
	{
		Win32kfullBase = (ULONG_PTR)tmpa->DllBase;
		Win32kfullSize = (ULONG_PTR)tmpa->SizeOfImage;
#ifdef DBG
		Log("[WIN32kfullBase]%llx\n", Win32kfullBase);
#endif // DBG

	}
	else
	{
#ifdef DBG
		DbgBreakPoint();
#else
		KeBugCheck(0xbbbbbbbb);
#endif
	}

	tmpa = GetSystemModule(&Win32kbaseBaseString, 0);
	if (tmpa)
	{
		Win32kbaseBase = (ULONG_PTR)tmpa->DllBase;
#ifdef DBG
		Log("[WIN32kbaseBase]%llx\n", Win32kbaseBase);
#endif // DBG
	}
	else
	{
#ifdef DBG
		DbgBreakPoint();
#else
		KeBugCheck(0xcccccccc);
#endif
	}


	if (!KernelBase)
		KeBugCheck(0xaaaaaaaa);
	//LdrpKrnGetDataTableEntry = (LdrpKrnGetDataTableEntryType)(KernelBase + OffsetLdrpKrnGetDataTableEntry);

	PtrKiSystemServiceStart = (ULONG_PTR)&DetourKiSystemServiceStart;

	//KiSystemServiceCopyStart = OffsetKiSystemServiceCopyStart + KernelBase;
	KiSystemServiceStart = OffsetKiSystemServiceStart + KernelBase;

	aSYSTEM_SERVICE_DESCRIPTOR_TABLE = 
	(SYSTEM_SERVICE_DESCRIPTOR_TABLE*)(OffsetKeServiceDescriptorTable + KernelBase);

	PspCidTable = *(ULONG_PTR*)(KernelBase + OffsetPspCidTable);

#ifdef HOOK_SYSCALL

	SystemCallFake.Construct();

#endif // HOOK_SYSCALL


	return STATUS_SUCCESS;
}

void DoSystemCallHook()
{
	OriKiSystemServiceStart = (PVOID)((ULONG_PTR)KiSystemServiceStart + 0x14);
	auto exclusivity = ExclGainExclusivity();
	//
	//push r15
	//mov r15,xx
	//jmp r15
	// 
	//r15:pop r15
	//
	char hook[] = { 0x41,0x57,0x49,0xBF,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x41,0xFF,0xE7 };
	memcpy(SystemCallRecoverCode, (PVOID)KiSystemServiceStart, sizeof(SystemCallRecoverCode));
	memcpy(hook + 4, &PtrKiSystemServiceStart, sizeof(PtrKiSystemServiceStart));
	auto irql = WPOFFx64();
	memcpy((PVOID)KiSystemServiceStart, hook, sizeof(hook));
	WPONx64(irql);

	ExclReleaseExclusivity(exclusivity);
}

//ֻ����SSDT����������ShadowSSDT
PVOID GetSSDTEntry(IN ULONG index)
{
	PSYSTEM_SERVICE_DESCRIPTOR_TABLE pSSDT = aSYSTEM_SERVICE_DESCRIPTOR_TABLE;
	PVOID pBase = (PVOID)KernelBase;

	if (pSSDT && pBase)
	{
		// Index range check ��shadowssdt��Ļ�����0
		if (index > pSSDT->NumberOfServices)
			return NULL;

		return (PUCHAR)pSSDT->ServiceTableBase + (((PLONG)pSSDT->ServiceTableBase)[index] >> 4);
	}

	return NULL;
}

void InitUserSystemCallHandler(decltype(&SystemCallHandler) UserHandler)
{
	UserSystemCallHandler = UserHandler;
}

void SystemCallHandler(KTRAP_FRAME * TrapFrame,ULONG SSDT_INDEX)
{

#ifdef DBG
	//������¼�����˶��ٴ�ϵͳ���ã�����debug��ֻ�е�һ�ε�ʱ������
	static LONG64 SysCallCount = 0;
	if (!SysCallCount) {
		Log("[SysCallCount]at %p\n", &SysCallCount);
		Log("[SYSCALL]%s\nIndex %x\nTarget %llx\n", GetSyscallProcess(), SSDT_INDEX, GetSSDTEntry(SSDT_INDEX));
	}
	SysCallCount++;
#endif

	//Ȼ��Ӧ�õ����û����Ĵ����������û���ṩ����ʹ��Ĭ�ϵ�

	if (UserSystemCallHandler)
	{
		UserSystemCallHandler(TrapFrame, SSDT_INDEX);
	}
}

void SystemCallLog(KTRAP_FRAME* TrapFrame, ULONG SSDT_INDEX)
{
	const char* syscall_name = GetSyscallProcess();
#if 0
	Log("%s\n", syscall_name);
#endif
	//
	//����Ҫ���shadow ssdt��
	//

	//
	//ConsoleApplication3.vmp.exe
	//��ΪEPROCESS.ImageFileName[16]��������Ҫ�ض���16���ֽ�(��ĩβ���ַ�)
	//�������exe����ĿĿ¼��
	//
	if (!strcmp(syscall_name, "ConsoleApplica")) {
		if (SSDT_INDEX < 0x1000)
			Log("[%s]Syscall rip %p SSDT Index %p\n", syscall_name,TrapFrame->Rip - 2, SSDT_INDEX);
	}
}