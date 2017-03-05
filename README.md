# nif_stack

A little experimentation with Erlang nifs and Erlang.mk

``` shell
make
make shell

1> Stack = nif_stack:new().
<<>>
2> nif_stack:push(Stack, 42).
ok
3> nif_stack:peek(Stack).
{ok,42}
4> nif_stack:pop(Stack).
{ok,42}
5> nif_stack:peek(Stack).
{error,"Stack is empty."}
```
