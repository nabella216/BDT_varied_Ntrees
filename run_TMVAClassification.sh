#!/bin/bash

trainlabel=20240701init
inputs=/afs/cern.ch/work/w/wangj/public/summer2024/mcfilelist_OfficialMCPthat5And8_GammaPlusGoing__add_zdcanalyzer-zdcdigi_sumPlus_sumMinus.root
bkgstrategy=sideband
inputb=/afs/cern.ch/work/w/wangj/public/summer2024/filelist_May07__add_Dfinder-ntDkpi_Dgen.root
inputms=(
    /afs/cern.ch/work/w/wangj/public/summer2024/filelist_May07__add_Dfinder-ntDkpi_Dgen.root
)
outputmvadir=/home/data/public/wangj/UPC/mva_output_${trainlabel}/

cut="pprimaryVertexFilter && vz > -15 && vz < 15 && nVtx <= 1 && cscTightHalo2015Filter > 0"
cut=$cut" && Dtrk1Pt > 0.9 && Dtrk2Pt > 0.9 && TMath::Abs(Dtrk1Eta) < 2.4 && TMath::Abs(Dtrk2Eta) < 2.4"
cut=$cut" && Dtrk1highPurity && Dtrk2highPurity && TMath::Abs(Dtrk1PtErr/Dtrk1Pt) < 0.1 && TMath::Abs(Dtrk2PtErr/Dtrk2Pt) < 0.1"
cut=$cut" && TMath::Abs(Dy) < 2.4 && Dchi2cl > 0.1"
cut=$cut" && DsvpvDisErr>1.e-5 && DsvpvDisErr_2D>1.e-5"

cuts=$cut
cutb=$cut

ntrees_values=(100 200 400 600 1000)

algo="BDT,BDTG,CutsGA,LD"
stages="0,1,2,5,6,7,12,13" 
sequence=0 

cuts=$cuts" && (Dgen == 23333  Dgen == 41022  Dgen == 41044)"
[[ $bkgstrategy == "sideband" ]] && cutb=$cutb" && TMath::Abs(Dmass-1.8648) > 0.05 && TMath::Abs(Dmass-1.8648) < 0.12"

output=rootfiles/TMVA_trainD0_${trainlabel}_${bkgstrategy}
[[ -d $output ]] && rm -r $output
[[ ${3:-0} -eq 1 ]] && mkdir -p $outputmvadir
tmp=$(date +%y%m%d%H%M%S)

[[ $# -eq 0 ]] && echo "usage: ./run_TMVAClassification.sh [train] [draw curves] [create BDT tree]"

# Compile any necessary macros (if needed) -- Commented out since ROOT macros are not compiled this way
#echo -e "\e[35m==> (1/5) building TMVAClassification.C\e[0m"
#make TMVAClassification || exit 1

# Run TMVAClassification.C within ROOT context for each NTrees value
for ntrees in "${ntrees_values[@]}"; do
    methodName="BDT_NTrees${ntrees}"
    output_n="${output}_${methodName}"

    if [[ ${1:-0} -eq 1 ]]; then
        root -l -b -q "TMVAClassification.C(\"$inputs\", \"$inputb\", \"$cuts\", \"$cutb\", \"$output_n\", \"$algo\", \"$stages\", $ntrees)"
    fi

    if [[ ${2:-0} -eq 1 ]]; then
        root -l -b -q "guivariables.C(\"$output_n\", \"$algo\", \"$stages\")"
        root -l -b -q "guiefficiencies.C(\"$output_n\", \"$algo\", \"$stages\")"
    fi

    for inputm in ${inputms[@]}; do
        if [[ ${3:-0} -eq 1 ]]; then
            root -l -b -q "mvaprod.C(\"$inputm\", \"Dfinder/ntDkpi\", \"$output_n\", \"$outputmvadir\", \"$algo\", \"$stages\")"
        fi
    done
done
