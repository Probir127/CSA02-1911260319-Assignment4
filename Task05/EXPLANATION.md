## Task 5 - Storage classes and counters (7 marks)

### 5.1 Four implementations and outputs (3 marks)
An automatic local initialized to1000 starts again on every call, so it cannot provide persistent token numbers. A static local initialized to1000 retains its updated value throughout program execution. The extern version defines shared_counter once in counters.c and declares it in counters.h; demo.c and other.c call the same token function, demonstrating sharing across source files. The register keyword is shown on both a local counter and a printing-loop variable: it does not create persistence, so a register local initialized on every call also repeats1001.

The supplied completed-code exercise prints:

```text
static: 1001 1002 1003
auto  : 1001 1001 1001
```

The complete experiment prints:

```text
automatic: 1001 1001 1001
static: 1001 1002 1003
extern across two callers: 1001 1002 1003
register-local: 1001 1001 1001
static pair: 1001 1001 1002 1002
extern pair: 1001 1001 1002 1002
automatic pair still resets: 1001 1001 1001 1001
caller-owned pair with register loop: 1001 1001 1002 1002
single printf argument-order experiment: 1001 1002 1003
PASS: independent caller state and overflow rejection.
```

The baseline static/global examples use only a few calls within int range. The recommended caller-owned implementation also checks INT_MAX and rejects overflow without changing counter or output. Counters restart when the process restarts; static storage does not mean disk persistence.

### 5.2 Two independent counters and change effort (2 marks)
Define kind0=veg and kind1=non-veg. Alternating0,1,0,1 should yield1001,1001,1002,1002. The static pair and extern pair achieve that using two state elements. The automatic pair demonstration still returns1001 every time; adding a second automatic variable does not repair its lifetime. Moving both counters into the caller and passing the selected address provides the required lifetime and isolation.

| Version | Concrete state/API changes made | Result |
| --- | --- | --- |
| Automatic local | Replace per-call local state with one caller-owned int[2]; change function to accept a counter pointer and output pointer; change increment to dereference it | Independent1001,1001,1002,1002; state survives for caller's lifetime |
| Static local | Replace one static int by a static int[2]; add a kind parameter and bounds check; index the chosen element | Same independent sequence |
| extern global | Add one int[2] definition in counters.c and matching extern declaration in the header; add kind parameter/bounds check to pair function; callers supply kind | Same independent sequence across modules |
| register printing loop | Retain register on the loop index; add one caller-owned int[2] and select counters[i%2] for each call | Same independent sequence; register does not own persistent token state |

This records the changed declarations, interfaces and call sites rather than treating a formatting-dependent line count as effort. Baseline and modified functions are retained together so the change is directly inspectable. Separate duplicate functions such as veg_token and nonveg_token could each hide a static, but would duplicate logic and make future counters harder to add. The caller-owned design generalizes without new globals and is easier to reset and test.

### 5.3 Scope, lifetime, initialization and modification risk (2 marks)
| Storage form | Scope / linkage | Lifetime | Initial value without explicit initializer | Main risk |
| --- | --- | --- | --- | --- |
| auto local | Block scope, no linkage | Current block invocation | Indeterminate | Reset each call; pointer becomes invalid after lifetime ends |
| static local | Block scope, no linkage | Entire program | Zero | Hidden shared state, testing/reset and concurrency concerns |
| extern shared definition | Scope of visible declaration; shared external linkage here | Entire program | Definition with static duration defaults to zero | Any permitted module can modify it; multiple definitions cause linkage problems |
| register local | Block scope, no linkage | Automatic | Indeterminate | No persistence and no permission to take its address |

The examples explicitly initialize token state to1000. An extern declaration normally introduces a reference to the single definition rather than allocating another counter. Uninitialized automatic objects must not be read. register is an implementation-dependent optimization hint; the compiler may ignore the hint, but C17 still prohibits computing the address of a register object. The compile-only negative test demonstrates this rule:

```text
Task05/register_error.c:2:40: error: address of register variable requested
    2 | int main(void){register int i=0;int *p=&i;return *p;}
      |                                        ^~
1 error generated.
```

### Argument order and pointer-state alternative
printf("%d %d %d", next_token(), next_token(), next_token()) does not specify which argument call is evaluated first. This compiler printed1001,1002,1003 in this run, but another permitted evaluation order may print another permutation. The function executions do not interleave, so this is not the same unsequenced-modification bug as count=count++ +1. Assigning each token in a separate statement before printf makes the association with each printed position explicit and portable.

A caller-owned counter has O(1) update time and one int of storage per independent sequence; its lifetime is whichever owner the program chooses (automatic, static or allocated). Static/global forms also have O(1) operations, but hide ownership and share state. No example is thread-safe merely because its storage duration is static; simultaneous counters need synchronization, and cross-process durable tokens need persistent storage.
