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
