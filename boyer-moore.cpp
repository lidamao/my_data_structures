#include<stdio.h>
#include <string.h>
#include <new>

int badCharShift[256];
int goodSuffixShift[512];


//array bmBC = 256
void BadCharRule(const unsigned char* pattern, int patternLen, int *bmBC)
{
    //#1 ��ʼ�����ַ�����
    for(int i=0; i<256; i++){
        bmBC[i] = -1;
    }
    for(int i=0; i<patternLen; i++){
        bmBC[pattern[i]] = i;   //�ַ�����ߵľ���
    }
}

//�����Ӵ��Ĺ�����׺
void Suffix(int* suffix, const char* pattern, int patternLen)
{
    int end = patternLen-1;
    suffix[end] = patternLen;
    for(int i=end-1; i>=0; i--) //��pattern�ұ߱���
    {
        int match = 0;
        while(i>=match && pattern[i-match] == pattern[end-match]) 
            match++;
        
        suffix[i] = match;
    }
}

//bmGS : ʧ����±�pos���Ӧ���ƶ�����
void GoodSuffixRule(const char* pattern, int patternLen, int *bmGS)
{
    //#2 ��ʼ���ú�׺����
    int* suffix = new int[patternLen];
    Suffix(suffix, pattern, patternLen);   //���㹫����׺
    
    //case 3 : �����׺��ģʽǰ��û��ƥ��
    for(int i=0; i<patternLen; i++)  
    {
        bmGS[i] = patternLen;   //�ƶ�����Ϊ����ģʽ�ַ���
    }
    
    //case 2 : ��׺��ģʽǰ����ƥ��, �Ҵ�ͷ����ʼ, ��ʱƥ�䳤��suffix[x] = x+1
    for(int i=patternLen-2; i>=0; i--)
    {
        if(suffix[i] == i+1){
            for(int unmatch_pos = 0; unmatch_pos < patternLen-1-i; unmatch_pos++)
            {
                if(bmGS[unmatch_pos] == patternLen)  //�����ǳ���׺���ƶ�����
                    bmGS[unmatch_pos] = patternLen - 1 - i;
            }
        }
    }
    
    //case 1 : ��׺��ģʽ�м���ƥ��
    for(int i=0; i<patternLen-2; i++)
    {
        //1. suffix[i] = 0 : ��һ���ַ���ʧ��,�ƶ������һ���ַ����ȵ��ַ�λ��
        //2. suffix[i] = i+1 : ��case2����ͻ
        //3. suffix[i] = x : �м�ƥ��
        //���ּ��㹫ʽһ��
        bmGS[patternLen-1-suffix[i]] = patternLen - 1 - i;
    }
    
    delete [] suffix;
}

void Preprocess(const char* pattern, int patternLen)
{
    BadCharRule((const unsigned char*)pattern, patternLen, badCharShift);
    
    GoodSuffixRule(pattern, patternLen, goodSuffixShift);
    
}

int BadCharacterMove(int unmatch, unsigned char badChar)
{
    //unmatch -1 : unmatch char position (with 0 begin)
    return (unmatch-1 > badCharShift[badChar] ? (unmatch-1 - badCharShift[badChar]) : 1);
}

int GoodSuffixMove(int unmatch_pos)
{
    return goodSuffixShift[unmatch_pos];
}

//�����ƶ��ַ���
int BM(char badChar, int patternLen, int match)
{
    int bcMove = BadCharacterMove(patternLen - match, badChar);
    int gsMove = GoodSuffixMove(patternLen-match-1);
    printf("bcMove|gsMove: %d|%d\n", bcMove, gsMove);
    return (bcMove > gsMove ? bcMove:gsMove);
}

//���ص�һ��ƥ����Ӵ�λ��
int Grep(const char *pattern, const char* text, int len)
{
    int patternLen = strlen(pattern);
    int end = patternLen - 1;
    int match = 0;
    int pos = end;
    
    Preprocess(pattern, patternLen);
    
    while(match < patternLen && pos < len){
        if(pattern[end-match] == text[pos-match]){
            match++;
            continue;
        }
        
        //�����ƶ���Ŀ
        int shift = BM(text[pos-match], patternLen, match);
        
        pos += shift;
        match = 0;    //����ƥ����Ŀ 
    }
    
    if(match == patternLen)
        return (pos+1 - match);
    else
        return -1;
}

int main(int argc, char**argv)
{
    char *test = argv[1];
    char *pattern = argv[2];
    int pos = Grep(pattern, test, strlen(test));
    printf("pos is:%d|%s\n", pos, test+pos);
    
}
