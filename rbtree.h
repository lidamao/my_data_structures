#ifndef _PUBLIC_RBTREE_H_
#define _PUBLIC_RBTREE_H_

/** @name red black tree implement
 *  \autor    lsf
 *  \date     2013-5
 *  \version  1.00
 *
 */

#include "memorypool.h"

//! @{
template <typename KeyType, typename ValueType, typename Allocator=mempool::BitmapMemPool >
class RBTree{

//!@name Constructors and destructor.
//@{
public:
    //! default constructor
    RBTree(){
        allocator_ = NULL;
        size_ = 0;
        nil_ = NULL;
        root_ = nil_;
    }
    
    //! Destructor
    ~RBTree(){
        Clear();
        if(allocator_ != NULL){
            allocator_->Free(nil_);   //free nil node
            delete allocator_;
            allocator_ = NULL;
        }
    }
    
private:
    //! Copy constructor is not permitted.
    RBTree(const RBTree& rhs);
    
//@}

private:
    enum Color{
        RED = 0,
        BLACK = 1
    };
//! rb tree node
    struct RBNode{
        struct RBNode*  parent; 
        struct RBNode*  left;
        struct RBNode*  right;
        unsigned char   color;
        
        KeyType         key;
        ValueType       value;
    };

//!@name Interface  
public:    
    //! init rbtree
    //! process memory allocate failure  
    // \return 0 if success or negative if failed.
    int Init(){
        if(allocator_ == NULL){
            allocator_ = new(std::nothrow) Allocator(sizeof(struct RBNode));
            if(allocator_ == NULL)
                return -1;
            
            if(allocator_->Init() < 0)    //pre-allocate
                return -1;
        }
        if(nil_ == NULL){
            nil_ = (RBNode*)allocator_->Malloc(sizeof(RBNode));  //nil node
            if(nil_ == NULL)
                return -2;
        }
        
        nil_->color = BLACK;
        root_ = nil_;
        
        return 0;
    }   
        
    //! insert a k/v pair
    /*! 
        \param key insert key. must support ==, < comparison.   
        \param value insert value. example, a pointer to real data.    
        \return 0-success; -1-exist key; -2-creat node failed.
    */
    int Insert(const KeyType& key, const ValueType& value){
        RBNode* parent;   
        if(Find(&parent, root_, key) == 0) //exist key
            return 0;   //action: ignore or update 
        
        RBNode* x = (RBNode*)allocator_->Malloc(sizeof(RBNode));  //new node
        if(x == NULL)
            return -2;
        
        //init node
        x->left = nil_;
        x->right = nil_;
        x->color = RED;
        x->parent = parent;
        x->key = key;
        x->value = value;
        ++size_;
        
        if(parent == nil_){   //empty tree
            root_ = x;
            root_->color = BLACK;
            return 0;
        }
        if(x->key > parent->key){
            parent->right = x;
        }
        else{
            parent->left = x;
        }
                
        while(parent->color == RED){ //with no need for process parent == BALCK 
            RBNode* uncle = (parent->parent->left == parent) ? (parent->parent->right) : (parent->parent->left);
            if(uncle->color == RED){   //case 1
                parent->color = BLACK;
                uncle->color = BLACK;
                parent->parent->color = RED; //grandparent set RED (maybe set root)
                
                //check grandparent recurse, since it's parent maybe RED
                x = parent->parent;  
                parent = x->parent;     //when x=root, then parent is nil_ so that break
                continue;
            }
            else if(parent == parent->parent->left){  
                
                /*   C (grandparent|BLACK)       C 
                    /                           /
                   A (parent|RED)     =>L      B        =>next:R rotate to balance
                    \                         /
                     B (x|RED)               A(new x)
    
                notice:if A is RED,then C exist surely, and C is BLACK! (proprety 2,3)
                       when [L Rotate] complete as above, it's on the [R Rotate] situation.  
                */
                
                //! \todo do one [LR rotate] like avl tree.
                if(x == parent->right){     //case 2
                    LeftRotate(parent);                 
                    x = parent;
                }
                
                //recolor
                x->parent->color = BLACK;          //case 3
                x->parent->parent->color = RED;
                
                /*     C (grandparent|BLACK)        
                      /                           B(BLACK)
                     B (parent|RED)     =>       /  \
                    /                           A    C
                   A (x|RED)                  (RED) (RED)
                
                notice: when [R Rotate] complete as above, the tree is balance done.
                */
                RightRotate(x->parent->parent); 
                break;  
            }
            else{
                if(x == parent->left){      //case 2
                    RightRotate(parent);                 
                    x = parent;
                }
                //recolor
                x->parent->color = BLACK;          //case 3
                x->parent->parent->color = RED;
                LeftRotate(x->parent->parent);
                break;                                  
            }
        }
        root_->color = BLACK;  // root's color must be BLACK (case 1 maybe change root's color)
        return 0;
    }
    
