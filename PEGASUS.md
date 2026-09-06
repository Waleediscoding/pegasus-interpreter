# Language Grammar & Syntax

## Grammar

The language is very minimalistic; in that it only consists of a few keywords similar to C, which are as follows:

```
and, break, continue, else, elif, false,

fn, for, if, nil, or, print, input, return,

true, var, while
```

## Syntax

The syntax of the language is reminiscent of any c-style syntax, with a few tweaks.

### Code Examples

```
// Single-line comment

/*
Multi-
Line
Comment
*/

// Variable declaration

var x = 3.1415;

x = "pi";


// Control Flow

    //  If Statement

    if (true)
        print "Always executes.";
    else
        print "Never executes.";


    // While Statement

    while (x == "pi")
        print "Prints forever.";


    // For Sttatement

    for (var i = 1; i <= 5; i += 1)
    {
        // Block instantiation
        print "i: " + i;
    }


// Functions

fn fact(n)
{
    if (n == 0) return 1;
    if (n == 1) return 1;

    return n * fact(n - 1);
}

// Calls

print fact(5); // Output: 120
```

You can find the entire grammar [here](AST.md).