  <typedecl>     ::= type <typedeflist>
  <typedeflist>  ::= <typedef> [ <typedeflist> ]
  <typedef>      ::= <typeid> = <typespec> ;
  <typespec>     ::= <typeid> |
                     <arraydef>  |  <ptrdef>  |  <rangedef>  | 
                     <enumdef>  |  <recdef>
  <typeid>       ::= <ident>

  <arraydef>     ::= [ packed ] array <lbrack> <rangedef> <rbrack> of <typeid>
  <lbrack>       ::= [
  <rbrack>       ::= ]

  <ptrdef>       ::= ^ <typeid>

  <rangedef>     ::= <number> .. <number>
  <number>       ::= <digit> [ <number> ]

  <enumdef>      ::= <lparen> <idlist> <rparen>
  <lparen>       ::= (
  <rparen>       ::= )
  <idlist>       ::= <ident> { , <ident> }

  <recdef>       ::= record <vardecllist> end ;
