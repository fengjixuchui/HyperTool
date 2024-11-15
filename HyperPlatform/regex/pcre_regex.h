#define PCRE_STATIC // ��̬�����ѡ��
#include "pcre.h"

extern "C" int _fltused = 0;

#define OVECCOUNT	30 /* should be a multiple of 3 */
#define OVECCOUNTJIT 64 /*for jit*/
#define EBUFLEN		128
#define BUFLEN		1024
#define PCRE_BUG	0x80000000

#define MUA     (PCRE_MULTILINE | PCRE_UTF8 | PCRE_NEWLINE_ANYCRLF)
#define MUAP    (PCRE_MULTILINE | PCRE_UTF8 | PCRE_NEWLINE_ANYCRLF | PCRE_UCP)
#define CMUA    (PCRE_CASELESS | PCRE_MULTILINE | PCRE_UTF8 | PCRE_NEWLINE_ANYCRLF)
#define CMUAP   (PCRE_CASELESS | PCRE_MULTILINE | PCRE_UTF8 | PCRE_NEWLINE_ANYCRLF | PCRE_UCP)
#define MA      (PCRE_MULTILINE | PCRE_NEWLINE_ANYCRLF)
#define MAP     (PCRE_MULTILINE | PCRE_NEWLINE_ANYCRLF | PCRE_UCP)
#define CMA     (PCRE_CASELESS | PCRE_MULTILINE | PCRE_NEWLINE_ANYCRLF)

/** 
* @_ismatch  ʵ���ַ����������Ƿ�ƥ�� 
* @param   src Դ�ַ���
* @param   pattern  ������ʽ 
* @return  ������ط� -1 ������ƥ�䵽 
*/ 

int _ismatch( char* src, char* pattern)
{   
	pcre  *re;
	const char *error;
	int  erroffset;
	int  ovector[OVECCOUNT];
	int  result;

	re = pcre_compile(pattern,// pattern, �����������Ҫ��������ַ�����ʽ��������ʽ
		0,            // options, �������������ָ������ʱ��һЩѡ��
		&error,       // errptr, ����������������������Ϣ
		&erroffset,   // erroffset, ���������pattern�г���λ�õ�ƫ����
		NULL);        // tableptr, �������������ָ���ַ���һ�������NULL


	if (re == NULL) {  //�������ʧ�ܣ����ش�����Ϣ
		return -1;
	}
	result = pcre_exec(re, // code, �����������pcre_compile����õ�������ṹ��ָ��
		NULL,          // extra, ���������������pcre_exec��һЩ�����������Ϣ�Ľṹ��ָ��
		src,           // subject, ���������Ҫ������ƥ����ַ���
		strlen(src),   // length, ��������� Ҫ������ƥ����ַ�����ָ��
		0,             // startoffset, �������������ָ��subject��ʲôλ�ÿ�ʼ��ƥ���ƫ����
		0,             // options, ��������� ����ָ��ƥ������е�һЩѡ��
		ovector,       // ovector, �����������������ƥ��λ��ƫ����������
		OVECCOUNT);    // ovecsize, ��������� ��������ƥ��λ��ƫ���������������С

	if (result < 0) {
		pcre_free(re);
		return -1;
	}

	pcre_free(re);

	return result;
}  