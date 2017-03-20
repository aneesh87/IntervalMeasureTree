#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

//This is my comment

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


typedef int key_t;
typedef char object_t;
struct m_tree_t {key_t      key; 
                 struct m_tree_t   *left;
                 struct m_tree_t  *right;
                 int height;
                 int lower_val;
                 int upper_val;
                 int leftmin;
                 int rightmax;
                 int measure;
                /* possibly additional information */ };

struct interval_t {
  int a;
  int b;
};

struct interval_list {
  struct interval_t interval;
  struct interval_list * next;
};

#define BLOCKSIZE 256            
m_tree_t *currentblock = NULL;
int size_left;
m_tree_t *free_list = NULL;
int nodes_taken = 0;
int nodes_returned = 0;

m_tree_t *get_node()
{ m_tree_t *tmp;
  nodes_taken += 1;
  if( free_list != NULL )
  {  tmp = free_list;
     free_list = free_list -> right;
  }
  else
  {  if( currentblock == NULL || size_left == 0)
     {  currentblock = 
                (m_tree_t *) malloc( BLOCKSIZE * sizeof(m_tree_t) );
        size_left = BLOCKSIZE;
     }
     tmp = currentblock++;
     size_left -= 1;
  }
  return( tmp );
}

void return_node(m_tree_t *node)
{  node->right = free_list;
   free_list = node;
   nodes_returned +=1;
}

void right_rotate (m_tree_t * n) 
{

   m_tree_t * tmp = n->right;
   int tmp_key = n->key;
   n->right = n->left;
   n->key = n->left->key;
   n->left = n->left->left;
   n->right->left = n->right->right;
   n->right->right = tmp; 

   m_tree_t * x = n->right;
   x->key = tmp_key;
   x->height = 1 + MAX(x->left->height, x->right->height);

   n->height = 1 + MAX(n->left->height, n->right->height);
}

void left_rotate (m_tree_t * n) 
{
   m_tree_t * tmp = n->left;
   int tmp_key = n->key;
   n->left = n->right;
   n->key = n->right->key;
   n->right = n->right->right;
   n->left->right = n->left->left;
   n->left->left = tmp; 

   m_tree_t * x = n->left;
   x->key = tmp_key;
   x->height = 1 + MAX(x->left->height, x->right->height);
   
   n->height = 1 + MAX(n->left->height, n->right->height);

}

void insert(m_tree_t *tree, key_t new_key, struct interval_t T)
{  
    int st_size = 200;
    m_tree_t * stack[200];
    m_tree_t *tmp_node;
    tmp_node = tree;
    int top = -1;
    while( tmp_node->right != NULL ) {
          if (top == st_size -1) {
              // TODO: realloc ??
              printf("Insert Failed due to Stack Overflow\n");
              return;
          }
          stack[++top] = tmp_node;
          if( new_key <= tmp_node->left->key ) {
               tmp_node = tmp_node->left;
          } else {               
               tmp_node = tmp_node->right;
          }
    }
    /* found the candidate leaf. Test whether key distinct */ 
    /* key is distinct, now perform the insert */ 
    m_tree_t *old_leaf, *new_leaf;
    old_leaf = get_node();
    old_leaf->left = tmp_node->left; 
    old_leaf->key = tmp_node->key;
    old_leaf->right  = NULL;
    new_leaf = get_node();
    /* 
       new_leaf->left = (m_tree_t *) new_object; 
    */
    new_leaf->key = new_key;
    new_leaf->right  = NULL;

    if (tmp_node->key < new_key ) {
        tmp_node->left = old_leaf;
        tmp_node->right = new_leaf;
        tmp_node->key = new_key;
    } else {     
        tmp_node->left = new_leaf;
        tmp_node->right = old_leaf;
    }

    //set the heights of the nodes
    tmp_node->height = 1;
    new_leaf->height = 0;
    old_leaf->height = 0;
    
    while (top >= 0) { 
        tmp_node = stack[top--];
        int prev_height = tmp_node->height;
        if (tmp_node->left->height - tmp_node->right->height == 2) { 
            if(tmp_node->left->left->height == tmp_node->right->height + 1) { 
              right_rotate(tmp_node);
            } else { 
              left_rotate(tmp_node->left);
              right_rotate(tmp_node);
            }
        } else if(tmp_node->right->height - tmp_node->left->height == 2) { 
                  if(tmp_node->right->right->height ==tmp_node->left->height + 1) { 
                     left_rotate(tmp_node);
                  } else { 
                      right_rotate(tmp_node->right);
                      left_rotate(tmp_node);
                  }
        } else { 
               tmp_node->height = 1 + MAX(tmp_node->left->height, tmp_node->right->height);
        }
        if(tmp_node->height == prev_height) break;
  }
}

