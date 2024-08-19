// https://root.cern.ch/doc/v610/TMVAGui_8cxx_source.html
// https://root.cern.ch/doc/v608/efficiencies_8cxx_source.html
#include <iostream>
#include <string>

#include <TKey.h>
#include <TList.h>
#include <TGraph.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include "TMVA/efficiencies.h"
#include "TMVA/mvas.h"
#include "TMVA/correlations.h"

#include "mvaeffs.h"
#include "TMVAClassification.h"
#include "xjjcuti.h"

namespace mytmva
{
  void guiefficiencies(std::string outputname, float ptmin, float ptmax, std::string mymethod, std::string stage = "0,1,2,3");
  void efficiencies(std::string outfname);
}

void mytmva::guiefficiencies(std::string outputname, float ptmin, float ptmax, std::string mymethod, std::string stage/* = "0,1,2,3"*/)
{
  std::string outfname = mytmva::mkname(outputname, ptmin, ptmax, mymethod, stage);
  mytmva::efficiencies(outfname);
}

void mytmva::efficiencies(std::string outfname)
{
  TString dataset("");
  std::string outputstr = xjjc::str_replaceallspecial(outfname);

  // set up dataset
  TFile* file = TFile::Open(outfname.c_str());
  if (!file)
  {
    std::cout << "==> Abort " << FUNCTION << ", please verify filename." << std::endl;
    return;
  }
  if (file->GetListOfKeys()->GetEntries() <= 0)
  {
    std::cout << "==> Abort " << FUNCTION << ", please verify if dataset exists." << std::endl;
    return;
  }
  // --->>
  if ((dataset == "" || dataset.IsWhitespace()) && (file->GetListOfKeys()->GetEntries() == 1))
  {
    dataset = ((TKey*)file->GetListOfKeys()->At(0))->GetName();
  }
  // <<---
  else if ((dataset == "" || dataset.IsWhitespace()) && (file->GetListOfKeys()->GetEntries() >= 1))
  {
    std::cout << "==> Warning " << FUNCTION << ", more than 1 dataset." << std::endl;
    file->ls();
    return;
  }
  else
  {
    return;
  }

  // Array of NTrees values
  std::vector<int> ntrees_values = {100, 200, 400, 600, 1000};
  std::vector<TGraph*> rocGraphs;

  // Load and draw ROC curves for each NTrees
  TCanvas* c1 = new TCanvas("c1", "ROC Curves", 800, 600);
  c1->cd();

  for (size_t i = 0; i < ntrees_values.size(); i++)
  {
    int ntrees = ntrees_values[i];
    TString rocFile = Form("%s/weights/TMVAClassification_%s_BDT_%d.weights.xml", dataset.Data(), dataset.Data(), ntrees);

    TFile* file = TFile::Open(rocFile);
    if (!file)
    {
      std::cerr << "Failed to open file: " << rocFile << std::endl;
      continue;
    }

    TGraph* roc = (TGraph*)file->Get("dataset/Method_BDT/BDT/MVA_BDT_rejBvsS");
    if (roc)
    {
      roc->SetLineColor(i + 1); // Assign a different color for each curve
      roc->SetLineWidth(2);
      rocGraphs.push_back(roc);

      if (i == 0)
        roc->Draw("AL"); // Draw the first curve
      else
        roc->Draw("L SAME"); // Overlay subsequent curves
    }

    file->Close();
  }

  // Add a legend to the canvas
  TLegend* legend = new TLegend(0.7, 0.2, 0.9, 0.4);
  for (size_t i = 0; i < ntrees_values.size(); i++)
  {
    legend->AddEntry(rocGraphs[i], Form("NTrees = %d", ntrees_values[i]), "l");
  }
  legend->Draw();

  // Save the canvas as an image
  c1->SaveAs("ROC_Curves_NTrees.png");

  // Cleanup
  delete c1;
  for (auto& graph : rocGraphs)
    delete graph;
}

int main(int argc, char* argv[])
{
  if (argc == 4)
  {
    for (int i = 0; i < mytmva::nptbins; i++)
    {
      mytmva::guiefficiencies(argv[1], mytmva::ptbins[i], mytmva::ptbins[i + 1], argv[2], argv[3]);
    }
    return 0;
  }
  return 1;
}
