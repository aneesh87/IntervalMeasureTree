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

// 4 conditions taken from textbook
void setMeasure(m_tree_t* node)
{
  // if leaf
  if(node->right == NULL){
    node->measure = MIN(node->rightmax, node->upper_val) - MAX(node->leftmin, node->lower_val);
    return;
  }  
  if(node->right->leftmin < node->lower_val && node->left->rightmax >= node->upper_val)
      node->measure = node->upper_val - node->lower_val;
  if(node->right->leftmin >= node->lower_val && node->left->rightmax >= node->upper_val)
      node->measure = node->upper_val - node->key + node->left->measure;
  if(node->right->leftmin < node->lower_val && node->left->rightmax < node->upper_val)
      node->measure = node->right->measure + node->key - node->lower_val;
  if(node->right->leftmin >= node->lower_val && node->left->rightmax < node->upper_val)
      node->measure = node->right->measure + node->left->measure;
}

void setMinMax(m_tree_t* tree){
  if(tree->right != NULL)
    return;
  struct interval_list *head = (struct interval_list*) tree->left;
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
   x->lower_val = n->key;
   x->upper_val = n->upper_val;
   x->leftmin = MIN(x->left->leftmin, x->right->leftmin);
   x->rightmax = MAX(x->left->rightmax, x->right->rightmax);
   setMeasure(x);

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
   x->lower_val = n->lower_val;
   x->upper_val = n->key;
   x->leftmin = MIN(x->left->leftmin, x->right->leftmin);
   x->rightmax = MAX(x->left->rightmax, x->right->rightmax);
   setMeasure(x);
   
   n->height = 1 + MAX(n->left->height, n->right->height);

}

