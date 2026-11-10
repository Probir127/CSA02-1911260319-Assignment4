# CSA02 - C Programming - Assignment 4 - CO4

## Student and execution details
Student: PROBIR SAHA SHOHOM. Register number: 1911260319. Course: CSA02 - C Programming. Assignment4, CO4, 50marks.

Repository: https://github.com/Probir127/CSA02-1911260319-Assignment4

Modelling real-world data with structures and unions. This report covers all six tasks, the three supplied completed-code exercises, memory layouts, diagnoses, pseudocode, measured outputs and alternative designs. All assignment programs are C17. The gcc command on this Mac invokes Apple Clang21.0.0 (arm64); compiler.txt records the version. Commands use -Wall -Wextra -Wpedantic, and the shell runner also executes sanitizer checks. Fixed synthetic records make the demonstrations reproducible; memory addresses vary by run.

Pseudocode uses named ALGORITHM, INPUT, OUTPUT, BEGIN/END and structured IF/FOR blocks. The separate SIMATS pseudocode style guide is referenced by the brief but was not attached or publicly located, so exact typography-specific conformity to that separate guide has not been verified. Output previews are labelled renderings of saved execution transcripts; personal terminal screenshots and faculty submission remain separate steps.

## Task 1 - Data design review (6 marks)

### 1.1 Diagnose the poor design (2 marks)
| Deficiency | What can go wrong | Repair |
| --- | --- | --- |
| Related values in parallel arrays | Sorting age alone attaches another person's phone or BP to the wrong name | One Patient record contains all related fields |
| phone[10] for ten digits | The terminating null byte has no space; strcpy writes beyond the row | phone[11], ten validated digits, keep leading zeroes |
| One float for BP | Two independent measurements cannot be reconstructed reliably from 120/80 or a decimal encoding | BloodPressure has systolic and diastolic members |
| visitDate[8] as unvalidated text | Common dates need ten visible characters plus a terminator; malformed dates can be accepted | Date contains day, month and year, with calendar validation |
| No count capacity check | The 101st patient writes beyond the arrays and may corrupt other records | Check count before the copy |
| Unbounded strcpy on names | A long input can overwrite the next field or another row | Validate bounded, null-terminated source records before structure assignment |
| Mutable global arrays/count | Any source file with access can accidentally change the register; independent registers are awkward | Caller owns a PatientRegister and passes its pointer |
| No duplicate identity or field checks | The same ID may be registered twice; impossible dates or nonsensical data can enter the system | Positive unique ID and explicit field validation |

These are data-integrity problems, not simply syntax errors. Parallel arrays can be correct if every operation maintains their shared index, but the junior design provides no encapsulation to enforce that rule.

### 1.2 Repair with nested structures and a pointer interface (3 marks)
Patient contains id, name[64], age, phone[11], BloodPressure bp and Date visited. PatientRegister owns Patient patients[100] and a size_t count. The shared Date type is reused in the courier and canteen tasks. addPatient(PatientRegister *r, const Patient *p) checks pointers, capacity, field validity and duplicate ID before copying *p into the next slot and incrementing count. The source is const because adding a record must not modify it. No write occurs on validation failure.

The demonstration accepts age 0..130, systolic 1..300, diastolic 1..200 with systolic>=diastolic, and calendar dates in 1900..9999. These are explicit classroom input bounds, not a medical interpretation or diagnosis. Name length is at most 63 characters and phone contains exactly ten digits. memchr first establishes a terminator within each bounded array before strlen is used. Actual interactive input would need its own bounded parser; the program constructs and checks synthetic records directly.

```text
ALGORITHM AddPatient
INPUT: register pointer R, patient pointer P
OUTPUT: success or rejection
BEGIN
    IF a pointer is invalid OR R.count = capacity THEN RETURN rejection
    IF P has invalid fields THEN RETURN rejection
    FOR each existing patient in R
        IF existing.id = P.id THEN RETURN rejection
    END FOR
    R.patients[R.count] <- complete value of P
    R.count <- R.count + 1
    RETURN success
END
```

Measured Patient size is 104 bytes. The field sizes total 103 bytes, and one padding byte appears before BloodPressure at offset 84:
```patient_layout
0..3 id | 4..67 name | 68..71 age | 72..82 phone | 83 padding
84..91 BloodPressure | 92..103 Date
Total: 104 bytes. Each array element has this full layout.
```

The two initial records sort from Mina(age42), Arun(age19) into Arun, Mina while their associated fields stay together. The tests reject a duplicate ID, short phone, invalid leap date, invalid BP ordering, missing name terminator and the 101st insertion.

