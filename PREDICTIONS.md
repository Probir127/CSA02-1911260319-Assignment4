# Predictions recorded before execution

Task1: After adding Mina(age42) and Arun(age19), age sorting should print Arun then Mina while keeping each phone/BP/date attached. Invalid phone/date/BP, duplicate ID and capacity101 must be rejected without changing the register.

Task2: The supplied parcels should sort as502,503,501. Selection sort needs3weight comparisons and2swaps. Assuming4-byte int/float and4-byte alignment, the supplied struct parcel should be44bytes; each three-assignment swap represents132logical bytes copied,264forbothswaps.

Task3: Assuming4-byte int,float,enum and4-byte alignment: union8bytes; all-fields structure16; tagged all-fields20; tagged union12. For31.5f on IEEEbinary32, the bits are0x41fc0000; interpreting them as a32-bit int gives1107034112 on this host, not a meaningful humidity.

Task4: Pass-by-value leaves balances5000,12000,800 unchanged. The local copy and original have distinct addresses while both live. Pointer and return-copy repairs should give5200,12480,832.

Task5: Automatic local and register local reinitialised to1000 each call give1001,1001,1001. Static and shared extern state give1001,1002,1003. Alternating independent veg/non-veg counters should give1001,1001,1002,1002. Single-printf argument order is unspecified; predict a permutation, not a guaranteed print order.

Task6: Fixed test day10/11/2026. Water2units at2000paise =>4000revenue; Breakfast combo2units at5500 =>11000; cooked item1unit at4000 =>4000; milk1unit at2500 =>2500. Total21500paise. Water stock10->8->6, cooked stock8->4->3, milk4->3. Overselling2morecombos requires4cooked items and must fail with every field unchanged. Expiry dates11Nov and12Nov are within2days;9Nov is expired. Summary order: combo11000, water4000, cooked4000, milk2500, expired snack0; ties break by item ID.
