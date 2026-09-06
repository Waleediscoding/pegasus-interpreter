# Pegasus in Backus-Naur Form (BNF)

```
program ::= <statement>*

statement ::= <declaration> | <block> | <return> | <if> | <elif>
            | <while> | <for> | <print> | <input> | <keyword> | <exprStmt>

declaration ::= <varDeclaration> | <funcDeclaration>

varDeclaration ::= "var" IDENTIFIER (";" | ("=" <expression> ";"))

block ::= "{" <statement>* "}"

return ::= "return" (";" | <expression> ";")

if ::= "if" "(" <expression> ")" <statement> <elif>* ("else" <statement>)?

elif ::= "elif" "(" <expression> ")" <statement>

while ::= "while" "(" <expression> ")" <statement>

for ::= "for" "(" <varDeclaration> <exprStmt> <expression> ")" <statement>

print ::= "print" <expression> ";"

input ::= "input" VARIABLE ";"

keyword ::= ("break" | "continue") ";"

exprStmt ::= <expression> ";"

expression ::= <comma>

comma ::= <ternary> | <ternary> ("," <ternary>)+

ternary ::= <assignment> | <assignment> "?" <expression> ":" <ternary>

assignment ::= <equality> | VARIABLE ("=" | "+=" | "-=" | "*=" | "/=") <assignment>

equality ::= <comparison> | <comparison> ("!=" | "==") <comparison>

comparison ::= <term> | <term> (">" | ">=" | "<" | "<=") <term>

term ::= <factor> | <factor> ("+" | "-" | "or") <factor>

factor ::= <unary> | <unary> ("/" | "*" | "and") <unary>

unary ::= <call> | ("!" | "-") <unary>

call ::= <primary> | VARIABLE "(" <arguments> ")" 

arguments ::= <expression>? ("," <expression>)*

primary ::= <bool> | NIL | NUMBER | STRING | IDENTIFIER | <group>

bool ::= TRUE | FALSE

group ::= "(" <expression> ")"
```