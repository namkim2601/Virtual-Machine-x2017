#!/bin/bash

# Trigger all your test cases with this script
test_diff() { 
    # $1 = test number, $2 = x2017 file to pass, $3 = expected output

    if [[ "$1" -lt "17" ]] || [[ "$1" -gt "26" ]]; then
        if [[ "$1" -eq "1" ]]; then
            ./vm_x2017 2> temp.out
        else
            ./vm_x2017 tests/$2 2> temp.out
        fi
    else
        ./vm_x2017 tests/$2 > temp.out
    fi


    if diff tests/$3 temp.out >/dev/null; then
        echo "TEST $1 PASSED"
    else
        echo "TEST $1 FAILED"
        expected=$(cat tests/$3)
        actual=$(cat temp.out)
        echo -e "Expected: $expected"
        echo -e "But was: $actual"
    fi
}

# Creating .x2017 files
cd tests
./encode_all_files.sh
cd - > /dev/null

# Run tests
# Negative test cases for parsing
test_diff 1 "Wrong Args" 1_wrongArgs.out 
test_diff 2 "doesn'tExist" 2_noFile.out
test_diff 3 3_emptyFile.x2017 3_emptyFile.out
test_diff 4 4_noRet.x2017 4_noRet.out
test_diff 5 5_noInstr.x2017 5_noInstr.out
test_diff 6 6_noFunc0.x2017 6_noFunc0.out
test_diff 7 7_sameLabel.x2017 7_sameLabel.out
test_diff 8 8_movDestVal.x2017 8_movDestVal.out
test_diff 9 9_calNotVal.x2017 9_calNotVal.out
test_diff 10 10_calDoesntExist.x2017 10_calDoesntExist.out
test_diff 11 11_refDestVal.x2017 11_refDestVal.out
test_diff 12 12_refSrcNotStk.x2017 12_refSrcNotStk.out
test_diff 13 13_addDestNotReg.x2017 13_addDestNotReg.out
test_diff 14 14_addSrcNotReg.x2017 14_addSrcNotReg.out
test_diff 15 15_notArgNotReg.x2017 15_notArgNotReg.out
test_diff 16 16_equArgNotReg.x2017 16_equArgNotReg.out

# Positive test cases for VM
test_diff 17 17_printVal.x2017 17_printVal.out
test_diff 18 18_printReg.x2017 18_printReg.out
test_diff 19 19_printStk.x2017 19_printStk.out
test_diff 20 20_printPtr.x2017 20_printPtr.out
test_diff 21 21_usePtr.x2017 21_usePtr.out
test_diff 22 22_refPtr.x2017 22_refPtr.out
test_diff 23 23_jump.x2017 23_jump.out
test_diff 24 24_subtract.x2017 24_subtract.out
test_diff 25 25_manyStkFrames.x2017 25_manyStkFrames.out
test_diff 26 26_manyCalls.x2017 26_manyCalls.out

# Negative test cases for VM
test_diff 27 27_outOfRam.x2017 27_outOfRam.out
test_diff 28 28_addPcTooBig.x2017 28_addPcTooBig.out
test_diff 29 29_addPcTooSmall.x2017 29_addPcTooSmall.out
test_diff 30 30_recurSubtractPc.x2017 30_recurSubtractPc.out


rm temp.out