object_t *_delete(m_tree_t *tree, key_t delete_key, struct interval_t T)
{  m_tree_t *tmp_node, *upper_node, *other_node;
   
   object_t *deleted_object;
   int st_size = 200;
   //m_tree_t ** stack = (m_tree_t **) calloc(st_size, sizeof(m_tree_t *));
   m_tree_t * stack[200];
   int top = -1;
   
   if (tree->key == 1 || delete_key >= tree->key) {
       return( NULL );
   } else  {  
       tmp_node = tree;
       while (tmp_node->right != NULL ) {
          if (top == st_size -1) {
              // TODO: realloc ??
              printf("Delete Failed due to Stack Overflow\n");
              return (NULL);
          }
          stack[++top] = tmp_node;   
          upper_node = tmp_node;
          
          if(delete_key <= tmp_node->left->key ) {  
              tmp_node   = upper_node->left; 
              other_node = upper_node->right;
          } else { 
               tmp_node   = upper_node->right; 
               other_node = upper_node->left;
          } 
       } 
       // upper_node->key   = 1;
       // printf("upper %d tmp %d other %d \n", upper_node->key, tmp_node->key, other_node->key);
       // printf("tmp obj %s\n", (char *)tmp_node->left);
       
       upper_node->left  = other_node->left;
       upper_node->right = other_node->right;
       upper_node->height = other_node->height;
       upper_node->key = other_node->key;
       deleted_object = (object_t *) tmp_node->left;
       return_node( tmp_node );
       return_node( other_node );

        top--;

        while (top >= 0) { 
           tmp_node = stack[top--];
           int prev_height = tmp_node->height;
           if (tmp_node->left->height - tmp_node->right->height == 2) { 
               if(tmp_node->left->left->height == tmp_node->right->height + 1) { 
                  right_rotate(tmp_node);
               } else { 
                  left_rotate(tmp_node->left);
                  right_rotate(tmp_node);
               }
           } else if(tmp_node->right->height - tmp_node->left->height == 2) { 
                     if(tmp_node->right->right->height ==tmp_node->left->height + 1) { 
                        left_rotate(tmp_node);
                     } else { 
                        right_rotate(tmp_node->right);
                        left_rotate(tmp_node);
                     }
           } else { 
                  tmp_node->height = 1 + MAX(tmp_node->left->height, tmp_node->right->height);
           }
           if(tmp_node->height == prev_height) break;
        }

       return( deleted_object );
    }
}


// the main functions

m_tree_t * create_m_tree() {
  
  m_tree_t * new_tree;
  new_tree = get_node();
  
  new_tree->left = NULL;
  new_tree->right = NULL;
  new_tree->height = 0;
  new_tree->measure = 0;
  new_tree->leftmin = INT_MIN;
  new_tree->rightmax = INT_MAX;
  
  return ( new_tree );
}

int query_length(m_tree_t *tree) {
  /* returns the measure of the current intervals set. */
   if (tree == NULL) return -1;
   return (tree->measure); 
}

void insert_interval( m_tree_t *tree, int lower, int upper) {
  /* inserts the interval into the tree
  */
    if (tree == NULL || lower >= upper) return;
    
    struct interval_t y;
    y.a = lower;
    y.b = upper;

    insert(tree, lower,  y);
    insert(tree, upper,  y);
}

