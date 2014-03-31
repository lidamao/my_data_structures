#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "avl_tree.h"
#include "skiplist.h"
#include "string.h"
#include "rbtree.h"

int   MAX_SORT_NUM =  1000000;

void test_avltree()
{
    srandom(time(NULL));
    AVLTree<int,int> avl_tree;
    
    int  data_array[MAX_SORT_NUM];
    //srand((unsigned int)time(NULL)); 
    for(int i=0; i<MAX_SORT_NUM; i++)
    {
        //data_array[i] = (rand()|rand()<<15)%(MAX_SORT_NUM*10);       
        data_array[i] = random() % 10000000;
        //printf("data_array[i].key:%d\n", data_array[i].key);
        avl_tree.InsertNode(data_array[i], data_array[i]+10);
        //avl_tree.dump();
    }
    //avl_tree.dump();
    //printf("avl_tree depth:%d\n", avl_tree.get_depth());
    
    //printf("Delete Node!\n");
    /*for(int i=0; i<MAX_SORT_NUM-10; i++)
    {
        printf("delete key:%d |||", data_array[i]);
        avl_tree.DeleteNode(data_array[i]);
        avl_tree.dump();
    }*/
    
    int sucessForSearch = 0;
    int successForErase = 0;
    int testNum = MAX_SORT_NUM/10;
    for(int i=0; i<testNum; i++)
    {
        int f = random() % 10000000;
        int v;
        if(avl_tree.SearchNode(v, f) == 0)
            sucessForSearch++;
        
        if(avl_tree.DeleteNode(f) == 0)
            successForErase++;
    }
    printf("success:%d|%d\n", sucessForSearch, successForErase);
}

void test_skiplist()
{
    SkipList<int,int> list(16);
    list.init();
    srandom(time(NULL));
    
    for(int i=0; i<MAX_SORT_NUM; i++)
    {
        
        int k = random() % 10000000;
        if(list.insert(k, k+10) < 0)
            printf("insert failed!\n");
    }
    //list.dump();
    
    int sucessForSearch = 0;
    int successForErase = 0;
    int testNum = MAX_SORT_NUM/10;
    for(int i=0; i<testNum; i++)
    {
        int f = random() % 10000000;
        int v;
        if(list.search(f, v) == 0)
            sucessForSearch++;
        
        if(list.erase(f) == 0)
            successForErase++;
    }
    printf("success:%d|%d\n", sucessForSearch, successForErase);
    //list.dump();
}

void test_rbtree()
{
    RBTree<int,int,mempool::LinkListMemPool>  rbtree;
    //RBTree<int,int,mempool::BitmapMemPool>  rbtree;
    //RBTree<int,int,mempool::CrtAllocator>  rbtree;
    
    int ret;
    ret = rbtree.Init();
    if(ret < 0)
        printf("Init failed:%d\n", ret);
    
    srandom(time(NULL));
    
    for(int i=0; i<MAX_SORT_NUM; i++)
    {
        int k = random() % 10000000;
        if((ret = rbtree.Insert(k, k+10)) < 0)
            printf("insert failed:%d!\n", ret);
    }
    //list.dump();
    
    int sucessForSearch = 0;
    int successForErase = 0;
    int testNum = MAX_SORT_NUM/10;
    for(int i=0; i<testNum; i++)
    {
        int f = random() % 10000000;
        int v;
        if(rbtree.Search(f, v) == 0)
            sucessForSearch++;
        
        if(rbtree.Delete(f) == 0)
            successForErase++;
    }
    printf("success:%d|%d\n", sucessForSearch, successForErase);
    
}

int main(int argc, char* argv[])
{
    MAX_SORT_NUM = atoi(argv[2]);
    if(strcmp(argv[1],"avl") == 0){
        test_avltree();
    }
    else if(strcmp(argv[1],"rb") == 0){
        test_rbtree();
    }
    else{
        test_skiplist();
    }
    
    
    return 0; 
    
}