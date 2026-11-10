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
