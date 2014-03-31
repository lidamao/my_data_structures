#ifndef __B_TREE__H
#define __B_TREE__H


#define  m  5

typedef struct _DATA
{
    char value[64];
    
}DATA;

typedef struct _KEY
{
    int  thekey;
    
}KEY;

typedef struct _BTNode
{
    public:
    
    _BTNode()
    {
        n_key = 0;
        parent = NULL;
        memset(child, 0, sizeof(child));
    }
    
    bool   isLeaf;
    bool   loaded;
    long   fpos;
    
    struct _BTNode *child[m];
    struct _BTNode *parent;
    
    int    n_key;            //key 个数
    KEY    key[m];         //key  0号单元未用
    DATA*  data_ptr[m];    //数据,与KEY对应
    
    
}BTNode, *BTree;

typedef struct _b_tree
{
    int     n_degree;
    int     n_nodes;
    int     n_depth;
        
}b_tree;

class CBTree
{
    
    public:
        CBTree();
        ~CBTree();
    
        int  InitCBTree();
        int  DestroyBTree();
        
        int  InsertNode(DATA &new_data);
        int  DeleteNode(DATA &del_data);
        int  UpdateNode(DATA &up_data);
        int  SearchNode(AVLTree &pNode, int key);
        int  dump();
        int  get_depth() {return bt.n_depth;}
            
    private:
        
        int  Search(AVLTree &pNode, AVLNode* T, AVLNode* parent, int key);
        int  PreOrderTraverse(AVLNode* pCurrent);
    
    private:
        
        BTNode    *head;
        b_tree    bt;
        
};







#endif