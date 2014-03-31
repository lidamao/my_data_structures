#ifndef _PULBIC_SKIPLIST_H_
#define _PUBLIC_SKIPLIST_H_

/** one skip list implement, not support concurrent.
 *  \autor    lsf
 *  \date     2013-4
 *  \version  1.00
 */

#include <stdlib.h>

#define DEFAULT_MAX_LEVEL   16

//! @{
template <typename KEY, typename VALUE>
class SkipList{

public:    
    //!@name Constructors and destructor.
    //@{
    
    //! Default constructor, use Default max levels
    SkipList() : max_level(DEFAULT_MAX_LEVEL){}
    
private:
    //! Copy constructor is not permitted.
    SkipList(const SkipList& rhs);

public:    
    SkipList(int levels) : max_level(levels){}
    
    //! Destructor.
    /*!
       need to free node every level.
    */
    ~SkipList(){
        clear();
    }
    
    //@}
    
    //!@name operator
    //@{
    
    //! init a SkipList
    int init(){
        //construct header node, max_level-1 because forward[1] hold array[0] space
        int header_size = sizeof(struct Node) + sizeof(struct Node*) * (max_level-1); 
        header = (struct Node*)malloc(header_size);
        if(header == NULL)
            return -2;
        
        for(int i=0; i<max_level; i++){
            header->forward[i] = NULL; 
        }
                            
        //construct update node array
        update = (struct Node**)malloc(sizeof(struct Node*) * (max_level));
        if(update == NULL){
            free (header);
            return -2;
        }
        //init current level
        level = 0; 
        return 0;
    }
    
    //! search a node
    /*! 
        \param v output value when search key success 
        \param k search key
        \return 0 if success or -1 if failed.
    */
    int search(const KEY& k, VALUE& v){
                  
        Node* x = header;
        for(int i=level; i>=0; i--){
            //printf("GO lv%d:", i);
            while(x->forward[i] != NULL){
                //printf("%d -> ",x->forward[i]->key);
                if(x->forward[i]->key < k)
                    x = x->forward[i];
                else
                    break;
            }
            //! \todo break if forward[i]->key == key  
            //printf("\n");
        }
        x = x->forward[0];
        if(x !=NULL && x->key == k){
            v = x->value;
            return 0;
        }
        else
            return -1;
            
    }
    
    //! insert a node
     /*! 
        \param k insert key, support ==, < comparison.   
        \param v insert value, usually a pointer to real data.
        \return 0 if success or -1 if failed.
    */
    int insert(const KEY& k, const VALUE& v){
        Node* x = header;
        for(int i=this->level; i>=0; i--){
            while(x->forward[i] != NULL){
                if(x->forward[i]->key < k)
                    x = x->forward[i];
                else
                    break;
            }
            update[i] = x;  //not need clear update[], x is prev node at insert position
        }
        x = x->forward[0];
        //! \todo key conflicting action: update; to add other action by flag;
        if(x != NULL && x->key == k){
            x->value = v;   
            return 1;
        
        }
        else{
            //new node level
            int i_level = random_level();  //random level, 1 to MAX_LEVEL
            //--i_level : level begin at 0 
            x = (struct Node *)malloc(sizeof(struct Node)+ sizeof(struct Node*) * (--i_level));
            
            if(x == NULL)
                return -2;
            
            if(i_level > this->level){
                for(int j=this->level+1; j<=i_level; j++){
                    update[j] = header;
                }
                this->level = i_level;
            }
            
            //update linklist every insert level
            do{
                x->forward[i_level] = update[i_level]->forward[i_level];
                update[i_level]->forward[i_level]= x; 
                
            }while(--i_level >= 0);
            
            x->key = k;
            x->value = v;
            
            return 0;
        }
    }
    
    //! delete a node
    /*! 
        \param r_key key of delete node   
        \return 0 if success or -1 if failed.
    */
    int erase(const KEY& r_key){
        struct Node* x = header;
        for(int i=this->level; i>=0; i--){
            while(x->forward[i] != NULL){
                if(x->forward[i]->key < r_key)
                    x = x->forward[i];
                else
                    break;
            }
            update[i] = x;  //not need clear update[]
        }
        x = x->forward[0];
        if(x != NULL && x->key == r_key){
            
            int lv = 0;    //update linklist every from 0 to x.level
            do{
                update[lv]->forward[lv] = x->forward[lv];
                ++lv;
            }while(lv <= this->level && update[lv]->forward[lv] == x);
            
            free(x);
            
            //x is the only top level, then level reduce 1 
            //notice x maybe the only second level, etc.. , so here is a while
            while(level > 0 && header->forward[level] == NULL){
                level--;
            }
            return 0;   //delete success
        }
        
        return -1; //not found key       
    }
    
    //! clear list
    /*! 
        \return 0 if success or -1 if failed.
    */
    int clear(){
        while(header->forward[0] != NULL){
            Node *x = header->forward[0];
            header->forward[0] = x->forward[0];
              
            free (x);
        }
        
        free (header);
        free (update);
        header = NULL;
        update = NULL;
        level = 0;
        
        return 0;
    }
    
    //! dump list
    void dump(){
        for(int i=this->level; i>=0; i--){
            printf("lv%d: ",i);
            Node *x = header->forward[i];
            Node *bottom = header->forward[0]; 
            while(x != NULL){
                while(bottom != x){
                    printf("------");
                    bottom = bottom->forward[0];
                }
                printf("%4d->", x->key);
                x = x->forward[i];
                bottom = bottom->forward[0];
            }
            printf("\n");
        }
    }
    
    //@}
    
private:
    //! make a random level
    int random_level(){
        int rand_lv = 1;
        int probability = 4;   // 1/2 probability
        
        static bool rand_init = false;   
        if(!rand_init){
            srand(time(NULL));
            rand_init = true;
        }
        
        //rand() is not thread-safe
        int max_rand_lv = this->max_level;
        
        //level increase 1 at most
        //int curr_lv = level+1;
        //int max_rand_lv = (curr_lv < max_level)? curr_lv+1 : max_level;  //fix the dice
                
        while((rand_lv < max_rand_lv) && (rand() % probability) == 0){
            ++rand_lv;
        }
        return rand_lv;
    }
    
private:
    //! skip list node
    struct Node{
        KEY     key;
        VALUE   value;
        struct Node* forward[1];     
    };
    
    //! only pointer
    /*struct PNode{
        struct Node* forward[1];
    }*/
    
    struct  Node*   header;       //!< 头结点
    int             level;        //!< 当前level
    int             max_level;    //!< 最大level
    struct  Node**  update;       //!< 插入或删除时临时prev数组
    
};

//! @}



#endif