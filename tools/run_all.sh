#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
CC=${CC:-gcc}
flags=(-std=c17 -Wall -Wextra -Wpedantic -O2)
compile(){ local exe=$1;shift;"$CC" "${flags[@]}" "$@" -o "build/$exe"; }
run(){ local output=$1;shift;"$@" > "$output" 2>&1;printf 'PASS %s\n' "$output"; }
"$CC" --version > compiler.txt
compile task1 Task01/patient.c common/date.c
run Task01/output.txt build/task1
compile task2_complete Task02/complete_code.c
run Task02/complete_code_output.txt build/task2_complete
compile task2 Task02/parcels.c common/date.c
run Task02/output.txt build/task2
compile task3_complete Task03/complete_code.c
run Task03/complete_code_output.txt build/task3_complete
compile task3 Task03/layout.c
run Task03/output.txt build/task3
for task in 4 5;do
    if [ "$task" = 4 ];then src=Task04/compile_error.c;else src=Task05/register_error.c;fi
    status=0
    "$CC" "${flags[@]}" -c "$src" -o "build/error_$task.o" > "Task0$task/compiler_error.txt" 2>&1 || status=$?
    if [ "$status" -eq 0 ];then printf 'Expected compile failure: %s\n' "$src" >&2;exit 1;fi
done
compile task4 Task04/accounts.c
run Task04/output.txt build/task4
compile task5_complete Task05/complete_code.c
run Task05/complete_code_output.txt build/task5_complete
compile task5 Task05/demo.c Task05/counters.c Task05/other.c
run Task05/output.txt build/task5
compile canteen Task06/main.c Task06/canteen.c Task06/demo_data.c common/date.c
run Task06/output.txt build/canteen
compile canteen_tests Task06/tests.c Task06/canteen.c Task06/demo_data.c common/date.c
run Task06/tests_output.txt build/canteen_tests
for task in 1 2 4 5 6;do
    case "$task" in
    1) sources=(Task01/patient.c common/date.c);;
    2) sources=(Task02/parcels.c common/date.c);;
    4) sources=(Task04/accounts.c);;
    5) sources=(Task05/demo.c Task05/counters.c Task05/other.c);;
    6) sources=(Task06/tests.c Task06/canteen.c Task06/demo_data.c common/date.c);;
    esac
    compile "sanitized_$task" -O1 -fsanitize=address,undefined -fno-omit-frame-pointer "${sources[@]}"
    run "Task0$task/sanitizer_output.txt" "build/sanitized_$task"
done
printf 'All C programs, expected diagnostics and validation checks passed.\n'