// 4 conditions taken from textbook
void setMeasure(m_tree_t* node)
{
  // if leaf
  if(node->right == NULL){
    node->measure = MIN(node->rightMax, node->r) - MAX(node->leftMin, node->l);
    return;
  }  
  if(node->right->leftMin < node->l && node->left->rightMax >= node->r)
      node->measure = node->r - node->l;
  if(node->right->leftMin >= node->l && node->left->rightMax >= node->r)
      node->measure = node->r - node->key + node->left->measure;
  if(node->right->leftMin < node->l && node->left->rightMax < node->r)
      node->measure = node->right->measure + node->key - node->l;
  if(node->right->leftMin >= node->l && node->left->rightMax < node->r)
      node->measure = node->right->measure + node->left->measure;
}


void setMinMax(m_tree_t* tree){
  if(tree->right != NULL)
    return;
  struct interval_list *head = tree->right;
  int minimum = head->interval.a;
  int maximum = head->interval.b;
  while(head != NULL){
    if(head->interval.a < minimum)
      minimum = head->interval.a;
    if(head->interval.b > maximum)
      maximum = head->interval.b;
    head = head->next;
  }
  tree->leftmin = minimum;
  tree->rightmax = maximum;
}

void delete_interval( m_tree_t *tree, int lower, int upper) {
  /* deletes the interval from the tree
  */
  if (tree == NULL || lower >= upper) return;
  
  struct interval_t y;
  y.a = lower;
  y.b = upper;

  _delete(tree, lower, y); 
  _delete(tree, upper, y); 
}

void destroy_m_tree(m_tree_t *tree)
{  m_tree_t *current_node, *tmp;
   if( tree->left == NULL )
      return_node( tree );
   else
   {  current_node = tree;
      while(current_node->right != NULL )
      {  if( current_node->left->right == NULL )
         {  return_node( current_node->left );
            tmp = current_node->right;
            return_node( current_node );
            current_node = tmp;
         }
         else
         {  tmp = current_node->left;
            current_node->left = tmp->right;
            tmp->right = current_node; 
            current_node = tmp;
         }
      }
      return_node( current_node );
   }
}

/*
 Functions to Test
 */

void level_order (m_tree_t *tree) {
    m_tree_t * queue[200];
    m_tree_t * tmp = NULL;
    int i = 0;
    queue[0] = tree;
    queue[1] = NULL;
    int rear = 1;
    while (rear != -1) {
       tmp = queue[0];
       if (tmp == NULL) printf("Level completed\n\n");
       else {
          printf("(key=%d height=%d", tmp->key, tmp->height);
          if (tmp->right == NULL) {
              printf(" %s ",(char *)tmp->left);
          }
          printf("):");
       }
       for (i = 0; i< rear; i++) {
            queue[i] = queue[i+1];
       }
       rear = rear - 1;
       if (tmp == NULL) {if (rear>= 0) queue[++rear]= NULL;}
       else if (tmp->right != NULL) {
              queue[++rear] = tmp->left;
              queue[++rear] = tmp->right;
       }
    }
}

int main()
{  
   
   m_tree_t *searchtree;
   char nextop;
   searchtree = create_m_tree();
   printf("Made Tree\n");
   printf("In the following, the key n is associated wth the objecct 10n+2\n");
   while( (nextop = getchar())!= 'q' )
   { if( nextop == 'i' )
     { int x,y, success;
       scanf(" %d", &x);
       fseek(stdin,0,SEEK_END);
       scanf ("%d %d", &x, &y);
       insert_interval(searchtree, x, y);
       printf("  insert line successful, key = %d, key = %d, \n",
            x, y);
       level_order(searchtree);
     }  
     if(nextop == 'f' ) { 
        printf("query_length %d\n", query_length(searchtree));
     }
     if( nextop == 'd' )
     { int x,y;
       scanf(" %d", &x);
       scanf(" %d", &y);
       delete_interval( searchtree, x, y);
       level_order(searchtree);
     }
   }
   destroy_m_tree(searchtree);
   printf("Removed tree.\n");
   printf("Total number of nodes taken %d, total number of nodes returned %d\n",
    nodes_taken, nodes_returned );
   return(0);
}
