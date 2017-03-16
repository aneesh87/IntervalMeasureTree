#include <stdio.h>
#include <stdlib.h>

//This is my comment

#define MAX(a,b) (a>b?a:b)

typedef int key_t;
typedef char object_t;
typedef struct text_t {key_t      key; 
                struct text_t   *left;
                struct text_t  *right;
                int height;
               /* possibly additional information */ } tree_node_t;

#define BLOCKSIZE 256            
tree_node_t *currentblock = NULL;
int size_left;
tree_node_t *free_list = NULL;
int nodes_taken = 0;
int nodes_returned = 0;

tree_node_t *get_node()
{ tree_node_t *tmp;
  nodes_taken += 1;
  if( free_list != NULL )
  {  tmp = free_list;
     free_list = free_list -> right;
  }
  else
  {  if( currentblock == NULL || size_left == 0)
     {  currentblock = 
                (tree_node_t *) malloc( BLOCKSIZE * sizeof(tree_node_t) );
        size_left = BLOCKSIZE;
     }
     tmp = currentblock++;
     size_left -= 1;
  }
  return( tmp );
}

void return_node(tree_node_t *node)
{  node->right = free_list;
   free_list = node;
   nodes_returned +=1;
}

void right_rotate (text_t * n) 
{

   text_t * tmp = n->right;
   n->right = n->left;
   n->left = n->left->left;
   n->right->left = n->right->right;
   n->right->right = tmp; 

   text_t * x = n->right;
   x->key = x->left->key + x->right->key;
   x->height = 1 + MAX(x->left->height, x->right->height);

   n->key = n->left->key + n->right->key;
   n->height = 1 + MAX(n->left->height, n->right->height);
}

void left_rotate (text_t * n) 
{
   text_t * tmp = n->left;
   n->left = n->right;
   n->right = n->right->right;
   n->left->right = n->left->left;
   n->left->left = tmp; 

   text_t * x = n->left;
   x->key = x->left->key + x->right->key;
   x->height = 1 + MAX(x->left->height, x->right->height);

   n->key = n->left->key + n->right->key;
   n->height = 1 + MAX(n->left->height, n->right->height);

}

void insert(tree_node_t *tree, key_t new_key, object_t *new_object)
{  
    int st_size = 200;
    //text_t ** stack = (text_t **) calloc(st_size, sizeof(text_t *));
    text_t * stack[200];
    tree_node_t *tmp_node;
    tmp_node = tree;
    int top = -1;
    while( tmp_node->right != NULL ) {
          if (top == st_size -1) {
              // TODO: realloc ??
              printf("Insert Failed due to Stack Overflow\n");
              return;
          }
          stack[++top] = tmp_node;
          tmp_node->key = tmp_node->key + 1;
          if( new_key <= tmp_node->left->key ) {
               tmp_node = tmp_node->left;
          } else {               
               new_key = new_key - tmp_node->left->key;
               tmp_node = tmp_node->right;
          }
    }
    /* found the candidate leaf. Test whether key distinct */ 
    /* key is distinct, now perform the insert */ 
    tree_node_t *old_leaf, *new_leaf;
    old_leaf = get_node();
    old_leaf->left = tmp_node->left; 
    old_leaf->key = tmp_node->key;
    old_leaf->right  = NULL;
    new_leaf = get_node();
    new_leaf->left = (tree_node_t *) new_object; 
    new_leaf->key = 1;
    new_leaf->right  = NULL; 
    tmp_node->left  = new_leaf;
    tmp_node->right = old_leaf;
    tmp_node->key  = 2;
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
  //free(stack);
}

