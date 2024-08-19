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

// #include "xjjcuti.h"
#include "xjjrootuti.h"
#include "TMVAClassification.h"

void TMVAClassification() {

     // Add the include path for xjjcuti.h before including it
     gSystem->AddIncludePath("-I/afs/cern.ch/user/n/nnabiila/REDO/CMSSW_13_2_5_patch1/src/mvaHF/include");
     #include "xjjcuti.h"
    
     // Initialize TMVA
    TMVA::Tools::Instance();

    // Output file
    TFile* outputFile = TFile::Open("TMVAClassificationOutput.root", "RECREATE");

    // Create the factory object
    TMVA::Factory* factory = new TMVA::Factory("TMVAClassification", outputFile, 
                      "!V:!Silent:Color:DrawProgressBar:Transformations=I:AnalysisType=Classification");

    // Create the DataLoader object
    TMVA::DataLoader* dataloader = new TMVA::DataLoader("dataset");

    // Load your data here (replace with your data file paths and trees)
    TFile* input = TFile::Open("input_data.root");
    TTree* signalTree = (TTree*)input->Get("SignalTree");
    TTree* backgroundTree = (TTree*)input->Get("BackgroundTree");

    dataloader->AddSignalTree(signalTree, 1.0);
    dataloader->AddBackgroundTree(backgroundTree, 1.0);

    // Define the input variables that you want to use for training
    dataloader->AddVariable("var1", 'F');
    dataloader->AddVariable("var2", 'F');
    // Add more variables as needed

    // Set the weight expression for signal and background
    dataloader->SetSignalWeightExpression("weight");
    dataloader->SetBackgroundWeightExpression("weight");

    // Prepare the training and test tree
    dataloader->PrepareTrainingAndTestTree("", "", 
                                           "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V");

    // Array of NTrees values for multiple BDTs
    std::vector<int> ntrees_values = {100, 200, 400, 600, 1000};

    // Loop over the NTrees values and create a BDT for each
    for (size_t i = 0; i < ntrees_values.size(); i++) {
        int ntrees = ntrees_values[i];

        TString methodName = TString::Format("BDT_NTrees%d", ntrees);
        TString methodTitle = TString::Format("BDT with NTrees = %d", ntrees);
        TString options = TString::Format("!H:!V:NTrees=%d:MinNodeSize=2.5%%:BoostType=AdaBoost:AdaBoostBeta=0.5:MaxDepth=3", ntrees);

        factory->BookMethod(dataloader, TMVA::Types::kBDT, methodName, options);
    }

    // Train the methods
    factory->TrainAllMethods();

    // Test the methods
    factory->TestAllMethods();

    // Evaluate the methods
    factory->EvaluateAllMethods();

    // Save the output
    outputFile->Close();

    // Cleanup
    delete factory;
    delete dataloader;
    delete input;

    std::cout << "==> Finished TMVAClassification" << std::endl;
}
