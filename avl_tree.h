/*
author:lsf
date: 2012-3

平衡二叉树(Balanced Binary Tree|Height-Balanced Tree) 又称AVL树
在AVL树中任何节点的两个子树的高度最大差别为一，所以它也被称为高度平衡树。
查找、插入和删除在平均和最坏情况下都是O（log n）。增加和删除可能需要通过
一次或多次树旋转来重新平衡这个树。 

max_depth
 
This is the maximum tree depth for an instance of the instantiated class. You almost certainly want to 
choose the maximum depth based on the maximum number of nodes that could possibly be in the 
tree instance at any given time. To do this, let the maximum depth be M such that:
 ?MN(M) <= maximum number of nodes < MN(M + 1)
 
where MN(d) means the minimum number of nodes in an AVL Tree of depth d. Here is a table 
of MN(d) values for d from 2 to 45.

含有n个结点的平衡树的最大深度为logφ(【root-5】(n+1)) -2; 其中φ = (1+ 【root-5】)/2; 黄金分割
 

D  MN(d)
2  2
3  4
4  7
5  12
6  20
7  33
8  54
9  88
10 143
11 232
12 376
13 609
14 986
15 1,596
16 2,583
17 4,180
18 6,764
19 10,945
20 17,710
21 28,656
22 46,367
23 75,024
24 121,392
25 196,417
26 317,810
27 514,228
28 832,039
29 1,346,268
30 2,178,308
31 3,524,577
32 5,702,886
33 9,227,464
34 14,930,351
35 24,157,816
36 39,088,168
37 63,245,985
38 102,334,154
39 165,580,140
40 267,914,295
41 433,494,436
42 701,408,732
43 1,134,903,169
44 1,836,311,902
45 2,971,215,072

以下为AVL树的实现

*/

#ifndef  __AVL_TREE_H_
#define  __AVL_TREE_H_

#include <stdio.h>
#include <new>

template <typename KEY_TYPE, typename VALUE_TYPE>
class AVLTree
{
public:
    AVLTree()
    {
        avl_head = NULL;
        size = 0;
        //depth = 0;
    }
    ~AVLTree(){Clear(avl_head);}

    int  InitAVLTree();
    int  DestroyAVLTree();

private:
    
    //结点定义        
    typedef struct _AVLNode
    {
        KEY_TYPE    key;    //数据
        VALUE_TYPE  value;
        
        int    bf;          //balance factor 平衡因子
        struct _AVLNode  *lchild;
        struct _AVLNode  *rchild;
        struct _AVLNode  *parent;
        
    }AVLNode;

public:
    //插入结点
    int  InsertNode(const KEY_TYPE& ins_key, const VALUE_TYPE& ins_value)
    {
        if(avl_head == NULL)   //插入首结点
        {
            avl_head = new(std::nothrow)AVLNode();
            if(avl_head == NULL)
                return -1;
            
            avl_head->key = ins_key;
            avl_head->value = ins_value;
            avl_head->bf = 0;
            avl_head->lchild = NULL;
            avl_head->rchild = NULL;
            avl_head->parent = NULL;
            size++;
            return 1;
        }
        
        AVLNode* pPrevNode = NULL;  //插入结点的父结点
        int ret = Search(pPrevNode, ins_key);
        if(ret == 0) // key已存在
        {
            return 0;  //action: ignore or update 
        }
        
        AVLNode* pInsNode = new(std::nothrow) AVLNode();
        pInsNode->key = ins_key;
        pInsNode->value = ins_value;
        pInsNode->bf = 0;
        pInsNode->lchild = NULL;
        pInsNode->rchild = NULL;
        pInsNode->parent = pPrevNode;
        if(pInsNode->key < pPrevNode->key)
        {
            pPrevNode->lchild = pInsNode;
        }
        else
        {
            pPrevNode->rchild = pInsNode;
        }
        
        AVLNode *childNode = pInsNode;
        
        while(pPrevNode != NULL)    //回溯修改平衡因子
        {
            pPrevNode->bf += (pPrevNode->key > childNode->key) ? 1:-1;
            if(pPrevNode->bf == 2)
                {R_Balance(pPrevNode); }       //右旋    
            else if(pPrevNode->bf == -2)
                {L_Balance(pPrevNode); }       //左旋
            
            //当结点的BF值由1或-1变为0，表明高度小的子树添加了新结点，
            //树的高度没有增加，所以不必修改祖先结点的平衡因子，回溯停止
            if(pPrevNode->bf == 0)   
                break;
            
            childNode = pPrevNode;
            pPrevNode = pPrevNode->parent;
        }
        
        ++size;
        return 1;
    }
        
