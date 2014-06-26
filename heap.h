/** @name heap implement
 * 大根堆/小根堆 堆排序 插入 删除
 * \autor    lsf
 * \date     2014-3
 * \version  1.00
 **/


#include "memorypool.h"

// namespace  algorithm_test{}

#define  LEFT_CHILD(parent)  (parent*2+1)   
#define  RIGHT_CHILD(parent) (parent*2+2)
#define  PARENT(child)       ((child-1)/2)

//! @{
template <typename KeyType, typename Allocator=mempool::CrtAllocator >
class Heap{

//!@name Constructors and destructor.
//@{
public:
    //! default constructor
    Heap(){
        allocator_ = NULL;
        size_ = 0;
        count_ = 0;
        root_ = NULL;
    }
    
    //! Destructor
    ~Heap(){
        Clear();
        if(allocator_ != NULL){
            allocator_->Free(nil_);   //free nil node
            delete allocator_;
            allocator_ = NULL;
        }
    }
    
private:
    //! Copy constructor is not permitted.
    Heap(const Heap& rhs);
    
//@}

    //结点定义
    typedef struct HeapNode
    {
        KeyType *   key;
        // add other data here
    }heap;

    //!@name Interface  
public:    
    //! init Heap Class itself
    //! process memory allocate failure  
    // \return 0 if success or negative if failed.
    int Init(int size = 0){
        if(allocator_ == NULL){
            allocator_ = new(std::nothrow) Allocator(sizeof(heap));  //固定大小的分配器
            if(allocator_ == NULL)
                return -1;
            
            if(allocator_->Init() < 0)    //pre-allocate
                return -1;
        }
        Resize(size);     
        return 0;
    }

    //delete heap, but allocator not
    void Clear(){
        if(root_ != NULL){
            delete root_;
        }
        root_ = NULL;
        count_ = 0;
        size_ = 0;
    }

    //! reset heap size 
    int Resize(int size){
        if(size <= 0)
            size = 1024; //default
        
        if(size <= this->size_)
            return 0;

        if(root_ == NULL){
            //TODO: process failure
            root_ = (heap *)allocator_->Malloc(sizeof(heap) * size);
        }
        else{
            heap* newRoot = (heap *)allocator_->Malloc(sizeof(heap) * size);
            if(count_ > 0)
                memcpy(newRoot, root_, count_ * sizeof(heap));
            int cnt = count_;
            Clear();
            root_ = newRoot;
            count_ = cnt;
        }
        this->size_ = size;
        return 0;
    }   

    //! construct heap from initial data list (heapify robert floyd 1964)
    //! data must have same life cycle with heap
    int HeapBuild(const KeyType *data, int count){
        if(count <= 0 || Resize(count * 2) < 0)   // twice space for insert
            return -1;

        //heap build in array
        for(int i=0; i<count; i++){
            root_[i].key = data[i];   //copy pointer, root begin at 0
        }

        count_ = count;
        //floyd堆构造法
        int lastNonLeafNode = (count-2) / 2; //最后一个非叶子结点编号
        
        for(int i=lastNonLeafNode; i>=0; i--){
            HeapDown(i, count-1);    
        }
        return 0;
    }

    //下滤, pos 下滤结点编号, end 末结点编号
    void HeapDown(int pos, int end) 
    {
        HeapNode  t = root_[pos];
        int curr = pos;
        while(true){
            int left = LEFT_CHILD(pos);
            int right = left+1;
            if(right < end && CompareNode(root_[right].key, root_[left].key) < 0){  //左右结点比较
                left = right;
            }
            if(left < end && CompareNode(root_[left].key, t.key) < 0){  
                root_[pos] = root_[left];
                pos = left;
            }
            else{
                break;
            }
        }
        if(curr != pos){
            root_[pos] = t;
        }
        return ;
    }

    //上滤, pos 上滤结点编号
    void HeapUp(int pos)
    {
        HeapNode  t = root_[pos];
        int  curr = pos;
        while(pos > 0){
            int parent = PARENT(pos);
            if(CompareNode(root_[pos].key, root_[parent].key) < 0){ //子结点比父结点小,继续往上
                root_[pos] = root_[parent];
                pos = parent;
            }
            else{
                break;
            }
        }
        if(pos != curr)
            root_[pos] = t;

        return;
    }

    int Insert(const KeyType *data){
        if(count_ == size_){
            if(Resize(size_ * 2) < 0)    //enlarge space
                return -1;
        }
        //从最后沿父结点往上筛选
        root_[count_].key = data;     
        count_++;
        HeapUp(count_);
        
        return 0;
    }

    //! delete root, then adjust heap
    int Delete(){
        if(count_ <= 1){
            count_ = 0;
            return 0;
        }
        count_--;
        root_[0] = root_[count_];
        
        HeapDown(0, count_-1);
        return 0;
    }

    const KeyType* Root()
    {
        if(count_ > 0)
            return root_[0].key;
        else
            return NULL;
    }

    

private:
    int CompareNode(const KeyType *k1, const KeyType *k2){
        if(*k1 > *k2)
            return 1;
        else if(*k1 < *k2)
            return -1;
        else 
            return 0;
    }
    int SwapNode(int pos1, int pos2){
        heap t;
        root_[pos1] = t;
        root_[pos1] = root_[pos2];
        root_[pos2] = t;
    }


private:
    Allocator*      allocator_;    //memory allocator pointer
    heap*           root_;         //root node
    unsigned int    count_;        //current nodes number 
    unsigned int    size_;         //total nodes space
};
//! @}

