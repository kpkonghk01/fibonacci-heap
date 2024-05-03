#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_COMMAND_LENGTH 512

typedef struct Node
{
  int key;
  int value;
  struct Node *parent;
  struct Node *children; // null or a circular doubly linked list
  struct Node *prev;
  struct Node *next;
  int degree;
  int mark;
} Node;

typedef struct FHeap
{
  struct Node *min; // null or a circular doubly linked list, also works as the entry of the root list
  int size;
} FHeap;

// function to print the list
void print_list(Node *head)
{
  Node *temp = head;

  if (temp == NULL)
  {
    printf("Empty list\n");
    return;
  }

  while (temp->next != head)
  {
    printf("->R(%d)%d", temp->key, temp->value);

    if (temp->children != NULL)
    {
      printf("\n->SC(%d)%d: ", temp->key, temp->value);
      print_list(temp->children);
    }
    temp = temp->next;
  }

  printf("->R(%d)%d->ER(%d)%d\n", temp->key, temp->value, head->key, head->value);
  if (temp->children != NULL)
  {
    printf("\n->LC(%d)%d: ", temp->key, temp->value);
    print_list(temp->children);
  }
}

// if all nodes in the root list have the minimum nodes, the max degree grows exponentially with base 1.618
int get_max_degree(int n)
{
  if (n == 0)
  {
    return 1;
  }

  return (int)ceil(log(n) / log(1.618)) + 1;
}

Node *create_node(int key, int value)
{
  Node *newNode = (Node *)malloc(sizeof(Node));
  newNode->key = key;
  newNode->value = value;
  newNode->parent = NULL;
  newNode->children = NULL;
  newNode->prev = newNode->next = newNode;
  newNode->degree = 0;
  newNode->mark = 0;

  return newNode;
}

FHeap *create_f_heap()
{
  FHeap *heap = (FHeap *)malloc(sizeof(FHeap));
  heap->min = NULL;
  heap->size = 0;
  return heap;
}

// merge two circular doubly linked list
void merge_lists(Node *list1, Node *list2)
{
  if (list1 == NULL || list2 == NULL)
  {
    return;
  }

  Node *tmp = list1->prev;
  list1->prev = list2->prev;
  list2->prev = tmp;

  list1->prev->next = list1;
  list2->prev->next = list2;
}

void remove_node_from_list(Node *node)
{
  if (node->next == node)
  {
    return;
  }

  node->prev->next = node->next;
  node->next->prev = node->prev;
  node->prev = node->next = node;

  return;
}

Node *deep_find(Node *head, int key, int value)
{
  Node *node = head;
  Node *start = node;

  if (node == NULL)
  {
    return NULL;
  }

  do
  {
    if (node->key == key && node->value == value)
    {
      return node;
    }

    Node *nested_found = deep_find(node->children, key, value);

    if (nested_found != NULL)
    {
      return nested_found;
    }

    node = node->next;
  } while (node != start);

  return NULL;
}

// Slow, may use hashtable to speed up
Node *find(FHeap *heap, int key, int value)
{
  if (heap->min == NULL)
  {
    return NULL;
  }

  Node *nested_found = deep_find(heap->min, key, value);

  if (nested_found != NULL)
  {
    return nested_found;
  }

  return NULL;
}

Node *shallow_find_min(Node *head)
{
  Node *node = head;
  Node *start = node;
  Node *min = node;

  do
  {
    if (node->key < min->key)
    {
      min = node;
    }

    node = node->next;
  } while (node != start);

  return min;
}

// to merge two nodes, we attach the node with smaller key to the larger one
// ref: https://youtu.be/6JxvKfSV9Ns?si=U-sT8C5aMeDAf8Sw&t=690
void merge_nodes(Node *node1, Node *node2)
{
  Node *parent;
  Node *child;

  if (node1->key < node2->key)
  {
    parent = node1;
    child = node2;
  }
  else
  {
    parent = node2;
    child = node1;
  }

  if (parent->children == NULL)
  {
    parent->children = child;
    parent->degree = 1;
  }
  else
  {
    merge_lists(parent->children, child);
    parent->degree++;
  }

  child->parent = parent;
}

// the return value can be used to implement find node via hashtable
Node *insert(FHeap *heap, int key, int value)
{
  Node *newNode = create_node(key, value);
  merge_lists(heap->min, newNode);

  if (heap->min == NULL || newNode->key < heap->min->key)
  {
    heap->min = newNode;
  }

  heap->size++;

  return newNode;
}

