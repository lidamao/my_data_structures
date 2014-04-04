#include<stdio.h>
#include <string.h>
#include <new>

void Print(int* next, int len)
{
    printf("next:");
    for(int i=0; i<len; i++)
    {
        printf("%d ", next[i]);
    }
    printf("\n");
}

//部分匹配表, 蛮力法
void CreatePartialMatchTable_1(int* next, const char* pattern, int patternLen)
{
    next[0] = 0;
    for(int i=1; i<patternLen; i++)
    {
        int submatch;  //匹配长度
        for(int j=1; j<=i; j++)
        {
            submatch = 0;
            while(pattern[j+submatch] == pattern[submatch] && submatch <= i-j)
                submatch++;
            if(submatch > i-j)
                break;
        }
        next[i] = submatch;
    }
}

//递推, next[j] = 前后缀匹配的最长字符数目
void CreatePartialMatchTable_2(int* next, const char* pattern, int patternLen)
{
    //next[j]=k, 位置j截止, 前后缀有k个匹配字符,或者叫match[]
    /*P0 P1...P(k-1) = P(j-k+1)...P(j)     P(j)=P(j-k+k)
      如果 P(k) == P(j+1),
      那么 next[j+1] = next[j]+1 = k+1 (k=0也成立,这时next[j+1]=1)
    */
    int j = 1;  
    int match = 0;
    next[0] = 0;
    while(j < patternLen)
    {
        if(pattern[j] == pattern[match]) 
        {
            match++;
            next[j] = match;   //next[j+1] = next[j]+1 = k+1
            j++;               //next[j+2]
            continue;
        }
        if(match == 0)
        {
            next[j++] = 0;
        }
        else
        {
            match = next[match-1];
        }
    }
}

//递推优化, next[j] = 前后缀匹配的最长字符数目
void CreatePartialMatchTable_3(int* next, const char* pattern, int patternLen)
{
    int j = 1;  
    int match = 0;
    next[0] = 0;
    while(j < patternLen)
    {
        if(pattern[j] == pattern[match]) 
        {
            match++;
            next[j] = next[match-1];    //优化 
            j++;               
            continue;
        }
        if(match == 0)
        {
            next[j++] = 0;
        }
        else
        {
            match = next[match-1];
        }
    }
}

//返回第一个匹配的子串位置
int KmpGrep(const char *pattern, const char* text, int len)
{
    int patternLen = strlen(pattern);
    int end = patternLen - 1;
    int match = 0;
    int pos = 0;
    
    int *next = new int[patternLen];
    
    CreatePartialMatchTable_2(next, pattern, patternLen);
    
    Print(next, patternLen);
    
    while(match < patternLen && pos < len){
        if(pattern[match] == text[pos]){
            match++;
            pos++;
            continue;
        }
        if(match == 0){
            pos++;
            printf("move : 1\n");
            continue;
        }
        
        //计算移动数目
        int shift = match - next[match-1];
        printf("move : %d\n", shift);
        
        //match位置变化,等于失配字符上一字符的match数目
        //pos不变,在失配字符处继续匹配
        match = next[match-1]; 
    }
    
    delete[] next;
    
    if(match == patternLen)
        return pos-match;
    else
        return -1;
}

//--------------------分割线------------------------------------------
void GetNext(int* next, const char* pattern, int patternLen)
{
    //next[j]=k, 第j个字符失配后, k为重新比较的位置 0 <= k < j
    /*已知P0 P1...P(k-1) = P(j-k)...P(j-1)  
      如果 P(k) == P(j),
      那么 next[j+1] = next[j]+1 = k+1
    */
    int j = 0;  
    int k = -1;
    next[0] = -1;   //作为终止条件,如果k = next[k] = 0,那么next[j+1] = 0 
    while(j < patternLen)
    {
        if( k == -1 || pattern[j] == pattern[k]) 
        {
            j++;
            k++;
            
            next[j] = k;      // next[1] = 0
            
            /*if(pattern[j] != pattern[k])      //优化,get_nextval
                next[j] = k;
            else
                next[j] = next[k];
            */
        }
        else
            k = next[k];
    }
}

//返回第一个匹配的子串位置
int KmpSearch(const char *pattern, const char* text, int len)
{
    int patternLen = strlen(pattern);
    int end = patternLen - 1;
    int pos = 0;
    
    int *next = new int[patternLen];
    
    GetNext(next, pattern, patternLen);
    Print(next, patternLen);
        
    int j = 0;
    while(j < patternLen && pos < len){
        if(j == -1 || pattern[j] == text[pos]){
            j++;
            pos++;
        }
        else{
            //计算移动数目
            int shift = j - next[j];
            printf("move : %d\n", shift);
                        
            j = next[j]; 
        }
    }
    
    delete[] next;
    
    if(j == patternLen)
        return pos-j;
    else
        return -1;
}

//--------------------------------------------------------------


int main(int argc, char**argv)
{
    char *test = argv[1];
    char *pattern = argv[2];
    printf("pos1 is: %d\n\n", KmpSearch(pattern, test, strlen(test)));
    printf("pos2 is: %d\n", KmpGrep(pattern, test, strlen(test)));
}