    //! search a node
    /*! \param value output value when search key success 
        \param key search key
        \return 0 if success or -1 if failed.
    */
    int Search(const KeyType& key, ValueType& value){
        RBNode* foundNode;
        if(Find(&foundNode, root_, key) < 0)
            return -1;
        
        value = foundNode->value;
        return 0;
    }
    
    //! delete  node
    /*! \param key delete key 
        \return 0 if success or -1 if key not exist.
    */
    int Delete(const KeyType& key){
        RBNode* delNode;   
        if(Find(&delNode, root_, key) < 0) //key not exist
            return -1;
        
        RBNode* realDelNode = delNode;
        if(delNode->left != nil_ && delNode->right != nil_){
            realDelNode = delNode->left;
            while(realDelNode->right != nil_) 
                realDelNode = realDelNode->right;
        }
        
        RBNode* x;  //real delete node's only child, and maybe nil_ .
        if(realDelNode->left != nil_)
            {x = realDelNode->left;}
        else
            {x = realDelNode->right;}
        
        //attention! here may use nil_->parent field, when x=nil_
        //so do not chang nil_'s parent in Rotate function.    
        x->parent = realDelNode->parent;  
        if(realDelNode->parent == nil_){
            root_ = x;
        }
        else if(realDelNode == realDelNode->parent->left){
            realDelNode->parent->left = x;
        }
        else{
            realDelNode->parent->right = x;
        }
        if(delNode != realDelNode){             
            delNode->key = realDelNode->key;
            delNode->value = realDelNode->value;
        }
        if(realDelNode->color == BLACK){  //need FixUp
            DeleteFixUp(x);   //delete is little more complicate than insert         
        }
        allocator_->Free(realDelNode);
        --size_;
        return 0;
    }
    
    //! clear rb tree
    //! \return 0 if success or -1 if failed.
    void Clear(){
        
        //DeleteAll(root_);
        
        //! non-recursive Inorder tree walk
        RBNode *prev = nil_;
        RBNode *next = nil_;
        RBNode *curr = root_;
        while(curr != nil_){
            if(prev == curr->parent){
                prev = curr;
                next = curr->left;
            }
            if(next == nil_ || prev == curr->left){
                prev = curr;
                next = curr->right;
            }
            if(next == nil_ || prev == curr->right){
                prev = curr;
                next = curr->parent;
                //visit node
                allocator_->Free(curr);
                --size_;
            }
            curr = next;
        }
        
    }
    
    //!recursive postorder tree walk
    void DeleteAll(RBNode* root){
        if(root != nil_){
            DeleteAll(root->left);
            DeleteAll(root->right);
            allocator_->Free(root);
            --size_;
        }
    }
    
    
//@}

private:
    
    /*! Left Rotate
      \param  x  the rotate node 
      parent                           parent
        |                                |
        x (rotate node)     =>L          t  
         \                              /
          t (pivot node)               x 
    
      notice: rotate node and pivot node must exist.
    */
    void LeftRotate(RBNode* x){
        RBNode* pivot = x->right;
        
        pivot->parent = x->parent;
        if(x->parent != nil_){
            if(x->parent->left == x)
                x->parent->left = pivot;
            else
                x->parent->right = pivot;
        }
        else{
            root_ = pivot;     //x == root_, reset root
        }
        
        x->right = pivot->left;
        if(pivot->left != nil_){      //do not change nil_'s parent
            pivot->left->parent = x;
        }
        
        x->parent = pivot;
        pivot->left = x;
    }
    
