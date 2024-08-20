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


int TMVAClassification(std::string signalInputFile, std::string backgroundInputFile, 
                        std::string cuts, std::string cutb, std::string outputDir, float ptmin, float ptmax, std::string algo, int NTrees, std:string mymethod) 
{
  std::vector<std::string> methods;
  std::vector<int> stages;
  std::string outfname = mytmva::mkname(outputname, ptmin, ptmax, mymethod, stage, methods, stages);
  std::string outputstr = xjjc::str_replaceallspecial(outfname);
  if(ptmax < 0) { ptmax = 1.e+10; }

    // Initialize TMVA
    TMVA::Tools::Instance();

  // Default MVA methods to be trained + tested
  std::map<std::string,int> Use;

// Boosted Decision Trees
  Use["BDT"]             = 0; // uses Adaptive Boost
  Use["BDTG"]            = 0; // uses Gradient Boost
  Use["BDTB"]            = 0; // uses Bagging
  Use["BDTD"]            = 0; // decorrelation + Adaptive Boost
  Use["BDTF"]            = 0; // allow usage of fisher discriminant for node splitting


    TFile *outputFile = TFile::Open((outputDir + "/TMVA_" + algo + "_NTrees" + std::to_string(NTrees) + mymethod + ".root").c_str(), "RECREATE");
    
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

    // ------>>
  if(mymethod != "")
    {
      for(std::map<std::string,int>::iterator it = Use.begin(); it != Use.end(); it++) it->second = 0;
      for(auto& m : methods)
        {
          if(Use.find(m) != Use.end())
            { Use[m] = 1; std::cout <<"==> " << __FUNCTION__ << ": Registered method " << m << std::endl; }
          else
            { std::cout << "==> Abort " << __FUNCTION__ << ": error: unknown method " << m << "." << std::endl; continue; }
        }
    }
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