```text
2 Arun age=19 phone=9876543210 BP=110/70 date=09/11/2026
1 Mina age=42 phone=0123456789 BP=120/80 date=10/11/2026
sizeof Patient=104; 100 records=10400 bytes; count=100
Patient offsets: id=0 name=4 age=68 phone=72 bp=84 date=92
PASS: associated fields, invalid data, duplicate ID and capacity boundary.
```

### 1.3 Design defence (under 100 words; 1 mark)
A Patient structure keeps each person's name, age, phone, blood pressure and visit date together. Sorting moves complete records, so their fields cannot become misaligned. Functions accept a pointer to one record or register, avoiding long parameter lists and hidden global state. Nested types reuse date and blood-pressure rules. Adding another field changes the record definition rather than introducing another parallel array that every loop must remember to move. Validation at insertion preserves the register's invariants.

### Alternative: structure of arrays
For the same repaired fields, separate member arrays require a logical payload of 103*n bytes on this platform, versus 104*n bytes for Patient[n], excluding any containing object's alignment and the count. The junior arrays use less memory partly because they represent less data and have inadequate buffers, so that is not a fair like-for-like saving. For 100 equivalent records, payload is about 10300 versus 10400 bytes.

A whole-record swap conceptually makes three copies of sizeof(Patient), or 312 bytes here. Swapping every corresponding member of a structure-of-arrays design touches the equivalent member payload, about 309 bytes, plus control operations; moving only age corrupts the relationship. Actual machine traffic can differ after optimization. For b bytes per complete record and O(n log n) comparison sorting, movement can cost O(b*n log n); insertion sort used here is O(n^2) comparisons/moves worst-case, O(n) on sorted data, with one record temporary. Parallel arrays are useful for scans of a single field, vectorized numerical processing and cache efficiency. Sorting an index array can avoid moving either representation's large records, at O(n) extra index storage.

## Task 2 - Courier nested structures (8 marks)

### 2.1 Entities and nesting (2 marks)
| Entity | Fields / relationship |
| --- | --- |
| Date | day, month, year |
| Address | city[32], pin[7] |
| Person | name[64], nested Address |
| Parcel | ID, weight in grams, booking Date, sender Person, receiver Person, service enum, charge in integer paise |
| Daily bookings | Parcel bookings[100] plus count |

PIN is textual in the full design because it is an identifier, not a quantity; char[7] holds six digits plus null. Exact whole grams avoid float comparisons for the full application's weight. NORMAL and EXPRESS represent the finite service choices. Charge is stored in long long paise; the brief supplies no tariff, so the example stores illustrative charges rather than inventing a pricing formula. The provided complete-code exercise retains its int PIN and float weight exactly as supplied.

### 2.2 Structure definitions (2 marks)
```c
typedef struct { char city[32], pin[7]; } Address;
typedef struct { char name[64]; Address address; } Person;
typedef enum { NORMAL, EXPRESS } Service;
typedef struct {
    unsigned id, weight_grams;
    Date booked;
    Person sender, receiver;
    Service service;
    long long charge_paise;
} Parcel;
Parcel bookings[100];
```
The booking list is caller-owned. A counter application would validate unique positive ID, nonempty bounded strings, six PIN digits, valid date/service, positive weight and non-negative charge before insertion. This task's program uses fixed valid fixtures and calendar assertions; it is not an arbitrary-input parser.

### 2.3 Operation pseudocode (3 marks)
```text
ALGORITHM SortByWeight
INPUT: parcel array P with N records
OUTPUT: P in ascending weight order
BEGIN
    FOR I <- 0 TO N - 2
        LIGHTEST <- I
        FOR J <- I + 1 TO N - 1
            IF P[J].weight < P[LIGHTEST].weight THEN
                LIGHTEST <- J
            END IF
        END FOR
        IF LIGHTEST != I THEN
            TEMP <- P[I]
            P[I] <- P[LIGHTEST]
            P[LIGHTEST] <- TEMP
        END IF
    END FOR
END

ALGORITHM ListByReceiverCity
INPUT: parcel array P, count N, target CITY
OUTPUT: all parcels addressed to CITY
BEGIN
    FOR I <- 0 TO N - 1
        IF CompareStrings(P[I].receiver.address.city, CITY) = 0 THEN
            OUTPUT P[I]
        END IF
    END FOR
END
```
Selection sort performs n(n-1)/2 weight comparisons and at most n-1 swaps: O(n^2) time, O(1) record temporaries. It is not generally stable for equal weights; if booking order among ties matters, use stable insertion sort or add a booking sequence tie-breaker. The city scan costs O(n*L) in the worst case for city length L, with constant auxiliary storage; it uses exact case-sensitive matching here.

