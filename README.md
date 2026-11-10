# CSA02 - C Programming - Assignment 4 (CO4)

**Student:** PROBIR SAHA SHOHOM  
**Register number:** 1911260319  
**Course:** CSA02 - C Programming  
**Repository:** https://github.com/Probir127/CSA02-1911260319-Assignment4

All assignment programs are standard C17 source (`.c` and `.h`). The optional runner is Bash. The local `gcc` command is Apple Clang21.0.0 on arm64 macOS; see compiler.txt.

## Run everything

```sh
bash tools/run_all.sh
```

Keep assertions enabled (do not add -DNDEBUG). The script compiles and runs every valid program, verifies two expected compiler failures, and runs AddressSanitizer/UndefinedBehaviorSanitizer checks. It saves outputs in the task folders and executables in ignored build/. Subsequent runs can change memory addresses. Task03's representation experiment records this machine's inactive-union-member value; it is not a portable conversion routine.

## Manual commands

From the repository root, first run `mkdir -p build`. Exact per-program commands:

```sh
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task01/patient.c common/date.c -o build/task1
./build/task1
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task02/complete_code.c -o build/task2_complete
./build/task2_complete
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task02/parcels.c common/date.c -o build/task2
./build/task2
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task03/complete_code.c -o build/task3_complete
./build/task3_complete
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task03/layout.c -o build/task3
./build/task3
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task04/accounts.c -o build/task4
./build/task4
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task05/complete_code.c -o build/task5_complete
./build/task5_complete
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task05/demo.c Task05/counters.c Task05/other.c -o build/task5
./build/task5
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task06/main.c Task06/canteen.c Task06/demo_data.c common/date.c -o build/canteen
./build/canteen
gcc -std=c17 -Wall -Wextra -Wpedantic -O2 Task06/tests.c Task06/canteen.c Task06/demo_data.c common/date.c -o build/canteen_tests
./build/canteen_tests
```

Task04/compile_error.c and Task05/register_error.c deliberately fail compilation and are captured in compiler_error.txt; do not try to run them. The runner lists their exact commands and sanitizer options. execution_log.txt records the actual shell execution trace.

## Results

| Task | Result |
| --- | --- |
| 1 | 104-byte patient records; sorted associated fields and validation/capacity tests |
| 2 | Correct502/503/501 order;44-byte supplied parcel;132logical bytes per swap |
| 3 | 8-byte union,12-byte tagged packet; tag-directed printing and representation evidence |
| 4 | Distinct copy addresses; pointer/returned-copy balances5200,12480,832 |
| 5 | Local/static/extern/register comparison; independent veg/non-veg state and overflow check |
| 6 | Four accepted sales totalRs215.00; rejected sales leave stock, totals and log unchanged |

SOLUTIONS.md and the PDF contain the full report, memory diagrams, pseudocode and complexity comparisons. PREDICTIONS.md was committed before program execution. Each Task folder has source, fixture descriptions, raw output, explanation and a labelled preview image. The separate institutional pseudocode style guide was not supplied; the report uses explicit structured algorithm notation.

Canteen assumptions: fixed single-day inventory; combos contain at most three leaf entries, aggregate duplicate ingredient IDs, and have no separate stock; packaged expiry is inclusive; cooked stock is permitted only on its preparation day and after preparation time. These are demonstration policies. No mutable global inventory is used. Concurrency, persistence, multiple stock batches and refunds require further design.

Output previews render saved program transcripts. Capture personal execution screenshots when running the commands and share/submit the repository link with the faculty.
