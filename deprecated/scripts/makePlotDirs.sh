#!/bin/bash
mkdir -p ../plots/
cd ../plots/
mkdir -p ttgamma
mkdir -p ewkino
mkdir -p tZq
#set up ewkino plots
cd ewkino
mkdir -p dilepCR
cd dilepCR
#make directories for every run
mkdir 2017
for dir in all2017 RunB RunC RunD RunE RunF
    do mkdir -p 2017/$dir
done
mkdir 2016
for dir in all2016 RunB RunC RunD RunE RunF RunG RunH
    do mkdir -p 2016/$dir
done

for dir in ./*/*
    do for subdir in inclusive ee em mm same-sign-ee same-sign-em same-sign-mm
        do for jetDir in nJetsInclusive 1pt40Jet
            do for puDir in noPuW PuW
                do mkdir -p $dir/$subdir/${jetDir}/${puDir}
            done
        done
    done
done
cd ../..

#set up ttgamma plots
cd ttgamma
for dir in inclusive ee em mm
    do mkdir -p $dir
done
cd ..

#set up tZq plots 
cd tZq 
for dir1 in 2016 2017; do
    for dir2 in mllInclusive onZ offZ; do
        for dir3 in inclusive 0bJets_01Jets 0bJets_2Jets 1bJet_01jets 1bJet_23Jets 1bJet_4Jets 2bJets; do
            mkdir -p ${dir1}/${dir2}/${dir3}
        done
    done 
done
cd ..