### 2.4 Access paths and execution (1 mark)
For the full data model, the third parcel's receiver PIN is bookings[2].receiver.address.pin. Given Parcel *p=bookings, it is (p+2)->receiver.address.pin, equivalently p[2].receiver.address.pin. In the supplied smaller model it is list[2].to.pincode or (p+2)->to.pincode with p pointing to struct parcel.

The supplied exercise fills only its TODOs and prints:

```text
502  1.2 kg  Chennai-600077  12/10/2026
503  2.8 kg  Coimbatore-641001  13/10/2026
501  4.5 kg  Madurai-625001  12/10/2026
```

The two swaps each use a temporary initialization and two assignments. With sizeof(struct parcel)=44, the conceptual payload is 3*44=132 bytes per swap and 264 bytes for both; this is a source-level copy count, not a hardware memory-traffic measurement. Three comparisons select the three records.

```text
Rich parcel size=240; flat size=240; comparisons=3 swaps=2
Supplied parcel size=44; logical bytes per swap=132; total two swaps=264
Rich parcel logical bytes per swap=720
Third receiver PIN via dot=625001; via pointer=625001
Receiver city filter: Chennai
ID=502 weight=1200 grams
PASS: whole-record sort and receiver-city filter.
```

### Alternative: flattened fields
FlatParcel replaces booked.day/month/year with day/month/year and replaces the Person/Address nesting with sender_name, sender_city, sender_pin and the corresponding receiver fields. A path such as p->receiver.address.pin becomes p->receiver_pin. Both measured layouts are 240 bytes with this field order and ABI: nesting itself does not require an extra pointer or dynamic allocation. Padding and member order can make other designs differ, so sizeof is measured rather than assumed. Access is O(1) in either layout. Reusing Person and Address gives one definition and one validation interface, which is the main design advantage of nesting.

## Task 3 - Unions and memory layout (9 marks)

### 3.1 Compare all-fields and tagged-union packets (3 marks)
An all-fields structure contains float celsius, int percent and char status[8] simultaneously. A tagged packet contains an enum kind and a union of those same alternatives. Since each packet carries exactly one value, the union shares storage efficiently; the tag records which interpretation the program may use. Updating a packet must update the tag and the corresponding payload as one logical operation.

An all-fields design can be useful if all three measurements are needed together. Here two fields would be irrelevant in every record. For a fair semantic comparison, TaggedAll adds an enum tag to the all-fields design, so both representations identify the active value explicitly.

### 3.2 Predict and measure sizes (2 marks)
The prediction assumes int, float and enum each occupy four bytes with four-byte alignment. The union needs enough space for its largest member, the eight-byte status buffer, rounded to the union alignment. The tag starts at offset0; the union starts at offset4. Total12 is already a multiple of four, so this layout needs no trailing padding.

| Design | Predicted bytes | Measured bytes |
| --- | --- | --- |
| AllFields (float + int + char[8]) | 16 | 16 |
| TaggedAll | 20 | 20 |
| Value union | 8 | 8 |
| Tagged Packet | 12 | 12 |

```union_layout
Packet bytes0..3: enum tag
Packet bytes4..11: shared eight-byte union storage
  float view: first4bytes (remaining bytes are not another payload)
  int view: first4bytes
  status view: all8bytes
The three views overlap; they are alternatives, not adjacent fields.
```
The structure is 4+max(4,4,8)=12, not 4+4+4+8. '64-bit machine' does not imply every member is eight bytes; pointer size is a separate property. Other ABIs, enum options, alignments or member types may change these results. For 10000 records the tagged union uses120000 bytes versus160000 for untagged AllFields or200000 for TaggedAll: savings40000 or80000 bytes respectively.

### 3.3 Tag-directed printing and validation (3 marks)
The completed show function switches on the tag and reads only its matching member, printing temperature to one decimal place, percent with a literal percent sign, or a status string. A default branch handles an invalid tag. The separate constructor set_status accepts at most seven characters, initializes the packet and terminates the buffer; an eight-character input is rejected without changing the previous packet. No dynamic allocation is needed for each record.

```text
sizeof union = 8, sizeof struct reading = 12
Temp 31.5 C
Humidity 68 %
Status OK
```

