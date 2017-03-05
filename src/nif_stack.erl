-module(nif_stack).

-on_load(init/0).

-export([ new/0
        , push/2
        , peek/1
        , pop/1
        ]).

init() ->
    init("priv/nif_stack").

init(Lib) ->
    erlang:load_nif(Lib, 0).

new() ->
    not_loaded(?LINE).

push(_Stack, _Value) ->
    not_loaded(?LINE).

peek(_Stack) ->
    not_loaded(?LINE).

pop(_Stack) ->
    not_loaded(?LINE).

not_loaded(Line) ->
    exit({not_loaded, [{module, ?MODULE}, {line, Line}]}).
