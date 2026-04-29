#include <assert.h>
#include <stdlib.h>
#define STACK_MAX 256
// The interpreter for the little language  - dynamically typed and has 2 types
// of objects : int & pairs

// Enum to identify the object's type
typedef enum { OBJ_INT, OBJ_PAIR } ObjectType;

typedef struct sObject {
  // tag
  ObjectType type;

  // tagged union used to define our 2 objects - it can be either of the 2
  union {
    // OBJ_INT
    int value;

    // OBJ_PAIR
    struct {
      struct sObject *head;
      struct sObject *tail;
    };
  };

} Object;


// A Model VirtualMachine to hold a stack
typedef struct {
    Object *stack[STACK_MAX];
    int stackSize;
} VM;



VM *newVM() {
    VM *vm = malloc(sizeof(VM));
    vm->stackSize = 0;
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



