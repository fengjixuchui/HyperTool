#pragma once 
#include<ntdef.h>

struct FakePage
{
    // //Ҫfake��guest���Ե�ַ
    PVOID GuestVA;
    PHYSICAL_ADDRESS GuestPA;
    // �������ҳ�����Ϣ����vmlaunch֮ǰ����,Ҳ����guest�ܿ�����ҳ������
    PVOID PageContent;
    PHYSICAL_ADDRESS PageContentPA;
};

struct ICFakePage
{
    virtual void Construct() = 0;
    virtual void Destruct() = 0;
    FakePage fp;
};
