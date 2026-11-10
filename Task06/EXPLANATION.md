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
