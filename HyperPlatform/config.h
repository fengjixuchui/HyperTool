#pragma once
#include "include/stdafx.h"

// һ��ж�ص�ʱ����ΪTrue
inline bool ConfigExitVar = false;

// ϵͳ�߳̾��
inline HANDLE hConfigThread;

void ConfigUpdateThread(
	PVOID StartContext
);


struct tagGlobalConfig {
	bool hooks_log;
};

HANDLE OpenFile(wchar_t* filepath);
ULONG GetFileSize(HANDLE pFileHandle);
NTSTATUS ReadFile(HANDLE pFileHandle, void* buffer, unsigned long size);
NTSTATUS CloseFile(HANDLE pFileHandle);