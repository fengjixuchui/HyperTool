#pragma once
#include "include/stdafx.h"
#include "include/string.hpp"

// һ��ж�ص�ʱ����ΪTrue
inline bool ConfigExitVar = false;

// ϵͳ�߳̾��
inline HANDLE hConfigThread;

void ConfigUpdateThread(
	PVOID StartContext
);


struct tagGlobalConfig {
	tagGlobalConfig() = default;
	~tagGlobalConfig() = default;
	bool hooks_log;
	std::string path;
};

HANDLE OpenFile(wchar_t* filepath);
ULONG GetFileSize(HANDLE pFileHandle);
NTSTATUS ReadFile(HANDLE pFileHandle, void* buffer, unsigned long size);
NTSTATUS CloseFile(HANDLE pFileHandle);