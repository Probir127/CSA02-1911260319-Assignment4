## Task 4 - Structure forensics (7 marks)

### 4.1 Defects (2 marks)
addInterest(struct account a, float rate) receives a value copy. Assigning a.balance modifies that copy and does not write back to list[i]. Separately, p is a pointer; p.number and p.balance are invalid member access expressions. Use p->number and p->balance, or (*p).number and (*p).balance. The compile-only reproduction includes stdio.h and records both member-access errors:

```text
Task04/compile_error.c:10:24: error: member reference type 'struct account *' is a pointer; did you mean to use '->'?
   10 |     printf("%d %f\n", p.number, p.balance);
      |                       ~^
      |                        ->
Task04/compile_error.c:10:34: error: member reference type 'struct account *' is a pointer; did you mean to use '->'?
   10 |     printf("%d %f\n", p.number, p.balance);
      |                                 ~^
      |                                  ->
2 errors generated.
```

### 4.2 Construct address evidence (2 marks)
The runnable diagnosis fixes the printing syntax but retains the ineffective by-value operation. It prints the original record's address and the callee's &a with %p and correctly cast pointer arguments. The addresses differ while both objects are alive. The local-copy address can be reused on later calls, which does not imply that the same object persists between calls. Exact addresses vary from run to run; their inequality and the unchanged balances are the relevant evidence.

```text
number=101 original=0x16af969a0 local-copy=0x16af96998 different=1
number=102 original=0x16af969a8 local-copy=0x16af96998 different=1
number=103 original=0x16af969b0 local-copy=0x16af96998 different=1
After by-value: 5000.00 12000.00 800.00
Pointer repair: 101 5200.00; returned copy: 5200.00
Pointer repair: 102 12480.00; returned copy: 12480.00
Pointer repair: 103 832.00; returned copy: 832.00
sizeof account=8 pointer=8; conceptual two-copy payload=16
PASS: original unchanged by value; pointer and return-copy repairs agree.
```

### 4.3 Repair using a structure pointer (2 marks)
```c
void addInterest(struct account *a, float rate) {
    a->balance += a->balance * rate / 100;
}
/* For each valid element: */
addInterest(&list[i], 4);
```
The pointer value is still passed by value, but it designates the original array element. Dereferencing it writes that object. The three results5200,12480,832 are asserted and match a return-copy alternative. These functions have the precondition of a valid account pointer and the fixed test rate. Float is retained to diagnose the supplied exercise; an actual monetary system should define rounding, range checks and an exact monetary representation instead of inheriting this classroom float model.

### 4.4 Passing guideline and alternative (1 mark)
Use by-value parameters for small independent snapshots, by-pointer parameters to update the caller's object, and const pointers to read a potentially large object without copying it. Pointer interfaces require documented validity/lifetime and ownership; const prevents modification through that pointer, not through every possible alias.

The return-copy alternative is list[i]=returned(list[i],4), where returned modifies its local account and returns it. At the source-level model this passes and returns sizeof(account) bytes,16 conceptual bytes for an8-byte record, versus passing an8-byte pointer. For a variable record size b, the copy work/state is O(b), versus O(1) for the pointer value, while the same field arithmetic occurs in both. With a fixed type both are O(1). ABIs may pass/return small records in registers and optimizers may eliminate copies, so neither exact stack usage nor a speed win follows from these source counts. Use mutation semantics and interface clarity first; measure generated code if performance matters.
