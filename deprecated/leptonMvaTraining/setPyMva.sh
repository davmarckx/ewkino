export LCGENV_PATH=/cvmfs/sft.cern.ch/lcg/releases
/cvmfs/sft.cern.ch/lcg/releases/lcgenv/latest/lcgenv -p LCG_91 --ignore Grid x86_64-slc6-gcc62-opt root_numpy > lcgenv.sh
#/cvmfs/sft.cern.ch/lcg/releases/lcgenv/latest/lcgenv -p LCG_93 --ignore Grid x86_64-slc6-gcc62-opt root_numpy > lcgenv.sh
#/cvmfs/sft.cern.ch/lcg/releases/lcgenv/latest/lcgenv -p LCG_88 --ignore Grid x86_64-slc6-gcc62-opt root_numpy > lcgenv.sh
echo 'export PATH=$HOME/.local/bin:$PATH' >> lcgenv.sh
source lcgenv.sh
