#include <assert.h>
#include <stdlib.h>
#define STACK_MAX 256
#define INITIAL_GC_THRESHOLD 128
typedef struct sObject Object;
typedef struct VM VM;
void gc(VM *vm);
void mark(Object *object);
// The interpreter for the little language  - dynamically typed and has 2 types
// of objects : int & pairs

// Enum to identify the object's type
typedef enum { OBJ_INT, OBJ_PAIR } ObjectType;

typedef struct sObject {
  // VM's reference to objects (users can't see this) = Linked list
  struct sObject *next;

  // markbit
  unsigned char marked;

  // tag
  ObjectType type;

  // tagged union used to define our 2 objects - it can be either of the 2
  union {
    // OBJ_INT
    int value;

    // OBJ_PAIR
    struct {
      struct sObject
          *head; // can be an Object type object. - ie' an int or a pairObject
      struct sObject *tail;
    };
  };

} Object;

// A Model VirtualMachine to hold a stack
typedef struct VM {
  // Total number of currently allocated objects
  int numObjects;
  // Number of objects to trigger a GC
  int maxObjects;

  // Linked list head of the objects
  Object *firstObject;

  Object *stack[STACK_MAX];
  int stackSize;

} VM;

VM *newVM() {
  VM *vm = malloc(sizeof(VM));
  vm->stackSize = 0;

  // initialize first Object to NULL pointer
  vm->firstObject = NULL;
  vm->numObjects = 0;
  vm->maxObjects = INITIAL_GC_THRESHOLD;

  return vm;
}

// manipulating stack of VM
void push(VM *vm, Object *value) {
  assert(vm->stackSize < STACK_MAX && "Stack overflow!!");
  vm->stack[vm->stackSize] = value;

  vm->stackSize++;
}

Object *pop(VM *vm) {
  assert(vm->stackSize > 0 && "Stack underflow!!");

  vm->stackSize--;
  return vm->stack[vm->stackSize];
}

// Create objects :

////Helper
Object *newObject(VM *vm, ObjectType type) {
  if (vm->numObjects == vm->maxObjects) {
    gc(vm);
  }

  Object *object = (Object *)malloc(sizeof(Object));
  object->type = type;
  object->marked = 0;

  // adding the object to the start of the linked list
  object->next = vm->firstObject;
  vm->firstObject = object;

  vm->numObjects++;

  return object;
}

void pushInt(VM *vm, int intValue) {
  Object *object = newObject(vm, OBJ_INT);
  object->value = intValue;
  push(vm, object);
}

// we push objects onto the stack before doing pushPair function. So that
// pushPair takes items from the stack to build the object - Canonical stack
// based VM way of creating objects
Object *pushPair(VM *vm) {
  Object *object = newObject(vm, OBJ_PAIR);
  object->tail = pop(vm);
  object->head = pop(vm);

  push(vm, object);
  return object;
}

// Marking
//

void markAll(VM *vm) {
  for (int i = 0; i < vm->stackSize; i++) {
    mark(vm->stack[i]);
  }
}

void mark(Object *object) {
  // cyclical referencing break
  if (object->marked) {
    return;
  }

  object->marked = 1;

  if (object->type == OBJ_PAIR) {
    mark(object->head);
    mark(object->tail);
  }

  // if OBJ_INT, then do nothing
}

// Marking complete, now sweep
// SWEEP - all the unreachable objects (they are unreachable.. now what to do ?)
//
//  Trick to solve this = VM has it's own reference to objects that are distinct
//  from the semantics that are visible to the language user

// ie; we can keep track of them ourselves

// Maintain a linked list by modifying the Object struct(next) & VM
// struct(firstObject)

// NOTE: //POINTERS:  Think of the place/address as the box with the address,
// and inside - is the value; Pointer (which has that address as the value)
// point to that box(address) Always think of all Variables or Objects as value
// within a Box. And the box border has the address. Always think of arrows and
// boxes and inside the box, there is the value. And the box is situated inside
// wherever
//  that address is. ie; if the address is of the variable of an object, then
//  box is situated there.
void sweep(VM *vm) {
  // go through all the objects in the list (referenced = marked , unreferenced
  // = unmarked)
  Object **curr =
      &vm->firstObject; // double pointer because even if the middle object is
                        // the unmarked,then i can point the next of the
                        // previous object to the next of the unmarked object.
  while (*curr) {
    if (!(*curr)->marked) {
      Object *unreached = *curr;
      *curr = unreached->next;
      free(unreached);

      vm->numObjects--;

    } else {
      (*curr)->marked = 0;   // for next GC
      curr = &(*curr)->next; // variable is next. so curr's value points to the
                             // box(address) of next's value
    }
  }
}

/// IMPLEMENTATION
///// we will collect after a certain number of allocations
void gc(VM *vm) {
  markAll(vm);
  sweep(vm);

  // When numObjects reduce, maxObjects reduce, and viceversa - dynamic
  // maxObjects Lets our heap grow as the number of living objects increases &
  // vice versa
  vm->maxObjects = vm->numObjects * 2;
}

int main() {
  VM *vm = newVM();
  int i = 0;
  pushInt(vm, i);
  pushInt(vm, i + 1);
  pushInt(vm, i + 2);
  pushPair(vm);
  pushInt(vm, i + 3);
  pushPair(vm);
  pushPair(vm);
  pushInt(vm, i + 4);
}
