/** @name mempools implement
 *  \autor    lsf
 *  \date     2013-5
 *  \version  1.00
 *
 */
 
#ifndef  __MEMORY_POOL_H_
#define  __MEMORY_POOL_H_

namespace mempool{

#define  ALIGNMENT    8        //alignling size to 8
#define  ALIGN(A) (((A)+(ALIGNMENT-1)) & (~(ALIGNMENT-1)))

typedef unsigned long long  UINT64;
typedef unsigned int        UINT32;
 
//! C-runtime library allocator.
/*! This class is just wrapper for standard C library memory routines.
    \implements Allocator
*/
class CrtAllocator {
public:
    CrtAllocator(unsigned int compatible){}

public:
    int   Init(){return 0;}  //do nothing
    void* Malloc(size_t size) { return malloc(size); }
    void* Realloc(void *ptr, size_t size) { return realloc(ptr, size); }
    void  Free(void *ptr) { free(ptr); }
};

//! memory pool implements.  NOT SUPPORT CONCURRENT!
//! 
/*! These allocators allocate memory blocks from pre-allocated memory chunks,
    to avoid frequently system memory allocation such as malloc/free. 
    1. 位图法:控制区与数据区分开,数据越界不影响内存分配,Malloc是O(n),Free O(1).
    2. 空闲链表:控制区与数据区混合, 分配与释放都是O(1) (固定大小分配). 
    3. 位图结合链表法:初始化耗时

*/ 

//! bitmap memory pool
/*!    
    "BMP" use bitmap to mark a chunk used(1) or unused(0).
    example, it just mark a bit '0' in bitmap struct to free a chunk.
    BMP assume every allocate request same memory size, means it doesn't 
    support Malloc various size. 
    \implements Allocator
*/
//template <size_t unit_size>
class BitmapMemPool{
private:
    static const int defaultChunkCapacity = 1024*1024;   //1M
    static const int defaultChunkMaxCount = 10;
    
//!@name Constructors and Destructor.
//@{
public:
    //! default constructor
    BitmapMemPool(UINT32 unitSize, UINT32 chunkCapacity=defaultChunkCapacity) : 
            unit_(ALIGN(unitSize)), defaultCapacity_(chunkCapacity){
        chunkHead_ = NULL;
        chunkCount_ = 0;
    }
    
    //! Destructor
    ~BitmapMemPool(){
        for(ChunkHeader* curr = chunkHead_;chunkHead_ !=NULL;curr=chunkHead_){
            chunkHead_ = chunkHead_->next;
            free(curr);
        }
    }
    
//@}

public:
    int Init(){
        if(AddChunk(defaultCapacity_) < 0)
            return -1;
        
        //printf("defaultCapacity|unitSize|chunkcount:%u|%u|%u\n", defaultCapacity_, unit_, chunkCount_);
        return 0;
    }
    void* Malloc(size_t size){
        if(size > unit_)
            return NULL;
        
        ChunkHeader* curr = chunkHead_;
        ChunkHeader* pre = NULL;
        while(curr != NULL){
            if(curr->size >= curr->capacity){
                pre = curr;
                curr = curr->next;
            }
            else{ 
                if(curr != chunkHead_){        //set new header
                    pre->next = curr->next;
                    curr->next = chunkHead_;
                    chunkHead_ = curr;
                }
                break;
            }
        }
        if(curr == NULL){
            if(AddChunk(defaultCapacity_) < 0)
                return NULL;
            
            curr = chunkHead_;
        }
        
        UINT32 index = curr->bitmap.Set();
        curr->size++;
        return (void*)(curr->base+(unit_ * index));
        
    }
    
    void  Free(void *ptr){
        ChunkHeader** curr = &chunkHead_;
        while(*curr != NULL){
            ChunkHeader* chunk = *curr;
            if(ptr < chunk->base || ptr >= chunk->end){
                curr = &chunk->next;
            }
            else{
                chunk->bitmap.Clear(((char*)ptr - chunk->base)/unit_);
                chunk->size--;
                if(chunk->size == 0 && chunkCount_ > defaultChunkMaxCount){  //free chunk
                    *curr = chunk->next;
                    --chunkCount_;
                    free(chunk);
                }
                break;
            }
        }
    }
    