    /*! Right Rotate is symmetric with L Rotate.
        just swap "left" and "right" in L Rotate code.
          parent                           parent
            |                                |
            x (rotate node)     =>R          t  
           /                                  \
          t (pivot node)                       x 
    */
    void RightRotate(RBNode* x){
        RBNode* pivot = x->left;
        
        pivot->parent = x->parent;
        if(x->parent != nil_){
            if(x->parent->left == x)
                x->parent->left = pivot;
            else
                x->parent->right = pivot;
        }
        else{
            root_ = pivot;     //x == root_, reset root
        }
        
        x->left = pivot->right;
        if(pivot->right != nil_){
            pivot->right->parent = x;
        }
        
        x->parent = pivot;
        pivot->right = x;
    }
    
    //! fix rb_tree proprety when delete
    void DeleteFixUp(RBNode* x){   
        // x is "double-BLACK" or "RED-BLACK"
        while(x != root_ && x->color == BLACK){   //double-BLACK
            if(x == x->parent->left){
                RBNode* brother = x->parent->right; 
                if(brother->color == RED){                  //case 1
                    x->parent->color = RED;   //one child RED, parent mustbe BLACK
                    brother->color = BLACK;
                    //brother's child mustbe BLACK, so next brother's color mustbe BLACK.
                    LeftRotate(x->parent);    //fall into case 2/3/4
                }
                else if(brother->left->color == BLACK && 
                          brother->right->color == BLACK){  //case 2   
                    //just recolor
                    brother->color = RED;    
                    x = x->parent;        //recurse, fall into case 1/2/3/4
                }
                else{
                    if(brother->left->color == RED){        //case 3
                        brother->left->color = BLACK;
                        brother->color = RED;
                        RightRotate(brother); //fall into case 4
                        brother = x->parent->right;
                    }
                    //brother->right->color == RED          //case 4
                    brother->color = x->parent->color;
                    x->parent->color = BLACK;
                    brother->right->color = BLACK;
                    LeftRotate(x->parent);
                    x = root_;
                }   
            }
            else{
                RBNode* brother = x->parent->left; 
                if(brother->color == RED){                  //case 1
                    x->parent->color = RED;   
                    brother->color = BLACK;
                    RightRotate(x->parent);    //fall into case 2/3/4
                }
                else if(brother->left->color == BLACK && 
                          brother->right->color == BLACK){  //case 2   
                    //just recolor
                    brother->color = RED;    
                    x = x->parent;        //recurse, fall into case 1/2/3/4
                }
                else {
                    if(brother->right->color == RED){       //case 3
                        brother->right->color = BLACK;
                        brother->color = RED;
                        LeftRotate(brother); //fall into case 4
                        brother = x->parent->left;
                    }
                    //brother->right->color == RED          //case 4
                    brother->color = x->parent->color;
                    x->parent->color = BLACK;
                    brother->left->color = BLACK;
                    RightRotate(x->parent);
                    x = root_;
                }
            }
        }
        x->color = BLACK;
    }
    
    //! search a node
    /*! 
        \param prevNode output param, if search success it point to found node, 
                        else point to prev/parent node.  
        \param root start search tree root. 
        \param key search key
        \return 0 if search success or -1 if failed.
    */
    int Find(RBNode** prevNode, RBNode* root, const KeyType &key){
        *prevNode = nil_;
        while(root != nil_){
            *prevNode = root;
            if(root->key == key)
                return 0;                
            else if(root->key > key)
                root = root->left;
            else
                root = root->right;
        }
        return -1;   //NOT FOUND
    }
    
private:
    
    RBNode*  root_;                //root node
    RBNode*  nil_;                 //sentinel node
    Allocator*      allocator_;    //memory allocator pointer
    unsigned int    size_;         //total tree nodes
    
};


//! @}


#endif