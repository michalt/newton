<postalcode>           ::= <forwardsortationarea> <space> <localdeliveryunit>
<forwardsortationarea> ::= <provarea> <loctype> <letter>
<localdeliveryunit>    ::= <digit> <letter> <digit>
<provarea>             ::= A | B | C | E | G | H | J | K | L | M | N | 
                             P | R | S | T | V | X | Y
<loctype>              ::= <rural> | <urban>
<rural>                ::= 0
<urban>                ::= 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
<letter>               ::= A | B | C | E | G | H | J | K | L | M | N | 
                             P | R | S | T | V | W | X | Y | Z
<digit>                ::= 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9
