#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TCanvas.h"


void analyzer_script(){

  TFile * inFile = TFile::Open("tac2_021_single.root", "READ");
  TTree * tree = (TTree*) inFile->Get("tree");

  TCutG *cutg1 = new TCutG("lowerTail",7);
  cutg1->SetVarX("tracePara[0][2]");
  cutg1->SetVarY("tracePara[0][1]");
  cutg1->SetPoint(0,5.7545,58.4969);
  cutg1->SetPoint(1,5.47218,54.6429);
  cutg1->SetPoint(2,8.50409,62.1197);
  cutg1->SetPoint(3,8.76187,68.9029);
  cutg1->SetPoint(4,7.09247,64.7405);
  cutg1->SetPoint(5,5.88953,58.8052);
  cutg1->SetPoint(6,5.7545,58.4969);
  cutg1->SetLineColor(kRed);
  cutg1->SetLineWidth(2);
  

  TCutG * cutg2 = new TCutG("highTail",7);
  cutg2->SetVarX("tracePara[0][2]");
  cutg2->SetVarY("tracePara[0][1]");
  cutg2->SetPoint(0,5.87725,58.9594);
  cutg2->SetPoint(1,4.8707,60.501);
  cutg2->SetPoint(2,6.33142,77.536);
  cutg2->SetPoint(3,7.43617,78.8464);
  cutg2->SetPoint(4,6.87152,67.2842);
  cutg2->SetPoint(5,6.11047,60.7323);
  cutg2->SetPoint(6,5.87725,58.9594);
  cutg2->SetLineColor(kYellow);
  cutg2->SetLineWidth(2);

  TCanvas *c1 = new TCanvas("c1", "Trace Analysis", 2400, 800);

  c1->Divide(3,1);

  c1->cd(1);
  tree->Draw("tracePara[0][1]:tracePara[0][2]>>h1(400, 0, 12, 400, 30, 90)", "traceDetID==12107 && tracePara[0][0] > 500", "COLZ");
  // cutg1->Draw("same");
  // cutg2->Draw("same");

  c1->cd(2);
  tree->Draw("tracePara[0][1]:tracePara[0][2]>>h2(400, 0, 12, 400, 30, 90)", "traceDetID==12107 && tracePara[0][0] < 500", "COLZ");
  // cutg1->Draw("same");
  // cutg2->Draw("same");

  // c1->cd(2);
  // c1->cd(2)->Divide(1,2);

  // c1->cd(2)->cd(1);
  // tree->Draw("tracePara[0][1]:tracePara[0][0]>>h2(400, 100, 6000, 400, 30, 90)", "traceDetID==12107 && lowerTail", "COLZ");
  // c1->cd(2)->cd(2);
  // tree->Draw("tracePara[0][0]>>h2a(400, 100, 6000)", "traceDetID==12107 && lowerTail", "");

  // c1->cd(3);
  // c1->cd(3)->Divide(1,2);

  // c1->cd(3)->cd(1);
  // tree->Draw("tracePara[0][1]:tracePara[0][0]>>h3(400, 100, 6000, 400, 30, 90)", "traceDetID==12107 && highTail", "COLZ");
  // c1->cd(3)->cd(2);
  // tree->Draw("tracePara[0][0]>>h3a(400, 100, 6000)", "traceDetID==12107 && highTail", "");


}