### 3.4 Inactive-member experiment (1 mark)
After storing31.5f, this host prints1107034112 when reading the same union through its int member. The little-endian float object bytes are00 00 fc 41, corresponding to IEEE binary32 bits0x41fc0000. This is a reinterpretation of representation, not the numerical conversion of31.5 to31 or a valid humidity reading. C's union rules allow this form of representation reinterpretation, but the value is not portable and some representations could be traps. It is inaccurate to promise the same integer on every system or to call it a legitimate tag-independent accessor. The normal application always follows the tag.

```text
int=4 float=4 enum=4 pointer=8
all-fields=16 tagged-all=20 union=8 tagged-union=12
union alignment=4 packet alignment=4 type offset=0 value offset=4
10000 packets: all-fields=160000 tagged-all=200000 tagged-union=120000 bytes
Misuse experiment on this platform: stored temperature=31.5; humidity member=1107034112
Float object bytes: 00 00 fc 41
Inactive-member result is representation-dependent, not a humidity conversion.
PASS: status length7 accepted; length8 rejected without changing packet.
```

### Alternative storage
Three separate typed arrays can use approximately4*T+4*H+8*S payload bytes for T temperature, H humidity and S status values on this ABI. Reading a known typed index is O(1). To reconstruct one chronological mixed stream, add a type/index mapping or timestamps; that metadata may erase the raw payload saving. Scanning one measurement type can have better locality.

A heap allocation per packet plus a void pointer still needs an external tag, a defined ownership rule and correct casts; void* does not remember a payload type. On this host the pointer alone uses8bytes, plus a tag, payload, alignment and allocator metadata. Access is O(1) after locating the packet but adds indirection and usually poorer locality. Allocator cost is implementation-dependent, with fragmentation and allocation failures to handle. A contiguous tagged-union array has predictable12-byte stride here, O(1) indexed access and O(n) total storage, making it appropriate for this constrained fixed-size packet stream. Its raw memory image is not a portable network serialization format; a wire protocol must define byte order, tags and encoding explicitly.

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

## Task 6 - Canteen inventory and billing (13 marks)

### Stage 1: requirements and invariants (2 marks)
The daily inventory holds at most32 items and128 sale records. Each item has a stable positive ID, bounded name, positive price in paise, kind and kind-specific details. A sale identifies an item, positive quantity, business date and time, and records its unit-price snapshot and total. Packaged items have expiry dates; cooked items have preparation date/time; combos list one to three ingredient entries, each an existing non-combo item ID and quantity.

| Requirement | Testable statement |
| --- | --- |
| Correct billing | Accepted sale amount equals quantity*price; daily total equals sum of accepted sales |
| Safe stock | No successful sale makes any stock negative; a failed sale changes no inventory field |
| Combo accounting | Aggregate repeated ingredient IDs before checking stock; charge combo revenue once, not again to ingredients |
| Expiry | Warn for packaged dates0..2days ahead inclusive; separately label expired stock and reject its sale |
| Cooked availability | Demonstration permits cooked items only on their preparation date and at/after preparation time |
| Daily boundaries | Sale date must equal this inventory's business date |
| Summary | Descending item revenue, with ascending item ID for ties |
| Capacity/overflow | Reject full item/sale arrays and arithmetic overflow before mutation |

The cooked-item rule is an explicit classroom assumption because the brief supplies no shelf-life policy. It is not a real food-safety guarantee. The program uses a fixed test date10/11/2026 for reproducible outputs. Prices are stored, not inferred from a tariff. Invalid requests return an explanation, and no rejected sale is added to the successful-sale log.

### Stage 2: data model (2 marks)
The complete types and APIs are in canteen.h. Item uses a common part (id, name, price, stock, kind, sales totals) and one union: packaged.expiry, cooked.prepared_date/prepared_time, or combo.count/parts[3]. The tag selects the valid alternative. Date and Time are nested structures; Part pairs ingredient ID and quantity; Sale captures the financial/time facts of one transaction. Inventory owns arrays of Item and Sale and their counts.
```canteen_model
Inventory owns Item[32] and Sale[128], business date and daily total.
Item has one tagged alternative: Packaged(Date) / Cooked(Date,Time) / Combo(Part[3]).
Part refers to a leaf Item by stable ID; it never points into a movable array.
Sale records item ID, quantity, unit price, total, date and time.
```
The measured Item size is112 bytes and Inventory size9768 bytes on this system. The Item layout is id0, name4, price48, stock56, tag60, union64, sold_units96, revenue104. Four padding bytes after name align the eight-byte price; the32-byte union accommodates the largest combo member. Reordering records or sorting a separate index list cannot invalidate ID-based recipes.

