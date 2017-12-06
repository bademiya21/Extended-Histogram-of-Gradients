#!/bin/sh
#Train and test on INRIA
WIDTH=64; export WIDTH
HEIGHT=128; export HEIGHT

HardOption=" --poscases 2416 --negcases 12057 "

BinDIR=./bin/
DumpRHOG=$BinDIR/dump_rhog
ClassifyRHOG=$BinDIR/classify_rhog
dump4mc=$BinDIR/dump4svmlearn
svm_learn=$BinDIR/svm_learn

Option=" -W $WIDTH,$HEIGHT \
    -C 8,8 -N 2,2 -B 18 -G 8,8 -S 0 --wtscale 2 --maxvalue 0.08 --epsilon 1 \
    --fullcirc 0 -v 3 --proc rgb_sqrt --norm l2hys --norm_exhog nonorm"

ExtraOption2=" -t 0 -m 0 "
ExtraOption1=" -p 1,0 --no_nonmax 0 -z 8,16,1.3 --cachesize 128 \
    --scaleratio 1.05 --winstride 8 --margin 4,4 --avsize 0,96 "
ExtraOption=$ExtraOption1$ExtraOption2
OutDir=data-INRIA/wrkdir

OutFile=$OutDir/record
CMDLINE=$OutDir/record


CMD="mkdir -p $OutDir"
$CMD 

# get features on positive images
CMD="$DumpRHOG $Option -s 1  positive_train_INRIA.lst $OutDir/train_pos.RHOG "
echo $CMD >> $CMDLINE
$CMD >> $OutFile
# get features on negative images
CMD="$DumpRHOG $Option -s 10 negative_train_INRIA.lst $OutDir/train_neg.RHOG "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

# dump to svmdense format
CMD=" $dump4mc -p $OutDir/train_pos.RHOG -n $OutDir/train_neg.RHOG \
    $OutDir/train_BiSVMLight.blt -v 4 "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

# learn
CMD="$svm_learn -j 3 -B 1 -z c -v 1 -t 0 $OutDir/train_BiSVMLight.blt \
    $OutDir/model_4BiSVMLight.alt "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

echo First iteration complete

# create hard examples
HardDir=$OutDir/hard
CMD="mkdir -p $HardDir"
echo $CMD >> $CMDLINE
$CMD >> $OutFile

CMD="$ClassifyRHOG negative_train_INRIA.lst $HardDir/list.txt \
    $OutDir/model_4BiSVMLight.alt -d $HardDir/hard_neg.txt -c $HardDir/hist.txt \
    -m 0 -t 0 --no_nonmax 1 --avsize 0 --margin 0 --scaleratio 1.05 -l N $Option --dumphard 1"

echo $CMD >> $CMDLINE
$CMD >> $OutFile

echo Hard examples created

## now second iteration
# dump hard examples
CMD="$DumpRHOG $Option -s 0 $HardDir/hard_neg.txt \
    $OutDir/train_hard_neg.RHOG $HardOption \
    --dumphard 1 --hardscore 0 --memorylimit 3700 "
echo $CMD >> $CMDLINE
$CMD >> $OutFile
    
# dump postive, negative and hard examples
CMD="$dump4mc -p $OutDir/train_pos.RHOG \
    -n $OutDir/train_neg.RHOG -n $OutDir/train_hard_neg.RHOG \
    $OutDir/train_BiSVMLight.blt -v 4 "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

# learn, second iteration
echo Doing second learning
CMD="$svm_learn -j 3 -B 1 -z c -v 1 -t 0 \
    $OutDir/train_BiSVMLight.blt \
    $OutDir/model_4BiSVMLight.alt "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

echo Second iteration complete

## Now test
echo Checking test images
ExtraOption2=" -t 0 -m 0 --avsize 0 --margin 0"
ExtraOption1=" -p 1 --no_nonmax 0 --nopyramid 0 --scaleratio 1.05 -l N"
ExtraOption=$ExtraOption1$ExtraOption2
OutDir=data-INRIA

ResultDir=$OutDir/wrkdir
ResultDir1=$OutDir/res/ExHoG
CMD="mkdir -p $ResultDir1"
echo $CMD >> $CMDLINE
$CMD >> $OutFile
CMD="$ClassifyRHOG $Option  $ExtraOption \
    test_INRIA.lst $ResultDir1 $ResultDir/model_4BiSVMLight.alt \
    --dumphard 0"
echo $CMD >> $CMDLINE
$CMD >> $OutFile