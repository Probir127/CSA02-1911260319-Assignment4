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
