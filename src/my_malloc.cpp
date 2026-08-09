#include <assert.h>
#include <my_malloc.h>

// A pointer to the head of the free list.
node_t *head = NULL;
node_t *tail = NULL;

// The heap function returns the head pointer to the free list. If the heap
// has not been allocated yet (head is NULL) it will use mmap to allocate
// a page of memory from the OS and initialize the first free node.
node_t *heap() {
  if (head == NULL) {
    // This allocates the heap and initializes the head node.
    head =
        (node_t *)mmap(NULL, HEAP_SIZE + sizeof(node_t), PROT_READ | PROT_WRITE,
                       MAP_ANON | MAP_PRIVATE, -1, 0);
    tail = (node_t *)((char *)head +
                      HEAP_SIZE);  // Set the tail to the end of the heap
    head->size =
        HEAP_SIZE - sizeof(node_t);  // The size does not include the header
    head->next = tail;
    tail->size = 0;
    tail->next = NULL;
  }

  return head;
}

// Reallocates the heap.
void reset_heap() {
  if (head != NULL) {
    munmap(head, HEAP_SIZE);
    head = NULL;
    heap();
  }
}

// Returns a pointer to the head of the free list.
node_t *free_list() { return head; }

// Calculates the amount of free memory available in the heap.
size_t available_memory() {
  size_t n = 0;
  node_t *p = heap();
  while (p != tail) {
    n += p->size;
    p = p->next;
  }
  return n;
}

// Returns the number of nodes on the free list.
int number_of_free_nodes() {
  int count = 0;
  node_t *p = heap();
  while (p != tail) {
    count++;
    p = p->next;
  }
  return count;
}

// Prints the free list. Useful for debugging purposes.
void print_free_list() {
  node_t *p = heap();
  while (p != tail) {
    printf("Free(%zd)", p->size);
    p = p->next;
    if (p != NULL) {
      printf("->");
    }
  }
  printf("\n");
}

// Finds a node on the free list that has enough available memory to
// allocate to a calling program. This function uses the "first-fit"
// algorithm to locate a free node.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// found - the node found on the free list with enough memory to allocate
// previous - the previous node to the found node
//
void find_free(size_t size, node_t **found, node_t **previous) {

  node_t *prev_node = NULL;
  node_t *this_node = heap();
  do {
    if (this_node->size + sizeof(node_t) >= size + sizeof(header_t)) {
      
      *found = this_node;
      *previous = prev_node;
      return;
    } else {
      prev_node = this_node;
      this_node = this_node->next;
    }
  } while (this_node != NULL);

  *found = NULL;
  *previous = NULL;
}

// Splits a found free node to accommodate an allocation request.
//
// The job of this function is to take a given free_node found from
// `find_free` and split it according to the number of bytes to allocate.
// In doing so, it will adjust the size and next pointer of the `free_block`
// as well as the `previous` node to properly adjust the free list.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
// previous - the previous node to the free block
// free_block - the node on the free list to allocate from
//
// RETURNS:
// allocated - an allocated block to be returned to the calling program
//
void split(size_t size, node_t **previous, node_t **free_block,
           header_t **allocated) {

  assert(*free_block != NULL);

  *allocated = (header_t *) *free_block;
  node_t old_node = **free_block;

  *free_block = (node_t *) (((char *) *free_block) + size + sizeof(header_t));

  (*free_block)->size = old_node.size - size - sizeof(header_t);
  (*free_block)->next = old_node.next;

  if (*previous == NULL) {
    head = *free_block;
  } else {
    (*previous)->next = *free_block;
  }
  

  (*allocated)->size = size;
  (*allocated)->magic = MAGIC;

}

// Returns a pointer to a region of memory having at least the request `size`
// bytes.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// A void pointer to the region of allocated memory
//
void *my_malloc(size_t size) {
  node_t *previous;
  node_t *found;
  find_free(size, &found, &previous);

  if (found == NULL) {
    return NULL;
  }

  header_t *allocated;
  split(size, &previous, &found, &allocated);

  return allocated + 1;
}

/*
 * Coalesces a free block with its neighbors if they are also free.
 *
 * The function assumes the free_list is sorted order. One way to do this is to
 * check the arrdress of the blocks, and insert the new block such that the
 * address of the new block is greater than the address of the previous block
 * and less than the address of the next block.
 *
 * PARAMETERS:
 * free_block - the block to coalesce
 *
 */
void coalesce(node_t *free_block) {
  node_t *next = head;
  node_t *prev = NULL;

  // first traverse the free list to find correct spot for free_block

  if (free_block < head) {
    head = free_block;
  } else {
    while (!(prev < free_block && free_block < next)) {
      prev = next;
      next = next->next;
    }
  }
  
  if ((char *) free_block + sizeof(node_t) + free_block->size == (char *) next) {
    free_block->size += (sizeof(node_t) + next->size);
    free_block->next = next->next;
  } else {
    free_block->next = next;
  }

  if (prev == NULL) { return; }

  if ((char *) prev + sizeof(node_t) + prev->size == (char *) free_block) {
    prev->size += (sizeof(node_t) + free_block->size);
    prev->next = free_block->next;
  } else {
    prev->next = free_block;
  }

}

// Frees a given region of memory back to the free list.
//
// PARAMETERS:
// allocated - a pointer to a region of memory previously allocated by my_malloc
//
void my_free(void *allocated) {

  header_t *header = ((header_t *) allocated) - 1;
  size_t abs_size = sizeof(header_t) + header->size;

  node_t *free_block = (node_t *) header;
  free_block->size = abs_size - sizeof(node_t);
  free_block->next = NULL;

  coalesce(free_block);
}