// ref: https://youtu.be/6JxvKfSV9Ns?si=j0mKKs0PPpQ_c0Bh&t=730
void consolidate(FHeap *heap)
{
  int max_degree = get_max_degree(heap->size);
  Node *TrackTable[max_degree];

  for (int i = 0; i < max_degree; i++)
  {
    TrackTable[i] = NULL;
  }

  Node *node = heap->min;
  Node *start = node;

  if (node == NULL)
  {
    return;
  }

  do
  {
    Node *next = node->next;
    int degree = node->degree;

    while (TrackTable[degree] != NULL)
    {
      Node *other = TrackTable[degree];

      if (node->key > other->key)
      {
        Node *temp = node;
        node = other;
        other = temp;
      }

      remove_node_from_list(other);
      merge_nodes(node, other);

      TrackTable[degree] = NULL;
      degree++;
    }

    TrackTable[degree] = node;
    node = next;
  } while (node != start);
}

void extract_min(FHeap *heap)
{
  Node *min = heap->min;

  if (min == NULL)
  {
    return;
  }

  if (min->children != NULL)
  {
    merge_lists(min, min->children);
    min->children->parent = NULL;
    min->children = NULL;
    min->degree = 0;
  }

  if (min != min->next)
  {
    heap->min = min->next;
  }
  else
  {
    heap->min = NULL;
  }

  remove_node_from_list(min);
  heap->size--;
  free(min);

  if (heap->min != NULL)
  {
    heap->min = shallow_find_min(heap->min);
  }

  consolidate(heap);
}

void extract(FHeap *heap)
{
  if (heap->min == NULL)
  {
    return;
  }

  Node *min = heap->min;

  printf("(%d)%d\n", min->key, min->value);

  extract_min(heap);
}

void cut(FHeap *heap, Node *node)
{
  Node *parent = node->parent;

  if (parent == NULL)
  {
    return;
  }

  Node *next = node->next;

  if (parent->children == node)
  {
    if (next == node)
    {
      parent->children = NULL;
    }
    else
    {
      parent->children = next;
    }
  }

  node->parent = NULL;
  remove_node_from_list(node);
  parent->degree--;

  merge_lists(heap->min, node);

  if (parent->mark == 0)
  {
    parent->mark = 1;
  }
  else
  {
    cut(heap, parent);
  }
}

void decrease(FHeap *heap, int key, int value, int decrease_by)
{
  Node *target = find(heap, key, value);

  if (target == NULL)
  {
    return;
  }

  target->key -= decrease_by;
  Node *parent = target->parent;

  if (parent != NULL && target->key < parent->key)
  {
    cut(heap, target);
  }

  if (target->key < heap->min->key)
  {
    heap->min = target;
  }
}

void delete_node(FHeap *heap, int key, int value)
{
  Node *target = find(heap, key, value);

  if (target == NULL)
  {
    return;
  }

  decrease(heap, key, value, 10000);
  extract_min(heap);
}

int main()
{
  // Create heap
  FHeap *heap = create_f_heap();

  if (feof(stdin))
  {
    fprintf(stderr, "Error: No input file provided.\n");
    return 1;
  }

  char line[MAX_COMMAND_LENGTH];
  while (fgets(line, MAX_COMMAND_LENGTH, stdin) != NULL)
  {
    line[strcspn(line, "\n")] = 0;

    char command[MAX_COMMAND_LENGTH];
    if (sscanf(line, "%s", command) == 1)
    {
      if (strcmp(command, "insert") == 0)
      {
        int key, value;
        if (sscanf(line, "%*s %d %d", &key, &value) == 2)
        {
          insert(heap, key, value);
        }
        else
        {
          fprintf(stderr, "Invalid insert command: %s\n", line);
        }
      }
      else if (strncmp(command, "delete", 6) == 0)
      {
        int key, value;
        if (sscanf(line, "%*s %d %d", &key, &value) == 2)
        {
          delete_node(heap, key, value);
        }
        else
        {
          fprintf(stderr, "Invalid delete command: %s\n", line);
        }
      }
      else if (strncmp(command, "decrease", 8) == 0)
      {
        int key, value, decrease_by;
        if (sscanf(line, "%*s %d %d %d", &key, &value, &decrease_by) == 3)
        {
          decrease(heap, key, value, decrease_by);
        }
        else
        {
          fprintf(stderr, "Invalid decrease command: %s\n", line);
        }
      }
      else if (strncmp(command, "extract", 7) == 0)
      {
        extract(heap);
      }
      else if (strncmp(command, "quit", 4) == 0)
      {
        break;
      }
      else
      {
        printf("Unknown command: %s\n", command);
      }
    }
  }

  return 0;
}