#!/bin/bash
source ../scripts/setCMSSW.sh

for leptonFlavor in muon electron; do
    #for algorithm in BDT MLP DNN; do
    for algorithm in DNN; do
        script=train_${leptonFlavor}_${algorithm}.sh
        > $script
        makeSubmit ${script} $PWD
        if [ "$algorithm" == "DNN" ]; then 
            echo "source /cvmfs/sft.cern.ch/lcg/views/LCG_88/x86_64-slc6-gcc62-opt/setup.sh" >> $script
        fi
        echo "./trainLeptonMva $leptonFlavor $algorithm" >> $script
        
        if [ "$algorithm" == "DNN" ]; then
            qsub $script -lnodes=1:ppn=16 -l walltime=80:00:00
            #qsub $script -l walltime=40:00:00
        else 
            qsub $script -l walltime=40:00:00
        fi
        rm $script
    done
done    
