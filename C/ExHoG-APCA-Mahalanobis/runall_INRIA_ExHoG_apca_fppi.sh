#!/bin/sh
#Train and test on INRIA
WIDTH=64; export WIDTH
HEIGHT=128; export HEIGHT

HardOption=" --poscases 0 --negcases 0"

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
CMD="$DumpRHOG $Option -s 1 positive_train_INRIA.lst $OutDir/train_pos_orig.RHOG "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

# get features on negative images
CMD="$DumpRHOG $Option -s 7 negative_train_INRIA.lst $OutDir/train_neg.RHOG --scaleratio 1.2 --pyramid 1"
echo $CMD >> $CMDLINE
$CMD >> $OutFile

# obtain covariance matrices for Mahalanobis distance classifier
CMD="$dump4mc -p $OutDir/train_pos_orig.RHOG -n $OutDir/train_neg.RHOG \
    $OutDir/mahalanobis.alt $OutDir/cov_apca.alt $OutDir/cov_ada.alt -r 200 -v 4 --alpha_weight 0.7 -a 1 --bootstrap 0"
echo $CMD >> $CMDLINE
$CMD >> $OutFile

echo First iteration complete

# create hard examples
HardDir=$OutDir/hard
CMD="mkdir -p $HardDir"
echo $CMD >> $CMDLINE
$CMD >> $OutFile

CMD="$ClassifyRHOG negative_train_INRIA.lst $HardDir/list.txt \
    $OutDir/mahalanobis.alt -d $HardDir/hard_neg.txt \
    -m 10 -t 10 --no_nonmax 1 --avsize 0 --margin 0 --scaleratio 1.05 -l N --use_bias 0 $Option --dumphard 1"
echo $CMD >> $CMDLINE
$CMD >> $OutFile

## now second iteration
# dump hard examples
CMD="$DumpRHOG $Option -s 0 $HardDir/hard_neg.txt \
    $OutDir/train_hard.RHOG $HardOption \
    --dumphard 1 --hardscore 34 --memorylimit 3000 "
echo $CMD >> $CMDLINE
$CMD >> $OutFile

echo Hard examples created

# obtain covariance matrices for Mahalanobis distance classifier
CMD="$dump4mc -n $OutDir/train_hard.RHOG \
    $OutDir/mahalanobis.alt $OutDir/cov_apca.alt $OutDir/cov_ada.alt -r 200 -v 4 --alpha_weight 0.7 -a 1 --bootstrap 1"
echo $CMD >> $CMDLINE
$CMD >> $OutFile

echo Second iteration complete

## Now test
echo Checking test images
ExtraOption2=" -t 0 -m 0 --avsize 0 --margin 0"
ExtraOption1=" -p 1 -z 8,16,2 --winstride 8 --no_nonmax 0 --nopyramid 0 --scaleratio 1.03 --use_bias 0"
ExtraOption=$ExtraOption1$ExtraOption2
OutDir=data-INRIA

ResultDir=$OutDir/wrkdir
ResultDir1=$OutDir/res/ExHoG_APCA
CMD="mkdir -p $ResultDir1"
echo $CMD >> $CMDLINE
$CMD >> $OutFile
CMD="$ClassifyRHOG $Option  $ExtraOption \
    test_INRIA.lst $ResultDir1 $ResultDir/mahalanobis.alt \
    --dumphard 0"
echo $CMD >> $CMDLINE
$CMD >> $OutFile