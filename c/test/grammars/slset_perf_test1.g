<x0> ::= "<b:1>" <x0><x1> | "<a:2>" <x1><x2> | "<>";
<x1> ::= "<a:2>" <x2> | "<b:5,a:1>" <x3> | "<>";
<x2> ::= "<b:2,a:1>" <x2><x3> | "<b:3>" <x2><x2> | "<a:2>";
<x3> ::= "<a:3,b:1>" <x2><x2> | "<a:2,b:2>" <x3><x3> | "<>";
