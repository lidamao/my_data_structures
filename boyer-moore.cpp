#include<stdio.h>
#include <string.h>
#include <new>

int badCharShift[256];
int goodSuffixShift[512];


//array bmBC = 256
void BadCharRule(const unsigned char* pattern, int patternLen, int *bmBC)
{
    //#1 初始化坏字符规则
    for(int i=0; i<256; i++){
        bmBC[i] = -1;
    }
    for(int i=0; i<patternLen; i++){
        bmBC[pattern[i]] = i;   //字符离左边的距离
    }
}

//计算子串的公共后缀
void Suffix(int* suffix, const char* pattern, int patternLen)
{
    int end = patternLen-1;
    suffix[end] = patternLen;
    for(int i=end-1; i>=0; i--) //从pattern右边遍历
    {
        int match = 0;
        while(i>=match && pattern[i-match] == pattern[end-match]) 
            match++;
        
        suffix[i] = match;
    }
}

//bmGS : 失配的下标pos与对应的移动长度
void GoodSuffixRule(const char* pattern, int patternLen, int *bmGS)
{
    //#2 初始化好后缀规则
    int* suffix = new int[patternLen];
    Suffix(suffix, pattern, patternLen);   //计算公共后缀
    
    //case 3 : 假设后缀在模式前部没有匹配
    for(int i=0; i<patternLen; i++)  
    {
        bmGS[i] = patternLen;   //移动长度为整个模式字符串
    }
    
    //case 2 : 后缀在模式前部有匹配, 且从头部开始, 此时匹配长度suffix[x] = x+1
    for(int i=patternLen-2; i>=0; i--)
    {
        if(suffix[i] == i+1){
            for(int unmatch_pos = 0; unmatch_pos < patternLen-1-i; unmatch_pos++)
            {
                if(bmGS[unmatch_pos] == patternLen)  //不覆盖长后缀的移动长度
                    bmGS[unmatch_pos] = patternLen - 1 - i;
            }
        }
    }
    
    //case 1 : 后缀在模式中间有匹配
    for(int i=0; i<patternLen-2; i++)
    {
        //1. suffix[i] = 0 : 第一个字符即失配,移动到与第一个字符不等的字符位置
        //2. suffix[i] = i+1 : 与case2不冲突
        //3. suffix[i] = x : 中间匹配
        //三种计算公式一致
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

//返回移动字符数
int BM(char badChar, int patternLen, int match)
{
    int bcMove = BadCharacterMove(patternLen - match, badChar);
    int gsMove = GoodSuffixMove(patternLen-match-1);
    printf("bcMove|gsMove: %d|%d\n", bcMove, gsMove);
    return (bcMove > gsMove ? bcMove:gsMove);
}

//返回第一个匹配的子串位置
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
        
        //计算移动数目
        int shift = BM(text[pos-match], patternLen, match);
        
        pos += shift;
        match = 0;    //重置匹配数目 
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
