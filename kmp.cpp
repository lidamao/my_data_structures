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

//����ƥ���, ������
void CreatePartialMatchTable_1(int* next, const char* pattern, int patternLen)
{
    next[0] = 0;
    for(int i=1; i<patternLen; i++)
    {
        int submatch;  //ƥ�䳤��
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

//����, next[j] = ǰ��׺ƥ�����ַ���Ŀ
void CreatePartialMatchTable_2(int* next, const char* pattern, int patternLen)
{
    //next[j]=k, λ��j��ֹ, ǰ��׺��k��ƥ���ַ�,���߽�match[]
    /*P0 P1...P(k-1) = P(j-k+1)...P(j)     P(j)=P(j-k+k)
      ��� P(k) == P(j+1),
      ��ô next[j+1] = next[j]+1 = k+1 (k=0Ҳ����,��ʱnext[j+1]=1)
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

//�����Ż�, next[j] = ǰ��׺ƥ�����ַ���Ŀ
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
            next[j] = next[match-1];    //�Ż� 
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

//���ص�һ��ƥ����Ӵ�λ��
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
        
        //�����ƶ���Ŀ
        int shift = match - next[match-1];
        printf("move : %d\n", shift);
        
        //matchλ�ñ仯,����ʧ���ַ���һ�ַ���match��Ŀ
        //pos����,��ʧ���ַ�������ƥ��
        match = next[match-1]; 
    }
    
    delete[] next;
    
    if(match == patternLen)
        return pos-match;
    else
        return -1;
}

//--------------------�ָ���------------------------------------------
void GetNext(int* next, const char* pattern, int patternLen)
{
    //next[j]=k, ��j���ַ�ʧ���, kΪ���±Ƚϵ�λ�� 0 <= k < j
    /*��֪P0 P1...P(k-1) = P(j-k)...P(j-1)  
      ��� P(k) == P(j),
      ��ô next[j+1] = next[j]+1 = k+1
    */
    int j = 0;  
    int k = -1;
    next[0] = -1;   //��Ϊ��ֹ����,���k = next[k] = 0,��ônext[j+1] = 0 
    while(j < patternLen)
    {
        if( k == -1 || pattern[j] == pattern[k]) 
        {
            j++;
            k++;
            
            next[j] = k;      // next[1] = 0
            
            /*if(pattern[j] != pattern[k])      //�Ż�,get_nextval
                next[j] = k;
            else
                next[j] = next[k];
            */
        }
        else
            k = next[k];
    }
}

//���ص�һ��ƥ����Ӵ�λ��
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
            //�����ƶ���Ŀ
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
