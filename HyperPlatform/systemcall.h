#include <ntdef.h>
#include <ntimage.h>

#include "log.h"
#include "FakePage.h"
#include "error_bugcheck.h"
#include "ssdt.h"
#include "sssdt.h"

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

inline 
NTSTATUS
(*MmAccessFault)(
	IN ULONG_PTR FaultStatus,
	IN KTRAP_FRAME* TrapInformation,
	IN PVOID VirtualAddress,
	IN KPROCESSOR_MODE PreviousMode
);

inline 
NTSTATUS
(*MmAttachSession)(
	IN PVOID OpaqueSession,
	OUT PRKAPC_STATE ApcState
	);

inline
NTSTATUS
(*MmDetachSession)(
	IN PVOID OpaqueSession,
	IN PRKAPC_STATE ApcState
);

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

	inline HANDLE g_CsrssPid;

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

}


//
//��vm��ʼ��֮ǰ��ʼ����Ҫ�ı���
//

NTSTATUS InitSystemVar();

void DoSystemCallHook();

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