### Stage 3: critical decisions (2 marks)
A combo has no separately decremented stock balance: availability derives from all its ingredients. Nested combos are rejected, eliminating recursive cycles and simplifying bounded demand calculation. Repeated IDs within a recipe are allowed and combined. For example, a recipe with water quantities1and2 requires3waters per combo, not whichever entry happened to be checked last.

record_sale first builds a scratch demand array, checks every ingredient's availability and quantity, verifies sale-log capacity and all monetary/counter bounds, and only then applies deductions and totals. Nothing in its commit phase can fail. Snapshot assertions verify byte-for-byte equality after rejected operations. This gives atomic behaviour within the single-threaded function; it does not promise thread or crash safety. Multiple terminals would require a lock/transaction around the whole validation-and-commit sequence.

Date validation handles month lengths and Gregorian leap-year rules for1900..9999. Expiry distance is an ordinal-date difference, so the two-day window works across month/year boundaries without timezone or daylight-saving arithmetic. A packaged item expiring today is allowed under the explicit inclusive policy. A previous-day cooked item or future preparation time is unavailable in this demonstration.

No mutable global or static inventory is needed: main owns it and passes pointers. Module-local helper functions use static for internal linkage. Immutable capacity macros constrain the arrays; business state lives in the Inventory. Item insertion establishes valid tags, bounded terminated names, prices <=10^9paise, stock <=10^6 and component quantities <=10^6. Sale quantity <=10^6 makes component products <=10^12 before accumulation into uint64_t; additions, money products and daily totals are checked. The functions assume subsequent state changes occur through these validated APIs, apart from deliberate boundary-state setup in tests.

### Stage 4: function interfaces and pseudocode (4 marks)
```c
bool add_item(Inventory *inventory, const Item *item);
SaleResult record_sale(Inventory *inventory, unsigned id,
                       unsigned quantity, Date day, Time time);
void expiry_warnings(const Inventory *inventory);
void print_summary(const Inventory *inventory);
```
add_item copies a valid new record; record_sale mutates stock and sales; reporting accepts const pointers. Clients receive status values instead of partially updated records.

```text
ALGORITHM RecordSale
INPUT: inventory I, item ID, quantity Q, date D, time T
OUTPUT: accepted or a specific rejection reason
BEGIN
    IF Q, D or T is invalid OR D is not I.business_day THEN RETURN invalid
    FIND item ID; IF absent THEN RETURN unknown_item
    IF I.sale_count = capacity THEN RETURN log_full
    INITIALIZE all required quantities to zero
    IF item is a combo THEN
        FOR each recipe entry
            FIND its existing leaf ingredient
            ADD Q * entry.quantity to that ingredient's requirement
            IF arithmetic overflows THEN RETURN overflow
        END FOR
    ELSE
        SET sold item's requirement to Q
    END IF
    FOR each required ingredient
        IF it is unavailable on D at T THEN RETURN unavailable
        IF required quantity exceeds stock THEN RETURN insufficient_stock
    END FOR
    COMPUTE charge and CHECK all revenue and unit-count additions
    IF any arithmetic check fails THEN RETURN overflow
    FOR each item
        SUBTRACT its required quantity from stock
    END FOR
    UPDATE sold item's units and revenue and daily revenue
    APPEND one Sale containing ID, Q, unit price, charge, D and T
    RETURN accepted
END

ALGORITHM ExpiryWarnings
INPUT: valid inventory I
OUTPUT: expired and soon-expiring packaged item warnings
BEGIN
    FOR each packaged item P
        DAYS <- Ordinal(P.expiry) - Ordinal(I.business_day)
        IF DAYS < 0 THEN OUTPUT expired warning
        ELSE IF DAYS <= 2 THEN OUTPUT soon-expiring warning
        END IF
    END FOR
END
```
Revenue summary sorts indices rather than moving Item objects. For n items, linear ID lookup is O(n); with at most three recipe entries, demand building and checking remain O(n). The scratch array uses O(n) conceptual storage (fixed32 slots here). A sale-log append is O(1). Insertion sorting the summary indices is O(n^2) worst-case with O(n) index storage; n<=32 makes this simple choice adequate. Expiry scanning is O(n). Larger catalogues could use an ID index and an O(n log n) summary sort.