    //! not implement
    void* Realloc(void *ptr, size_t size){return NULL;}

private:
    int AddChunk(UINT32 chunkCapacity){
        UINT32 unitCount = (chunkCapacity+ unit_-1) / unit_;
        chunkCapacity = unitCount * unit_;
        
        //size in bytes
        UINT32 bitmapSize = ((unitCount + 63) & ~63) >> 3;   // unitCount align 64
        UINT32 remainder = (bitmapSize<<3) - unitCount;      // must be set 1 
                
        UINT32 indexOneSize = (((bitmapSize >> 3)+63) & ~63) >> 3; //indexOneSize(bits) = bitmapSize/8, align 64
        UINT32 remainderOne = (indexOneSize << 3) - (bitmapSize >> 3); 
        if((bitmapSize & 0x7) > 0)
            remainderOne--;
                        
        ChunkHeader* newChunk=(struct ChunkHeader*)malloc(chunkCapacity+bitmapSize+indexOneSize+sizeof(ChunkHeader));
        if(newChunk == NULL)
            return -1;
        
        char *p = (char*)newChunk+sizeof(ChunkHeader);
        newChunk->bitmap.indexOne = (UINT64*)p;
        newChunk->bitmap.map = (UINT64*)(p + indexOneSize);
        newChunk->base = (char*)(p + indexOneSize + bitmapSize);
        newChunk->end = newChunk->base + chunkCapacity;
        //init bitmap
        memset(newChunk->bitmap.indexOne, 0, indexOneSize + bitmapSize);
        
        UINT64* correct;
        if(remainder > 0){  //correct bitmap, last 64bit set free(0) for remainder
            correct = (UINT64*)newChunk->base - 1;  //point to bitmap last 64bit
            *correct = (1uLL << remainder)-1;
        }   
        if(remainderOne){
            correct = newChunk->bitmap.map - 1;
            *correct = (1uLL << remainderOne) -1;
        }
        
        newChunk->capacity = unitCount;
        newChunk->size = 0;
        newChunk->next = chunkHead_;
        chunkHead_ = newChunk;
        chunkCount_++;
        return 0;
    }
    

private:
    //! Bitmap struct
    struct Bitmap{
      //UINT64*  indexTwo;
        UINT64*  indexOne;    //level-one index
        UINT64*  map;
        
        UINT32 Set(){
            UINT64* go = indexOne;
            while(go < map){
                if(*go == 0xFFFFFFFFFFFFFFFF)
                    go = go+1;
                else
                    break;
            }
            //ASSERT(go < map);
            UINT32 pos1 = GetPos(*go);
            UINT32 aim = ((go-indexOne)<<6) + pos1;
            UINT32 posMap = GetPos(map[aim]);
            //printf("pos1|aim|posMap:%u|%u|%u\n", pos1,aim,posMap);
            
            map[aim] |= 1uLL << (63-posMap);    //set 1
            if(map[aim] == 0xFFFFFFFFFFFFFFFF){
                *go |= 1uLL << (63-pos1);       //set index
            }
            //printf("##%lx|%lx\n", map[aim], *go);
            return  (aim<<6) + posMap; 
        }
        
        void Clear(UINT32 index){
            UINT32 aim = index >> 6;
            UINT32 pos1 = aim & 0x3F;
            UINT32 posMap = index & 0x3F;
            
            //clear
            map[aim] &= ~(1uLL << (63-posMap));     
            indexOne[aim>>6] &= ~(1uLL << (63-pos1));
        }
        
        //scans source operand for first bit set
        UINT32 GetPos(UINT64 val)
        {
            UINT32 pos = 0;
            val = ~val;   //取反 找右起第一个1的位置
            
            UINT32 mask = 0xFFFFFFFF;
            UINT32 step = 32;
            while(step != 0){             //BSR - bit scan forward
                if((val & mask) == 0){
                    pos += step;
                    val >>= step;
                }
                step >>= 1;      //div 2
                mask >>= step;
            }
            return (63-pos);  //turn to reverse
        }
        
    };
    