    //删除结点
    int  DeleteNode(KEY_TYPE& del_key)
    {
        AVLNode *pCurrent = NULL;
        if(Search(pCurrent, del_key) < 0)
            return -1;
        
        //左右子树都不为空,查找待删除结点的直接前驱结点
        AVLNode *prevNode = NULL;
        if(pCurrent->lchild != NULL && pCurrent->rchild != NULL)
        {
            prevNode = pCurrent->lchild;
            while(prevNode->rchild != NULL) 
                prevNode=prevNode->rchild;
            
            //修改链接关系
            if(prevNode->parent == pCurrent)
                pCurrent->lchild = prevNode->lchild;
            else
                prevNode->parent->rchild = prevNode->lchild;
            
            if(prevNode->lchild != NULL)   //删除结点(直接前驱)的左孩子挂在删除结点的父结点
            {
                prevNode->lchild->parent = prevNode->parent;
            }
           
            //pCurrent = prevNode;  //pCurrent 指向直接前驱(待删除结点)
        }
        else //待删除结点只有左子树或右子树是叶子结点
        {
            prevNode = (pCurrent->lchild == NULL)? pCurrent->rchild : pCurrent->lchild;
            if(pCurrent->parent == NULL)  //删除根结点
            {
                if(prevNode != NULL)
                    prevNode->parent = NULL;
                
                avl_head = prevNode;
                delete pCurrent;
                --size;
                return 0;
            }
            if(pCurrent == pCurrent->parent->lchild)
            {
                if(prevNode != NULL)
                    prevNode->parent = pCurrent->parent;
                
                pCurrent->parent->lchild = prevNode;
            }
            else
            {
                if(prevNode != NULL)
                    prevNode->parent = pCurrent->parent;
                
                pCurrent->parent->rchild = prevNode;
            }
            
            prevNode = pCurrent;  //prevNode 指向待删除结点
        }
        
        AVLNode *parentNode = prevNode->parent; 
        AVLNode *childNode = prevNode;
        while(parentNode != NULL)    //回溯修改平衡因子
        {
            parentNode->bf -= (parentNode->key > childNode->key) ? 1:-1;
            if(parentNode->bf == 2)
                R_Balance(parentNode);        //右旋    
            else if(parentNode->bf == -2)
                L_Balance(parentNode);        //左旋
            
            //与插入结点不同的是,删除结点旋转操作完成后,子树的高度降低,可能
            //导致祖先的不平衡, 需要继续向上回溯修改祖先的BF值(旋转次数可能多于插入结点)
            //当结点的BF值由0变为-1或1，表明删除结点后子树高度未变，回溯停止
            if(parentNode->bf == 1 || parentNode->bf == -1)   
                break;
            
            childNode = parentNode;
            parentNode = parentNode->parent;
        }
        if(pCurrent != prevNode){ 
            pCurrent->key = prevNode->key;  //直接前驱数据覆盖删除结点数据
            pCurrent->value = prevNode->value;        
        }
        --size;
        delete prevNode;
        return 0;
        
    }
        
