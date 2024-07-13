# Compile time proofs in C++ a la curry howard

Every type that matches the `Expression` concept can be interpreted as a logical statement and to produce an object of that type is to prove that statement.
To prevent inconsistencies:
- functions may not use `Expression` type objects from global scope **except for** the static expression values in the `Axioms` class
- lambdas used in proofs must not capture variables (this is to ensure "freshness" of variables in universal quantifier introduction, and existential quantifier elimination)
- reinterpret casts, C style casts of expressions, and generally pointers to expressions are forbidden
- extern declarations of expressions are forbidden, since, at compile time, there is no way to check whether the object was actually constructed. The linker does not work as a check for this as none of the types store any run time values, so the compiler doesn't actually generate any loads or stores for these objects.

## Goals

### Make Statements and proofs about run time values

## Todo

### Proper Substitution restriction



### Generalized Modus Ponens

Understand and implement GMP for Implication construction.
Currently it