### Stage 5: executed demonstration and tests (3 marks)
| Accepted sale | Charge (paise) | Stock effects |
| --- | --- | --- |
| Water x2 | 4000 | Water10->8 |
| Breakfast x2 | 11000 | Water8->6; cooked8->4 |
| Cooked item x1 | 4000 | Cooked4->3 |
| Milk x1 | 2500 | Milk4->3 |
| Total | 21500 | Four successful log entries |

A subsequent request for two more breakfast combos needs four cooked units but only three remain. It is rejected before any water or cooked stock is deducted. The expired snack, unknown ID and zero quantity are also rejected. Summary ties at4000paise use item ID to keep output deterministic.

```text
Expiry warnings (inclusive today through two days ahead):
EXPIRING: 1 Water (in 2 days)
EXPIRED: 2 Snack (1 days overdue)
EXPIRING: 5 Milk (in 1 days)
Sale item=1 quantity=2: accepted
Sale item=4 quantity=2: accepted
Sale item=3 quantity=1: accepted
Sale item=5 quantity=1: accepted
Sale item=4 quantity=2: insufficient stock
Sale item=2 quantity=1: expired or not available at sale time
Sale item=99 quantity=1: unknown item
Sale item=1 quantity=0: invalid request
Daily summary: descending revenue; equal revenues ordered by item ID
ID=4 Breakfast    sold=2 revenue=Rs 110.00 stock=derived from components
ID=1 Water        sold=2 revenue=Rs 40.00 stock=6
ID=3 Idli         sold=1 revenue=Rs 40.00 stock=3
ID=5 Milk         sold=1 revenue=Rs 25.00 stock=3
ID=2 Snack        sold=0 revenue=Rs 0.00 stock=5
Sales=4 total=Rs 215.00
sizeof Date=12 Time=8 Item=112 Sale=48 Inventory=9768
Item offsets: id=0 name=4 price=48 stock=56 kind=60 union=64 sold=96 revenue=104
PASS: expected totals and stocks; every rejected sale left all inventory bytes unchanged.
```

Additional tests aggregate a repeated ingredient recipe (three waters per combo), reject four combos when only ten waters exist, then accept three combos and leave one water. They verify that only the combo receives revenue. Tests also reject nested or missing recipe references, zero recipe quantities, excessive part count, invalid tag, duplicate item ID, item capacity overflow, sold-item/daily-revenue overflow and a full sale log. Preparation-time, previous-day cooked, expiry-today, expired-ingredient, leap-day and year-boundary cases are covered. Rejected sales compare the complete inventory snapshot before and after.

```text
PASS: duplicate component demand aggregated; successful combo revenue counted once.
PASS: rejected sale snapshots unchanged for stock, expiry, time, capacity and overflow.
PASS: invalid recipes/tags/IDs and item capacity rejected; leap/year date boundaries verified.
PASS: expiry today allowed; expired ingredient and previous-day cooked item rejected.
```

### Alternative: linked-list inventory and remaining limits
A linked list can prepend an item in O(1), or append in O(1) with a maintained tail, once validation/search costs are excluded. Finding by ID is still O(n). Removing by ID costs O(n) to find the node/predecessor, then O(1) to unlink; removal is only O(1) when the necessary node links are already known. An array appends in O(1) while spare capacity exists and removes an interior record by O(n) shifts. Array capacity growth may require allocation/copying. Both need an O(n) uniqueness search without a separate index.

On this machine one next pointer adds8bytes per node, potentially more after alignment and allocator metadata. Separate nodes can have weaker locality and require allocation-failure/ownership handling. Stable IDs are still necessary for recipes and historical sales; removing a referenced ingredient must be prohibited or handled by an explicit archival rule. The current fixed array is suitable for a small bounded single-terminal exercise.

Remaining limits include in-memory storage lost on exit, no persistence/recovery, no concurrency control, one expiry/preparation batch per item ID, no refunds/taxes and a single business-day summary. Supporting several stock batches would need a Batch entity, per-batch expiry/quantity and an explicit allocation policy. These are future requirements rather than silently assumed capabilities of the implemented model.
## References
The supplied CSA02 CO4 Assignment4 document defines the task requirements. WG14 N1570, the public C11 committee draft, was consulted for the relevant C rules retained in these C17 examples, particularly storage duration/linkage, structures/unions, address-of constraints and function-call sequencing. Its section6.5.2.3 footnote95 discusses inactive union-member representation: https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf. Local sizeof, offsets, diagnostics and outputs in this report were measured with the compiler recorded in compiler.txt.