void insert(m_tree_t *tree, key_t new_key, struct interval_t T)
{  
    // Quick inline stack
    int st_size = 400;
    m_tree_t * stack[400];
    m_tree_t *tmp_node;
    tmp_node = tree;
    int top = -1;

    struct interval_list * x = (struct interval_list *) calloc(1, sizeof(struct interval_list));
    x->interval = T;
    // Empty Tree
    if (tree->left == NULL) {
      tree->key = new_key;
      tree->leftmin = T.a;
      tree->rightmax = T.b;
      tree->lower_val = INT_MIN;
      tree->upper_val = INT_MAX;
      tree->left = (m_tree_t *) x;
      setMeasure(tree);
      return;
    }
    while( tmp_node->right != NULL ) {
          if (top == st_size -1) {
              // TODO: realloc ??
              printf("Insert Failed due to Stack Overflow\n");
              return;
          }
          stack[++top] = tmp_node;
          if( new_key < tmp_node->key ) {
               tmp_node = tmp_node->left;
          } else {               
               tmp_node = tmp_node->right;
          }
    }
    /* found the candidate leaf. Test whether key distinct */ 
    if (new_key == tmp_node->key) {
        struct interval_list * temp = (struct interval_list * ) tmp_node->left;
        x->next = (struct interval_list *)temp;
        tmp_node->left = (m_tree_t *) x;
        tmp_node->leftmin = MIN(T.a, tmp_node->leftmin);
        tmp_node->rightmax = MAX(T.b, tmp_node->rightmax);
        setMeasure(tmp_node);
    } else {

        m_tree_t *old_leaf, *new_leaf;
        old_leaf = get_node();
        old_leaf->left = tmp_node->left; 
        old_leaf->key = tmp_node->key;
        old_leaf->right  = NULL;
        old_leaf->leftmin = tmp_node->leftmin;
        old_leaf->rightmax = tmp_node->rightmax;
        new_leaf = get_node();

        new_leaf->left = (m_tree_t *)x;    
        new_leaf->key = new_key;
        new_leaf->leftmin = T.a;
        new_leaf->rightmax = T.b;
        new_leaf->right  = NULL;

        if (tmp_node->key < new_key ) {
            tmp_node->left = old_leaf;
            tmp_node->right = new_leaf;
            tmp_node->key = new_key;
            old_leaf->lower_val = tmp_node->lower_val;
            old_leaf->upper_val = new_key;
            new_leaf->lower_val = new_key;
            new_leaf->upper_val = tmp_node->upper_val;
        } else {     
            tmp_node->left = new_leaf;
            tmp_node->right = old_leaf;
            old_leaf->lower_val = tmp_node->key;
            old_leaf->upper_val = tmp_node->upper_val;
            new_leaf->lower_val = tmp_node->lower_val;
            new_leaf->upper_val = tmp_node->key;
        }

    //set the heights and measures of the nodes
        setMeasure(old_leaf);
        setMeasure(new_leaf);
        setMeasure(tmp_node);
        tmp_node->height = 1;
        new_leaf->height = 0;
        old_leaf->height = 0;

        tmp_node->leftmin = MIN(tmp_node->left->leftmin, tmp_node->right->leftmin);
        tmp_node->rightmax = MAX(tmp_node->left->rightmax, tmp_node->right->rightmax);
    }
    // set Measures along the path to the root
    // save top value for balancing which may need to be done
    int temp_top = top;
    while(top >=0) {
      tmp_node = stack[top--];
      setMeasure(tmp_node);
      tmp_node->leftmin = MIN(tmp_node->left->leftmin, tmp_node->right->leftmin);
      tmp_node->rightmax = MAX(tmp_node->left->rightmax, tmp_node->right->rightmax);
    }
    // Tree may need to be balanced
    top = temp_top;
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

void _delete(m_tree_t *tree, key_t delete_key, struct interval_t T)
{  m_tree_t *tmp_node, *upper_node = NULL, *other_node = NULL;
   
   object_t *deleted_object;
   int st_size = 400;
   m_tree_t * stack[400];
   int top = -1;
   
   //Empty Tree
   if (tree->left == NULL) {
       return;
   }  
   tmp_node = tree;
   while (tmp_node->right != NULL ) {
       
       if (top == st_size -1) {
           // TODO: realloc ??
           printf("Delete Failed due to Stack Overflow\n");
           return;
        }
        stack[++top] = tmp_node;   
        upper_node = tmp_node;
          
        if(delete_key < tmp_node->key ) {  
           tmp_node   = upper_node->left; 
           other_node = upper_node->right;
         } else { 
           tmp_node   = upper_node->right; 
           other_node = upper_node->left;
        } 
   } 
   // Check if key to delete matches one in the leaf    
   if (delete_key != tmp_node->key) {
       // Delete Key not matched
       return;
    }

    // Delete the interval from leaf's interval list
    struct interval_list * t = (struct interval_list *) tmp_node->left;
    
    if (t && T.a == t->interval.a &&  T.b == t->interval.b) {
        tmp_node->left = (struct m_tree_t *) t->next;
        free(t);
    } else  {
        while (t) {
            if (t->next != NULL && t->next->interval.a == T.a && t->next->interval.b == T.b) {
                struct interval_list * temp = t->next;
                t->next = temp->next;
                free(temp);
                break;
             } else {
                 t = t->next;
             }
        }
    }
    /* 
     * If all intervals have been deleted then we need to delete the leaf as well 
     * Otherwise just resetting measure and leftmin rightmax is enough
     */
    if (tmp_node->left != NULL) {
        setMinMax(tmp_node);
        setMeasure(tmp_node);      
    } else {
        if (upper_node == NULL) {
            /* Tree will be empty after this delete, reset it */
            tree->key = 0;
            tree->left = NULL;
            tree->right = NULL;
            tree->height = 0;
            tree->measure = 0;
            tree->lower_val = 0;
            tree->upper_val = 0;
            tree->leftmin = 0;
            tree->rightmax = 0;
            return;
        }
        upper_node->left  = other_node->left;
        upper_node->right = other_node->right;
        upper_node->height = other_node->height;
        upper_node->key = other_node->key;
        upper_node->rightmax = other_node->rightmax;
        upper_node->leftmin = other_node->leftmin;
        /* 
         * If new upper node is not a leaf, then the upper and lower limits of its
         * children need to be changed.
         */
        if(upper_node->right != NULL){
           upper_node->right->upper_val = upper_node->upper_val;
           upper_node->left->lower_val = upper_node->lower_val;
        }       
        return_node( tmp_node );
        return_node( other_node );
        top--;
        setMeasure(upper_node);
    }
    //save the top value for balancing
    int temp_top = top;
    while(top >=0) {
      tmp_node = stack[top--];
      setMeasure(tmp_node);
      tmp_node->leftmin = MIN(tmp_node->left->leftmin, tmp_node->right->leftmin);
      tmp_node->rightmax = MAX(tmp_node->left->rightmax, tmp_node->right->rightmax);
    }
    top = temp_top;
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

// the main functions

m_tree_t * create_m_tree() {
  
  m_tree_t * new_tree;
  new_tree = get_node();
  
  new_tree->left = NULL;
  new_tree->right = NULL;
  new_tree->height = 0;
  new_tree->measure = 0;
  new_tree->leftmin = 0;
  new_tree->rightmax = 0;
  new_tree->upper_val = 0;
  new_tree->lower_val = 0;
  
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
    // Insert the two endpoints one by one
    insert(tree, lower,  y);
    insert(tree, upper,  y);
}

void delete_interval( m_tree_t *tree, int lower, int upper) {
  /* deletes the interval from the tree
  */
  if (tree == NULL || lower >= upper) return;
  
  struct interval_t y;
  y.a = lower;
  y.b = upper;
  //del the two endpoints one by one
  _delete(tree, lower, y); 
  _delete(tree, upper, y); 
}

void destroy_list(struct interval_list * t) {
  while (t != NULL) {
      //printf("deleted (%d %d)\n", t->interval.a, t->interval.b);
      struct interval_list *tmp = t; 
      t = t->next;
      free(tmp);
  }
}
/* Reference: sample remove_tree code provided for program 1 */
void destroy_m_tree(m_tree_t *tree)
{  m_tree_t *current_node, *tmp;
   if( tree->left == NULL )
      return_node( tree );
   else
   {  current_node = tree;
      while(current_node->right != NULL )
      {  if( current_node->left->right == NULL )
         {  
            /* Leaves have associated lists which need to be freed*/
            destroy_list((struct interval_list *)current_node->left->left);
            return_node( current_node->left );
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
      if (current_node->right == NULL) {
          destroy_list((struct interval_list *)current_node->left);
      }
      return_node( current_node );
   }
}
/*
 Functions to Test
 */
void level_order (m_tree_t *tree) {
    if (tree == NULL) {
      return;
    }
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
          printf("(key=%d height=%d measure=%d l=%d u=%d", tmp->key, tmp->height, tmp->measure, tmp->lower_val, tmp->upper_val);
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
