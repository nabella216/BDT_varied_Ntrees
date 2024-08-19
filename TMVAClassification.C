// TMVAClassification.sh - Modified to train multiple BDT models with varing NTrees
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"

#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/Config.h"

#include "xjjcuti.h"
#include "xjjrootuti.h"
#include "TMVAClassification.h"

#include <iostream>
#include <string>
#include <TMVA/Factory.h>
#include <TMVA/DataLoader.h>
#include <TMVA/Tools.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TROOT.h>

void TMVAClassification(std::string signalInputFile, std::string backgroundInputFile, 
                        std::string cuts, std::string cutb, std::string outputDir,
                        std::string algo, int NTrees) {

    // Initialize TMVA
    TMVA::Tools::Instance();
    TFile *outputFile = TFile::Open((outputDir + "/TMVA_" + algo + "_NTrees" + std::to_string(NTrees) + ".root").c_str(), "RECREATE");
    
    // Initialize the factory
    TMVA::Factory *factory = new TMVA::Factory("TMVAClassification", outputFile,
                                                "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification");

    // DataLoader is used to load the datasets
    TMVA::DataLoader *dataloader = new TMVA::DataLoader("dataset");

    // Add variables
    dataloader->AddVariable("var1", 'F');
    dataloader->AddVariable("var2", 'F');
    // Add more variables here...

    // Load signal and background trees
    TFile *inputS = TFile::Open(signalInputFile.c_str());
    TFile *inputB = TFile::Open(backgroundInputFile.c_str());
    TTree *signalTree = (TTree*)inputS->Get("SignalTree");
    TTree *backgroundTree = (TTree*)inputB->Get("BackgroundTree");

    dataloader->AddSignalTree(signalTree, 1.0);
    dataloader->AddBackgroundTree(backgroundTree, 1.0);

    // Apply cuts
    dataloader->PrepareTrainingAndTestTree(cuts.c_str(), cutb.c_str(), "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V");

    // Book the BDT method with varying NTrees
    factory->BookMethod(dataloader, TMVA::Types::kBDT, "BDT",
                        Form("!H:!V:NTrees=%d:MinNodeSize=2.5%%:BoostType=AdaBoost:AdaBoostBeta=0.5:SeparationType=GiniIndex:nCuts=20", NTrees));

    // Train, test, and evaluate
    factory->TrainAllMethods();
    factory->TestAllMethods();
    factory->EvaluateAllMethods();

    // Clean up
    outputFile->Close();
    delete factory;
    delete dataloader;
    
    std::cout << "Training with NTrees=" << NTrees << " completed." << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " <signalInputFile> <backgroundInputFile> <cuts> <cutb> <outputDir> <algo> <NTrees>" << std::endl;
        return 1;
    }

    std::string signalInputFile = argv[1];
    std::string backgroundInputFile = argv[2];
    std::string cuts = argv[3];
    std::string cutb = argv[4];
    std::string outputDir = argv[5];
    std::string algo = argv[6];
    int NTrees = atoi(argv[7]);

    // Run TMVA Classification with the provided NTrees value
    TMVAClassification(signalInputFile, backgroundInputFile, cuts, cutb, outputDir, algo, NTrees);

    return 0;
}

