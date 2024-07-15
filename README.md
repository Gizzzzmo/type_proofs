# Compile time proofs in C++ a la curry howard

Every type that matches the `Expression` concept can be interpreted as a logical statement and to produce an object of that type is to prove that statement.
To prevent inconsistencies:
- lambdas used in proofs must not capture variables (this is to ensure "freshness" of variables in universal quantifier introduction, and existential quantifier elimination)
- reinterpret casts, C style casts of expressions, and generally pointers to expressions are forbidden
- extern declarations of expressions are forbidden, since, at compile time, there is no way to check whether the object was actually constructed. The linker does not work as a check for this as none of the types store any run time values, so the compiler doesn't actually generate any loads or stores for these objects.
- there might be further problems that I haven't thought about
Should investigate whether these rules can easily checked with some custom clang-tidy rules.

## Goals

### Formalize a non trivial theorem in peano arithmetic and proof it
Maybe something like "5 is prime" to start with?

### Implement ZFC
Set theory axioms.

### Make Statements and proofs about run time values
Somehow tie these compile time proofs to the run time values to allow very simple formal program verification.
Will probably not work with control flow constructs, but should be thought about.

### Validate soundness of implementation

## Todo

### Generalized Modus Ponens

Understand and implement GMP for Implication construction.
Currently the fundamental logical calculus is incomplete, due to an inability to generate objects of substitutions.
For example it is not possible right now to proof things like `x = 1 and y = 2 implies x + y = 3`.