object_t *_delete(tree_node_t *tree, key_t delete_key)
{  tree_node_t *tmp_node, *upper_node, *other_node;
   
   object_t *deleted_object;
   int st_size = 200;
   //text_t ** stack = (text_t **) calloc(st_size, sizeof(text_t *));
   text_t * stack[200];
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
          tmp_node->key = tmp_node->key - 1;
          upper_node = tmp_node;
          
          if(delete_key <= tmp_node->left->key ) {  
              tmp_node   = upper_node->left; 
              other_node = upper_node->right;
          } else { 
               delete_key = delete_key - upper_node->left->key;
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

void level_order (tree_node_t *tree) {
    text_t * queue[200];
    text_t * tmp = NULL;
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

void remove_tree(tree_node_t *tree)
{  tree_node_t *current_node, *tmp;
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

text_t * create_text();
void insert_line( text_t *txt, int index, char * new_line);
char * get_line( text_t *txt, int index);

int main()
{  
   
   tree_node_t *searchtree;
   char nextop;
   searchtree = create_text();
   printf("Made Tree\n");
   printf("In the following, the key n is associated wth the objecct 10n+2\n");
   while( (nextop = getchar())!= 'q' )
   { if( nextop == 'i' )
     { int inskey, success;
       char *strobj;
       strobj = (char *) malloc(sizeof(char) * 100);
       scanf(" %d", &inskey);
       fseek(stdin,0,SEEK_END);
       scanf ("%[^\n]%*c", strobj);
       insert_line( searchtree, inskey, strobj );
       printf("  insert line successful, key = %d, object value = %s, \n",
        	  inskey, strobj);
       level_order(searchtree);
     }  
     if(nextop == 'f' ) { 
        int findkey;
        char *findobj;
        scanf(" %d", &findkey);
        findobj = get_line( searchtree, findkey);
        if(findobj == NULL) {
           printf("  get line failed, for key %d\n", findkey);
        } else {
           printf("  get line successful, found object %s\n", findobj);
        }
     }
     if( nextop == 'd' )
     { int delkey;
       char *delobj;
       scanf(" %d", &delkey);
       delobj = _delete( searchtree, delkey);
       if( delobj == NULL )
         printf("  delete failed for key %d\n", delkey);
       else {
         printf("  delete successful, deleted object %s for key %d\n", delobj, delkey);
         level_order(searchtree);
       }
     }
   }
   remove_tree( searchtree );
   printf("Removed tree.\n");
   printf("Total number of nodes taken %d, total number of nodes returned %d\n",
	  nodes_taken, nodes_returned );
   return(0);
}

// the main functions

text_t * create_text() {
  text_t * new_text;
  new_text = get_node();
  new_text->left = ( text_t *) ("\0");
  new_text->key = 1;
  new_text->right = NULL;
  new_text->height = 0;
  return ( new_text );
}
int length_text( text_t *txt) {
  /* returns the number of lines of the current text. */
   if (txt == NULL) return -1;
   return (txt->key - 1); 
}

char * get_line( text_t *txt, int index) {
  /* gets the line of number index, if such a line exists, and
    returns NULL else.
  */
  if (txt == NULL || txt->key == 1 || index >= txt->key) {
     return NULL;
  }
  //printf("root height is %d key is %d\n",txt->height, txt->key);
  text_t *temp = txt;
  // later test and change to temp->key == 1 !!!!!!!!!!!!!!!! change to for ??
  while (temp->right != NULL){
      if (index > temp->left->key) {
          index = index - temp->left->key;
          temp = temp->right;
      } else {
          temp = temp->left;
     }
  }
  return (char *) temp->left;
}

void append_line( text_t *txt, char * new_line) {
  /* appends new line as new last line. */
  if (txt == NULL) return;
  insert(txt, txt->key, new_line);

}

char * set_line( text_t *txt, int index, char * new_line) {
  /* sets the line of number index, if such
    a line exists, to new line, and returns a pointer to the previous line of that number. If no line of
    that number exists, it does not change the structure and returns NULL.
  */
    if (txt == NULL || txt->key == 1 || index >= txt->key) {
     return NULL;
  }
  text_t *temp = txt;
  // later test and change to temp->key == 1 !!!!!!!!!!!!!!!! change to for ??
  while (temp->right != NULL){
      if (index > temp->left->key) {
          index = index - temp->left->key;
          temp = temp->right;
      } else {
          temp = temp->left;
     }
  }
  char * old  = (char *) temp->left;
  temp->left = (text_t *) new_line;
  return old;
}

void insert_line( text_t *txt, int index, char * new_line) {
  /* inserts the line before the line of
     number index, if such a line exists, to new line, renumbering all lines after that line. If no such
    line exists, it appends new line as new last line.
  */
    if (txt == NULL) return;
    if (index >= txt->key) {
        append_line(txt, new_line);
    } else {
        insert(txt, index, new_line);
    }
}

char * delete_line( text_t *txt, int index) {
  /* deletes the line of number index, renumbering all
    lines after that line, and returns a pointer to the deleted line.
  */
  if (txt == NULL) return NULL;
  
  return _delete(txt, index);  
}
