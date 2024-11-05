#include <ntdef.h>
#include <ntimage.h>

#include "log.h"
#include "FakePage.h"
#include "error_bugcheck.h"

typedef struct _SYSTEM_SERVICE_DESCRIPTOR_TABLE
{
	PULONG_PTR ServiceTableBase;
	PULONG ServiceCounterTableBase;
	ULONG_PTR NumberOfServices;
	PUCHAR ParamTableBase;
} SYSTEM_SERVICE_DESCRIPTOR_TABLE, * PSYSTEM_SERVICE_DESCRIPTOR_TABLE;


typedef struct _KLDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	PVOID ExceptionTable;
	ULONG ExceptionTableSize;
	// ULONG padding on IA64
	PVOID GpValue;
	PNON_PAGED_DEBUG_INFO NonPagedDebugInfo;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Unused5;
	PVOID SectionPointer;
	ULONG CheckSum;
	// ULONG padding on IA64
	PVOID LoadedImports;
	PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;


// ��Ҫ��asm�ļ�ʹ��
extern "C"
{
	// ntoskrnl.exe����ַ
	inline ULONG_PTR KernelBase = NULL;
	//win32kfull.sys����ַ
	inline ULONG_PTR Win32kfullBase = NULL;
	//win32kbase.sys����ַ
	inline ULONG_PTR Win32kbaseBase = NULL;
	inline ULONG Win32kfullSize = NULL;

	// �ں�ģ������ͷ
	inline PLIST_ENTRY PsLoadedModuleList;

	inline UNICODE_STRING Win32kfullBaseString = RTL_CONSTANT_STRING(L"win32kfull.sys");
	inline UNICODE_STRING Win32kbaseBaseString = RTL_CONSTANT_STRING(L"win32kbase.sys");


	// syscall -> nt!KiSystemCall64 -> nt!KiSystemServiceStart -> nt!KiSystemServiceRepeat -> call r10
	inline ULONG_PTR KiSystemServiceStart = NULL;
	// �洢���ǵ�Syscall Handler(���) �ĵ�ַ��ָ��
	inline ULONG_PTR PtrDetourKiSystemServiceStart = NULL;
	// ����Ļ�� Handler��������C Handler
	void SystemCallHandler(KTRAP_FRAME* TrapFrame, ULONG SSDT_INDEX);

	// Hook�еĵ���ԭʼ�����ĺ���ָ��
	inline PVOID OriKiSystemServiceStart = NULL;

	void InitUserSystemCallHandler(decltype(&SystemCallHandler) UserHandler);
	// �������������ʼ���������ָ��
	inline decltype(&SystemCallHandler) UserSystemCallHandler = NULL;
	
	// ���д�Ļ���ں˻���ַ
	ULONG_PTR GetKernelBase();

	const char* GetSyscallProcess();
}


//
//��vm��ʼ��֮ǰ��ʼ����Ҫ�ı���
//

NTSTATUS InitSystemVar();

void DoSystemCallHook();

PVOID GetSSDTEntry(IN ULONG index);

struct fpSystemCall : public ICFakePage
{
	virtual void Construct() override
	{
		fp.GuestVA = (PVOID)((KiSystemServiceStart >> 12) << 12);
		fp.PageContent = ExAllocatePoolWithTag(NonPagedPool, PAGE_SIZE,'zxc');
		if (!fp.PageContent)
		{
			KeBugCheck(ExAllocatePoolERROR);
		}
		
		// ���ҳ������
		memcpy(fp.PageContent, fp.GuestVA, PAGE_SIZE);

		fp.GuestPA = MmGetPhysicalAddress(fp.GuestVA);
		fp.PageContentPA = MmGetPhysicalAddress(fp.PageContent);
		if (!fp.GuestPA.QuadPart || !fp.PageContentPA.QuadPart)
			KeBugCheck(MmGetPhysicalAddressError);
	}
	virtual void Destruct() override
	{

	}

};

void SystemCallLog(KTRAP_FRAME* TrapFrame, ULONG SSDT_INDEX);