    //! Chunk header for perpending to each chunk.
    /*! Chunks are stored as a singly linked list.
    */
    struct ChunkHeader {
        UINT32       capacity;    //!< capacity of the chunk, here is number of Unit Entries
        UINT32       size;        //!< current count of allocated Unit.
        Bitmap       bitmap;      //!< bitmap struct 
        ChunkHeader* next;        //!< next chunk in the singly linked list.
        char *       base;        //!< unit area base address
        char *       end;         //!< unit area end address
    };
    
    ChunkHeader *chunkHead_;        //!< Head of the chunk linked-list.
    UINT32       defaultCapacity_;  //!< default chunk Capacity in bytes, without chunk head.
    UINT32       unit_;             //!< allocation Unit size in bytes.
    UINT32       chunkCount_;       //!< total chunks number
};

//! free link-list memory pool
/*!    
    it also doesn't support Malloc various size. 
    \implements Allocator
*/
class LinkListMemPool{

private:
    static const int defaultChunkCapacity = 1024*1024*4;   //memory blocks size
    static const int defaultChunkMaxCount = 10;             //max blocks number
    
//!@name Constructors and Destructor.
//@{
public:
    //! default constructor
    LinkListMemPool(UINT32 unitSize, UINT32 chunkCapacity=defaultChunkCapacity) : 
            unit_(ALIGN(unitSize)), defaultCapacity_(chunkCapacity){
        chunkHead_ = NULL;
        chunkCount_ = 0;
    }
    
    //! Destructor
    ~LinkListMemPool(){
        Clear();
    }
    
//@}

//! interface
public:
    int Init(){
        if(AddChunk(defaultCapacity_) < 0)
            return -1;
        
        //printf("defaultCapacity|unitSize|chunkcount:%u|%u|%u\n", defaultCapacity_, unit_, chunkCount_);
        return 0;
    }
    
    void* Malloc(size_t size){
        if(size > unit_)
            return NULL;
        
        ChunkHeader* curr = chunkHead_;
        ChunkHeader* pre = NULL;
        while(curr != NULL){
            if(curr->capacity - curr->size < size){
                pre = curr;
                curr = curr->next;
            }
            else{ 
                if(curr != chunkHead_){        //set new header
                    pre->next = curr->next;
                    curr->next = chunkHead_;
                    chunkHead_ = curr;
                }
                break;
            }
        }
        if(curr == NULL){
            if(AddChunk(defaultCapacity_) < 0)
                return NULL;
            
            curr = chunkHead_;
        }
        
        if(curr->freeArea == NULL){
            curr->freeArea = (FreeLinkList*)((char*)curr + sizeof(ChunkHeader) + curr->size);
            curr->freeArea->next = NULL;
            //chunk->freeArea->size = unit_;
        }
        void* ret = curr->freeArea;
        curr->freeArea = curr->freeArea->next;
        curr->size += size;
        
        return ret;
    }
    
    void  Free(void *ptr){
        ChunkHeader** curr = &chunkHead_;
        while(*curr != NULL){
            ChunkHeader* chunk = *curr;
            if(ptr < chunk || ptr >= ((char*)chunk+sizeof(ChunkHeader)+chunk->capacity)){
                curr = &chunk->next;
            }
            else{
                chunk->size -= unit_;  //todo: support various size later
                if(chunk->size == 0 && chunkCount_ > defaultChunkMaxCount){  //free chunk
                    *curr = chunk->next;
                    --chunkCount_;
                    free(chunk);
                }
                else{
                    ((FreeLinkList*)ptr)->next = chunk->freeArea;   //add to free list
                    chunk->freeArea = (FreeLinkList*)ptr;
                }
                break;
            }
        }
    }
    
    //! not implement
    void* Realloc(void *ptr, size_t size){return NULL;}

private:
    int AddChunk(UINT32 chunkCapacity)
    {
        UINT32 unitCount = (chunkCapacity+ unit_-1) / unit_;
        chunkCapacity = unitCount * unit_;
        
        ChunkHeader* newChunk=(struct ChunkHeader*)malloc(sizeof(ChunkHeader) + chunkCapacity);
        if(newChunk == NULL)
            return -1;
        
        //init All member
        newChunk->size = 0;
        newChunk->capacity = chunkCapacity;
        newChunk->freeArea = NULL;
        newChunk->next = chunkHead_;
        chunkHead_ = newChunk;
        
        return 0;
    }
    
