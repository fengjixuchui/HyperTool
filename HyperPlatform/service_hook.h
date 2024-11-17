#pragma once
#include "include/vector.hpp"
#include "include/string.hpp"
#include "util.h"
#include "FakePage.h"
#include <stdint.h>



struct ServiceHook : public ICFakePage
{
	~ServiceHook() {};
	virtual void Construct() override;
	virtual void Destruct() override;


	std::string funcName;       // ��hook�ĺ�������

// ��ȫж�����
	LONG refCount = 0;			// ���Ӻ���������,Ϊ0���ܰ�ȫж��
	
//private:
	PVOID DetourFunc;
	PVOID *TrampolineFunc;
	ULONG HookCodeLen;
	bool isWin32Hook = false;   // �漰��Win32kfullģ���ں�����hook��Ϊtrue
};
extern std::vector<ServiceHook> vServcieHook;
#define ENTER_HOOK(FUNC_NAME)  			for (auto &h : vServcieHook) {					\
				if (!strcmp(h.funcName.c_str(), FUNC_NAME)) {		\
					InterlockedAdd(&h.refCount, 1);			\
				}											\
			}												\
			auto a7808419_a956_4174_865a_4e62a3e7f969 = make_scope_exit([&]() {				\
				for (auto& h : vServcieHook) {				\
					if (!strcmp(h.funcName.c_str(), FUNC_NAME)) { \
						InterlockedAdd(&h.refCount, -1);	\
					}										\
				}											\
				});											

using NtCreateThreadExType = NTSTATUS(*)(
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
	OUT PVOID lpBytesBuffer);

//
// ���뱣֤�����Ҫhook�ĺ����ڸ�rax��ֵ֮ǰ��ʹ��rax����Ϊ����ʹ��rax��Ϊ����
// һ����˵c/c++����������ʹ��rax����ຯ���Ͳ�һ���ˡ�����ϵͳ����ʱ��raxΪssdt index
//
void AddServiceHook(PVOID HookFuncStart, PVOID Detour, PVOID *TramPoline,const char* funcName);
// ж�����й���
void RemoveServiceHook();
