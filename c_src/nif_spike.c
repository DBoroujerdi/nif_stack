#include "erl_nif.h"

#define MAXSIZE 100

ErlNifResourceType *STACK_RESOURCE_TYPE;

ERL_NIF_TERM atom_ok;
ERL_NIF_TERM atom_error;

/* This will be called whenever the VM garbage collects a stack resource  */
void *
stack_destructor(ErlNifEnv *env, void *obj)
{
  /*
     We don't actually need to free up stack memory as they are allocated with enif_alloc_resource
     and so the VM will automatically dealocate those on GC.

     We may want to perform other tasks on descruction, such as mutate a struct stored in priv_data
     which is where data that is stored between calls is stored.
     Get priv_data by calling 'enif_priv_data(env);'
  */
}

typedef struct
{
  int stk[MAXSIZE];
  int top;
} Stack;

static ERL_NIF_TERM
new(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  Stack *stack;
  ERL_NIF_TERM result;

  /* Allocate the memory-managed resource object  */
  stack = (Stack*) enif_alloc_resource(STACK_RESOURCE_TYPE, sizeof(Stack));

  /* Initialise the stack */
  (*stack).top = -1;

  /* Create an opaque handle the stack - this is what will be returned to the Erlang VM */
  result = enif_make_resource(env, stack);

  /* But first we must release the stack reference created above. */
  enif_release_resource(stack);

  /* The resource is now owned by the Erlang VM and can be garbage collected when it chooses */
  return result;
}

static ERL_NIF_TERM
push(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  int new_value;
  Stack *stack;

  /* Expect arity 2 */
  if(argc != 2) {
    return enif_make_badarg(env);
  }

  /* Read the first agrument into the location at stack */
  if (!enif_get_resource(env, argv[0], STACK_RESOURCE_TYPE, (void**) &stack)) {
    return enif_make_badarg(env);
  }

  /* Read the second arg as int into the location at value */
  if(!enif_get_int(env, argv[1], &new_value)) {
    return enif_make_badarg(env);
  }

  /* Check the stack is full */
  if ((*stack).top == (MAXSIZE - 1)) {
    ERL_NIF_TERM msg = enif_make_string(env, "Stack is full.", ERL_NIF_LATIN1);
    return enif_make_tuple2(env, atom_error, msg);
  }

  (*stack).top = (*stack).top + 1;
  (*stack).stk[(*stack).top] = new_value;

  return atom_ok;
}

static ERL_NIF_TERM
peek(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  Stack *stack;
  ERL_NIF_TERM result;

  /* Expect arity 1 */
  if(argc != 1) {
    return enif_make_badarg(env);
  }

  /* Read the first agrument into the location at stack */
  if (!enif_get_resource(env, argv[0], STACK_RESOURCE_TYPE, (void**) &stack)) {
    return enif_make_badarg(env);
  }

  /* Check if the stack is empty */
  if ((*stack).top == -1) {
    ERL_NIF_TERM msg = enif_make_string(env, "Stack is empty.", ERL_NIF_LATIN1);
    return enif_make_tuple2(env, atom_error, msg);
  }

  /* Get the value at the top of the stack */
  result = enif_make_int(env, (*stack).stk[(*stack).top]);

  return enif_make_tuple2(env, atom_ok, result);
}

static ERL_NIF_TERM
pop(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
  Stack *stack;
  ERL_NIF_TERM result;
  int popped;

  /* Expect arity 1 */
  if(argc != 1) {
    return enif_make_badarg(env);
  }

  /* Read the first agrument into the location at stack */
  if (!enif_get_resource(env, argv[0], STACK_RESOURCE_TYPE, (void**) &stack)) {
    return enif_make_badarg(env);
  }

  /* Check if the stack is empty */
  if ((*stack).top == -1) {
    ERL_NIF_TERM msg = enif_make_string(env, "Stack is empty.", ERL_NIF_LATIN1);
    return enif_make_tuple2(env, atom_error, msg);
  }

  popped = (*stack).stk[(*stack).top];
  (*stack).top = (*stack).top - 1;

  /* Create an Erlang Int for the value to live in */
  result = enif_make_int(env, popped);

  return enif_make_tuple2(env, atom_ok, result);
}

/* Called when the nif module is loaded */
int
load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
  /* Not sure what these flags mean - http://erlang.org/doc/man/erl_nif.html#enif_open_resource_type */
  int flags = ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER;
  STACK_RESOURCE_TYPE = enif_open_resource_type(env, NULL, "stack", stack_destructor, flags, NULL);

  /* Initialise the ok and error atoms, we'll use these to communicate success/failures on api calls */
  atom_ok = enif_make_atom(env, "ok");
  atom_error = enif_make_atom(env, "error");

  if(STACK_RESOURCE_TYPE == NULL) return -1;
  return 0;
}

/* All the API functions and arities */
static ErlNifFunc nif_funcs[] = {
  {"new", 0, new},
  {"push", 2, push},
  {"peek", 1, peek},
  {"pop", 1, pop}
};

/* This is the magic macro to initialize a NIF library */
ERL_NIF_INIT(nif_stack, nif_funcs, &load, NULL, NULL, NULL)
