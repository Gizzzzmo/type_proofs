# Compile time proofs in C++ a la curry howard

Every type that matches the `Expression` concept can be interpreted as a logical statement and to produce an object of that type is to prove that statement.
To prevent inconsistencies:
- lambdas used in proofs must not capture variables (this is to ensure "freshness" of variables in universal quantifier introduction, and existential quantifier elimination)
- reinterpret casts, C style casts of expressions, and generally pointers to expressions are forbidden
- extern declarations of expressions are forbidden, since, at compile time, there is no way to check whether the object was actually constructed. The linker does not work as a check for this as none of the types store any run time values, so the compiler doesn't actually generate any loads or stores for these objects.
- there might be further problems that I haven't thought about

Should investigate whether these rules can easily checked with some custom clang-tidy rules.

## Runtime mapping ("tagging", WIP)
One can also create runtime "tagged" types that wrap some other type, and additionally contain a "tag" that expresses something about the runtime contents of the wrapped type.

For example, in logic.hpp there are the `Variable` types that are used to make statements about natural numbers (implementation of peano arithmetic).
These are things like `Zero`, `Succ<Zero>`, `FV<int, 0>`, `Plus<FV<int, 1>, Succ<Zero>>`, etc.
These types do not contain any runtime values.
We can make statements (`Expression` types) about them using the `Equals` predicate, thereby enabling proof checking at compile time.
In an optimized build, constructing and manipulating these types has no effect on the behavior of the program.

Tagging allows us to tag for example an `int` with one of these types above (specifically any type matching the `Variable<int>` concept).
A tagged type that wraps an integer value could then  for example be `Int<FV<int, 0>>`.
And the logic for creating these tagged types ensures that any statements we produce about the tag variables hold true for the runtime value.

So if we have a variable `x` of the tagged type above, and an element of type `Equals<FV<int, 0>, Succ<Zero>>`, then this is a guarantee that the value of x is exactly 1.
And it is possible to make arbirary statements.
So one could write a function that only makes sense if its input is for example a prime number, and guarantee at compile time that all callers really do only supply prime numbers.


## Goals

### Formalize a non trivial theorem in peano arithmetic and proof it
Maybe something like "5 is prime" to start with?

### Implement ZFC
Set theory axioms.

### Change int formalization from peano axioms to integer axioms (include negative numbers)

### Validate soundness of implementation

## Todo

### Make Function Dispatch work for tagged types


### Improve syntax

In particular substitution could probably be implemented with a constructor in MAKE_EXPRESSION macro.