    //!recursive postorder tree walk
    void Clear(){
        for(ChunkHeader* curr = chunkHead_;chunkHead_ !=NULL;curr=chunkHead_){
            chunkHead_ = chunkHead_->next;
            --chunkCount_;
            free(curr);
        }
    }
    
private:
    //! Chunk header for perpending to each chunk.
    /*! Chunks are stored as a singly linked list.
    */
    struct FreeLinkList{
        FreeLinkList*  next;
        UINT32         size;        //not use
    };
    struct ChunkHeader{
        UINT32         capacity;    //!< capacity of the chunk.
        UINT32         size;        //!< current  allocated size.
        ChunkHeader*   next;        //!< chunk links list.
        FreeLinkList*  freeArea;    //!< free area link stack
    };
    
    ChunkHeader*   chunkHead_;        //!< Head of the chunk linked-list.
    UINT32         defaultCapacity_;  //!< default chunk Capacity in bytes, without chunk head.
    UINT32         unit_;             //!< allocation Unit size in bytes.
    UINT32         chunkCount_;       //!< total chunks number
    
};

//! free link-list memory pool
/*!    
    it also doesn't support Malloc various size. 
    \implements Allocator
*/
/*
class LinkListMemPool{

private:
    static const int defaultChunkCapacity = 1024*1024;   //memory blocks size
    static const int defaultChunkMaxCount = 10;             //max blocks number
    
    //! Chunk header for perpending to each chunk.
    //! Chunks are stored as a singly linked list.
    struct FreeLinkList{
        FreeLinkList*  next;
        UINT32         size;        //not use
    };
    struct ChunkHeader{
        UINT32         capacity;    //!< capacity of the chunk.
        UINT32         size;        //!< current  allocated size.
        ChunkHeader*   left;        //!< chunk links as binary search tree.
        ChunkHeader*   right;       //!<
        FreeLinkList*  freeArea;    //!< free area link stack
    };
    
//!@name Constructors and Destructor.
//@{
public:
    //! default constructor
    LinkListMemPool(UINT32 unitSize, UINT32 chunkCapacity=defaultChunkCapacity) : 
            unit_(ALIGN(unitSize)), defaultCapacity_(chunkCapacity){
        chunkHead_ = NULL;
        chunkCount_ = 0;
    }
    
    //! Destructor
    ~LinkListMemPool(){
        Clear(chunkHead_);
    }
    
//@}

//! interface
public:
    int Init(){
        if(AddChunk(defaultCapacity_) == NULL)
            return -1;
        
        printf("defaultCapacity|unitSize|chunkcount:%u|%u|%u\n", defaultCapacity_, unit_, chunkCount_);
        return 0;
    }
    
    void* Malloc(size_t size){
        if(size > unit_)
            return NULL;
        
        ChunkHeader * chunk = GetFreeChunk(chunkHead_);
        if(chunk == NULL){
            if((chunk = AddChunk(defaultCapacity_)) == NULL)
                return NULL;
        }
        
        if(chunk->freeArea == NULL){
            chunk->freeArea = (FreeLinkList*)((char*)chunk + sizeof(ChunkHeader) + chunk->size);
            chunk->freeArea->next = NULL;
            //chunk->freeArea->size = unit_;
        }
        void* ret = chunk->freeArea;
        chunk->freeArea = chunk->freeArea->next;
        chunk->size += unit_;
        
        return ret;
    }
    
    void  Free(void *ptr){
        //ChunkHeader * chunk = FindChunk(ptr);
        ChunkHeader* chunk = chunkHead_;
        ChunkHeader* parent = NULL;
        while(chunk != NULL){
            if( (char*)ptr > (char*)chunk && (char*)ptr < ((char*)chunk+sizeof(ChunkHeader)+chunk->capacity))
                break;
            else if((char*)ptr < (char*)chunk){
                parent = chunk;
                chunk = chunk->left;
            }
            else{ 
                parent = chunk;
                chunk = chunk->right;
            }
        }
        if(chunk  == NULL)  // invalid ptr
            return;
        
        chunk->size -= unit_;
        if(chunk->size == 0 && chunkCount_ > defaultChunkMaxCount)
            FreeChunk(chunk, parent);
        
        ((FreeLinkList*)ptr)->next = chunk->freeArea;
        chunk->freeArea = (FreeLinkList*)ptr; 
        //chunk->freeArea.size = unit_;
        
    }
    
    //! not implement
    void* Realloc(void *ptr, size_t size){return NULL;}

private:
    ChunkHeader * AddChunk(UINT32 chunkCapacity)
    {
        UINT32 unitCount = (chunkCapacity+ unit_-1) / unit_;
        chunkCapacity = unitCount * unit_;
        
        ChunkHeader* newChunk=(struct ChunkHeader*)malloc(sizeof(ChunkHeader) + chunkCapacity);
        if(newChunk == NULL)
            return NULL;
        
        //init All member
        newChunk->left = NULL;
        newChunk->right = NULL;
        newChunk->size = 0;
        newChunk->capacity = chunkCapacity;
        newChunk->freeArea = NULL;
        
        ChunkHeader* curr = chunkHead_;
        ChunkHeader* prev = NULL;
        while(curr != NULL){
            prev = curr;
            if(newChunk < curr)
                curr = curr->left;
            else
                curr = curr->right;
        }
        if(prev == NULL){
            chunkHead_ = newChunk;
        }
        else{
            if(newChunk < prev)
                prev->left = newChunk;
            else
                prev->right = newChunk;
        }
        
        return newChunk;
    }
    
    ChunkHeader* GetFreeChunk(ChunkHeader* root)
    {
        if(root == NULL)
            return NULL;
        else if((root->capacity - root->size) >= unit_){
            return root;
        }
        
        ChunkHeader* chunk = GetFreeChunk(root->left);
        if(chunk != NULL){
            return chunk;
        }
        else
            return GetFreeChunk(root->right);
        
    }
    
    ChunkHeader* FindChunk(char* ptr)
    {
        ChunkHeader* root = chunkHead_;
        while(root != NULL){
            if( ptr > (char*)root && ptr < ((char*)root+root->capacity))
                break;
            else if(ptr < (char*)root)
                root = root->left;
            else 
                root = root->right;
        }
        return root;
    }
    
    void FreeChunk(ChunkHeader* delChunk, ChunkHeader* parent)
    {
        ChunkHeader* successor = delChunk;
        ChunkHeader* prev = parent;  //successor's parent
        if(delChunk->left != NULL && delChunk->right != NULL){ // TREE-SUCCESSOR
            prev = delChunk;
            successor = delChunk->left;
            while(successor->right != NULL){
                prev = successor;
                successor = successor->right;
            }
        }
        
        ChunkHeader *child = NULL;   //realDelChunk's only child, may be NULL
        if(successor->left != NULL)
            child = successor->left;
        else
            child = successor->right;
        
        if(successor == chunkHead_){  // and prev == NULL
            chunkHead_ = child;
        }
        else if(successor == prev->left){
            prev->left = child;
        }
        else{
            prev->right = child;
        }
        if(delChunk != successor){             
            successor->left = delChunk->left;
            successor->right = delChunk->right;
            if(parent == NULL)    //delChunk is root
                chunkHead_ = successor;
            else if(parent->left == delChunk)
                parent->left = successor;
            else
                parent->right = successor;
        }
        --chunkCount_;
        free(delChunk);
    }
    
    //!recursive postorder tree walk
    void Clear(ChunkHeader* root){
        if(root != NULL){
            Clear(root->left);
            Clear(root->right);
            free(root);
            --chunkCount_;
        }
    }
    
private:
    
    ChunkHeader*   chunkHead_;        //!< Head of the chunk linked-list.
    UINT32         defaultCapacity_;  //!< default chunk Capacity in bytes, without chunk head.
    UINT32         unit_;             //!< allocation Unit size in bytes.
    UINT32         chunkCount_;       //!< total chunks number
    
};
*/

} //namespace MemPool





#endif