    //查找结点
    int  SearchNode(VALUE_TYPE &ret_value, const KEY_TYPE& s_key)
    {
        AVLNode* node = NULL;
        if(Search(node, s_key) == 0){
            ret_value = node->value;
            return 0;
        }
        return -1;
    }
    
    //!recursive postorder tree walk
    void Clear(AVLNode* root){
        if(root != NULL){
            Clear(root->lchild);
            Clear(root->rchild);
            delete root;
            --size;
        }
    }
    
    //just for test
    int  dump()
    {
        PreOrderTraverse(avl_head);
        printf(" |||size:%d\n", size);

        return 0;
    }
    //int  get_depth() {return depth;}
                    
private:
    /*
    递归查找关键字等于KEY的数据结点，若查找成功，则pNode指向该结点，并返回0. 否则pNode指向
    查找路径上访问的最后一个结点,并返回-1
    OUTPUT: pNode
    INPUT:  T, key
    RETURN: 0, -1
    */
    int  Search(AVLNode* &pNode, AVLNode* T, AVLNode* parent, const KEY_TYPE& s_key)
    {
        if(T == NULL)
        {
            pNode = parent;    
            return -1;
        }
        if(T->key == s_key)
        {
            pNode = T;
            return 0;
        }
        else if(T->key > s_key)
            return Search(pNode, T->lchild, T, s_key);
        else
            return Search(pNode, T->rchild, T, s_key);
    }
    
    /*
    非递归查找版本
    OUTPUT: pNode
    INPUT:  T, key
    RETURN: 0, -1
    */
    int Search(AVLNode* &pNode, const KEY_TYPE& s_key)
    {
        AVLNode *pCurrent = avl_head;
        AVLNode *pPrevNode = NULL;
        
        //查找待删除结点, 保存在pCurrent
        while(pCurrent != NULL)
        {
            pPrevNode = pCurrent;
            if(pCurrent->key == s_key)
                break;                
            else if(pCurrent->key > s_key)
                pCurrent = pCurrent->lchild;
            else
                pCurrent = pCurrent->rchild;
                        
        }
        if(pCurrent == NULL){   //NOT FOUND
            pNode = pPrevNode;
            return -1;
        }
        pNode = pCurrent;
        return 0;
    }
    
    /*当旋转根的BF值为-2时：
      如果旋转根的右孩子的BF值为-1，则进行L型旋转；
      如果旋转根的右孩子的BF值为1，则进行RL型旋转。
      旋转完成 root赋值为新的子树根.
    */
    void L_Balance(AVLNode* &root)
    {
        //printf("in L_Balance!\n");
        AVLNode* t = root->rchild;
        if(t->bf == -1 || t->bf == 0)  //L
        {
            t->parent = root->parent;    //t 作为新子树根
            root->parent = t;
            if(t->lchild != NULL)
            {
                t->lchild->parent = root;
            }
            root->rchild = t->lchild;
            t->lchild = root;
            
            //重置平衡因子
            if(t->bf == -1)
            {
                t->bf = 0;
                root->bf = 0;
            }
            else //if(t->bf == 0)  //旋转根的右孩子bf为0, 删除结点时可能出现
            {
                t->bf = 1;
                root->bf = -1;
            }
            
            //root重置
            root = t;
        }
        else if(t->bf == 1)    //RL
        {
            AVLNode* g = t->lchild;    
            g->parent = root->parent;  //g 作为新子树根
            t->parent = g;
            root->parent = g;
            t->lchild = g->rchild;
            
            if(g->rchild != NULL)
                g->rchild->parent = t;
            
            root->rchild = g->lchild;
            
            if(g->lchild != NULL)
                g->lchild->parent = root;
            
            g->rchild = t;
            g->lchild = root;
            //重置平衡因子
            switch (g->bf)
            {
                case 1:
                    {
                        t->bf = -1;
                        root->bf = 0;
                        g->bf = 0;
                        break;                    
                    }
                case -1:
                    {
                        t->bf = 0;
                        root->bf = 1;
                        g->bf = 0;
                        break;
                    }
                case 0:
                    {
                        t->bf = 0;
                        root->bf = 0;
                        g->bf = 0;
                        break;
                    }
                default:
                    {
                        //printf("L_Balance::wrong RL|g->bf:%d!\n", g->bf);
                        break;
                    }
            }
            
            //root重置
            root = g;    
        }    
        else
        {
            //printf("L_Balance::wrong t->bf:%d!\n", t->bf);
        }
        
        if(root->parent !=NULL)
        {
            if(root->parent->key > root->key)
                root->parent->lchild = root;
            else
                root->parent->rchild = root;
        }
        else
            avl_head = root;
               
    }
    
    /*当旋转根的BF值为2时：
      如果旋转根的左孩子的BF值为1，则进行R型旋转；
      如果旋转根的左孩子的BF值为-1，则进行LR型旋转。
      如果旋转根的左孩子的BF值为0，则进行R型旋转
      旋转完成 root赋值为新的子树根.
    */
    void R_Balance(AVLNode* &root)
    {
        //printf("in R_Balance!\n");
        AVLNode* t = root->lchild;
        if(t->bf == 1 || t->bf == 0)  //R
        {
            t->parent = root->parent;    //t 作为新子树根
            root->parent = t;
            if(t->rchild != NULL)
            {
                t->rchild->parent = root;
            }
            root->lchild = t->rchild;
            t->rchild = root;
            //重置平衡因子
            if(t->bf == 1)
            {
                t->bf = 0;
                root->bf = 0;
            }
            else //if(t->bf == 0)  //旋转根的左孩子bf为0, 删除结点时可能出现
            {
                t->bf = -1;
                root->bf = 1;
            }
            
            //root重置
            root = t;
        }
        else if(t->bf == -1)    //LR
        {
            AVLNode* g = t->rchild;    
            g->parent = root->parent;  //g 作为新子树根
            t->parent = g;
            root->parent = g;
            t->rchild = g->lchild;
            
            if(g->lchild != NULL)
                g->lchild->parent = t;
            
            root->lchild = g->rchild;
            
            if(g->rchild != NULL)
                g->rchild->parent = root;
            
            g->lchild = t;
            g->rchild = root;
            
            //重置平衡因子
            switch (g->bf)
            {
                case 1:
                    {
                        t->bf = 0;
                        root->bf = -1;
                        g->bf = 0;
                        break;                    
                    }
                case -1:
                    {
                        t->bf = 1;
                        root->bf = 0;
                        g->bf = 0;
                        break;
                    }
                case 0:
                    {
                        t->bf = 0;
                        root->bf = 0;
                        g->bf = 0;
                        break;
                    }
                default:
                    {
                        //printf("R_Balance::wrong g->bf:%d!\n", g->bf);
                        break;
                    }
            }
            
            //root重置
            root = g;    
        }    
        else
        {
            //printf("R_Balance::wrong t->bf:%d!\n", t->bf);
        }
        
        if(root->parent !=NULL)
        {
            if(root->parent->key > root->key)
                root->parent->lchild = root;
            else
                root->parent->rchild = root;
        }   
        else
            avl_head = root;
    }  
    
    //先序遍历
    void  PreOrderTraverse(AVLNode* pCurrent)
    {
        if(pCurrent != NULL)
        {
            printf("%d(%d)", pCurrent->key, pCurrent->bf);
            if(pCurrent->lchild != NULL)
            {        
                printf("->L");
                PreOrderTraverse(pCurrent->lchild);
            }
            if(pCurrent->rchild != NULL)
            {
                printf("->R");
                PreOrderTraverse(pCurrent->rchild);
            }
        }
    }
 private:
        
    AVLNode*  avl_head;
    int       size;
    //int       depth;
};


#endif 