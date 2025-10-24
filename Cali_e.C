#include <TFile.h>
#include <TTree.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TStyle.h>
#include <TProfile.h>
#include <TH2F.h>
#include <TH1F.h>
#include <TF1.h>
#include <TMath.h>
#include <TGraph.h>
#include <TLine.h>
#include <TSpectrum.h>

#define MAXEVENTSIZE 1000000

std::vector<std::vector<double>> combination(std::vector<double> arr, int r){
  
  std::vector<std::vector<double>> output;
  
  int n = arr.size();
  std::vector<int> v(n);
  std::fill(v.begin(), v.begin()+r, 1);
  do {
    //for( int i = 0; i < n; i++) { printf("%d ", v[i]); }; printf("\n");
    
    std::vector<double> temp;
    for (int i = 0; i < n; ++i) { 
      if (v[i]) {
        //printf("%.1f, ", arr[i]); 
        temp.push_back(arr[i]);
      }
    }
    //printf("\n");
    
    output.push_back(temp);
    
  } while (std::prev_permutation(v.begin(), v.end()));
  
  return output;
}

double* sumMeanVar(std::vector<double> data){
  
  int n = data.size();
  double sum = 0;
  for( int k = 0; k < n; k++) sum += data[k];
  double mean = sum/n;
  double var = 0;
  for( int k = 0; k < n; k++) var += pow(data[k] - mean,2);
  
  static double output[3];
  output[0] = sum;
  output[1] = mean;
  output[2] = var;
  
  return output;
}

std::vector<std::vector<double>> FindMatchingPair(std::vector<double> enX, std::vector<double> enY){

   //output[0] = fitEnergy;
   //output[1] = refEnergy;

   int nX = enX.size();
   int nY = enY.size();
   
   std::vector<double> fitEnergy;
   std::vector<double> refEnergy;
   
   if( nX > nY ){
      
      std::vector<std::vector<double>> output = combination(enX, nY);
      
      double * smvY = sumMeanVar(enY);
      double sumY = smvY[0];
      double meanY = smvY[1];
      double varY = smvY[2];
      
      double optRSquared = 0;
      double absRSqMinusOne = 1;
      int maxID = 0;
      
      for( int k = 0; k < (int) output.size(); k++){
        
        double * smvX = sumMeanVar(output[k]);
        double sumX = smvX[0];
        double meanX = smvX[1];
        double varX = smvX[2];
        
        double sumXY = 0;
        for( int j = 0; j < nY; j++) sumXY += output[k][j] * enY[j];
        
        double rSq = abs(sumXY - sumX*sumY/nY)/sqrt(varX*varY);
        
        //for( int j = 0; j < nY ; j++){ printf("%.1f, ", output[k][j]); }; printf("| %.10f\n", rSq);
        
        if( abs(rSq-1) < absRSqMinusOne ) {
          absRSqMinusOne = abs(rSq-1);
          optRSquared = rSq;
          maxID = k;
        }
      }
      
      fitEnergy = output[maxID];
      refEnergy = enY;
      
      printf(" R^2 : %.20f\n", optRSquared);      
      
      //calculation fitting coefficient
      //double * si = fitSlopeIntercept(fitEnergy, refEnergy);
      //printf( " y = %.4f x + %.4f\n", si[0], si[1]);
      
    }else if( nX < nY ){
    
      std::vector<std::vector<double>> output = combination(enY, nX);
      
      
      double * smvX = sumMeanVar(enX);
      double sumX = smvX[0];
      double meanX = smvX[1];
      double varX = smvX[2];
      
      double optRSquared = 0;
      double absRSqMinusOne = 1;
      int maxID = 0;
      
      for( int k = 0; k < (int) output.size(); k++){
        
        double * smvY = sumMeanVar(output[k]);
        double sumY = smvY[0];
        double meanY = smvY[1];
        double varY = smvY[2];
        
        double sumXY = 0;
        for( int j = 0; j < nX; j++) sumXY += output[k][j] * enX[j];
        
        double rSq = abs(sumXY - sumX*sumY/nX)/sqrt(varX*varY);
        
        //for( int j = 0; j < nX ; j++){ printf("%.1f, ", output[k][j]); }; printf("| %.10f\n", rSq);
        
        if( abs(rSq-1) < absRSqMinusOne ) {
          absRSqMinusOne = abs(rSq-1);
          optRSquared = rSq;
          maxID = k;
        }
      }
      
      fitEnergy = enX;
      refEnergy = output[maxID];
      printf(" R^2 : %.20f\n", optRSquared);   
    
    }else{
      fitEnergy = enX;
      refEnergy = enY;
      
      //if nX == nY, ther could be cases that only partial enX and enY are matched.   
    }
    
    printf("fitEnergy = ");for( int k = 0; k < (int) fitEnergy.size() ; k++){ printf("%7.2f, ", fitEnergy[k]); }; printf("\n");
    printf("refEnergy = ");for( int k = 0; k < (int) refEnergy.size() ; k++){ printf("%7.2f, ", refEnergy[k]); }; printf("\n");
    
    std::vector<std::vector<double>> haha;
    haha.push_back(fitEnergy);
    haha.push_back(refEnergy);
    
    return haha;
   
}

//^#########################################################
void Cali_e(TTree * tree){
///======================================================== initial input
   
   const int numDet = 110;
   const int rowDet = 11;
   const int colDet = numDet/rowDet;
   
   int energyRange[3] = {400, 1000, 3200}; // bin, min, max
   double threshold = 0.2;
   
///========================================================  load tree

   printf("============================================================= \n");
   printf("====================== Cali_xf_xn.C ========================= \n");
   printf("============================================================= \n");
   printf("=========== Total #Entry: %10lld \n", tree->GetEntries());
   
   int temp = 0;
   printf(" Min Raw Energy [ch] (default = %d ch) : ", energyRange[1]);
   scanf("%d", &temp);
   energyRange[1] = temp;

   printf(" Max Raw Energy [ch] (default = %d ch) : ", energyRange[2]);
   scanf("%d", &temp);   
   energyRange[2] = temp;
   
   printf("     Raw Energy is now %d ch to %d ch\n", energyRange[1], energyRange[2]);
///======================================================== Canvas

   Int_t Div[2] = {colDet,rowDet};  //x,y
   Int_t size[2] = {230,230}; //x,y
   TCanvas * cAlpha = new TCanvas("cAlpha", "cAlpha", 0, 0, size[0]*Div[0], size[1]*Div[1]);
   cAlpha->Divide(Div[0],Div[1]);
   
   for( int i = 1; i <= Div[0]*Div[1] ; i++){
      cAlpha->cd(i)->SetGrid();
   }

   gStyle->SetOptStat(0);
   gStyle->SetStatY(1.0);
   gStyle->SetStatX(0.99);
   gStyle->SetStatW(0.2);
   gStyle->SetStatH(0.1);
   
   if(cAlpha->GetShowEditor()  )cAlpha->ToggleEditor();
   if(cAlpha->GetShowToolBar() )cAlpha->ToggleToolBar();
   
///========================================================= Analysis

   //############################################################ energy correction
   printf("############## e correction \n");
   TH1F * q[numDet];
   TString gate[numDet];
   for( int i = 0; i < numDet; i ++){
      TString name;
      name.Form("q%d", i);
      q[i] = new TH1F(name, name, energyRange[0], energyRange[1], energyRange[2]);
      q[i]->SetXTitle(name);
      
      TString expression;
      expression.Form("(post_rise_energy - pre_rise_energy)/350 >> q%d" ,i);
      
      gate[i].Form("detID==%d", i+1);

      cAlpha->cd(i+1);
      tree->Draw(expression, gate[i] , "", MAXEVENTSIZE);
      cAlpha->Update();
      gSystem->ProcessEvents();
   }
   
   //----------- 1, pause for save Canvas
   int dummy = 0;
   temp = 0;
   cAlpha->Update();
   gSystem->ProcessEvents();


   std::vector<double> energy[numDet]; 
   double a0[numDet];
   double a1[numDet];
   std::vector<double> refEnergy;

   printf("---- finding peak using TSpectrum Class...\n");   
   
   printf(" peak threshold (default = %.3f) : ", threshold);
   scanf("%lf", &threshold);
   printf("     threshold is now %.3f\n", threshold);
      
   for( int i = 0; i < numDet; i++){

      if ( q[i]->GetEntries() == 0 )  continue;
      
      TSpectrum * spec = new TSpectrum();
      int nPeaks = spec->Search(q[i], 3, "", threshold);
      printf("%2d | found %d peaks | ", i,  nPeaks);

      double * xpos = spec->GetPositionX();
      double * ypos = spec->GetPositionY();
      
      std::vector<double> height;
      
      int * inX = new int[nPeaks];
      TMath::Sort(nPeaks, xpos, inX, 0 );
      for( int j = 0; j < nPeaks; j++){
         energy[i].push_back(xpos[inX[j]]);
         height.push_back(ypos[inX[j]]);
      }
      
      for( int j = 0; j < nPeaks; j++){
         printf("%7.2f, ", energy[i][j]);
      }
      printf("\n");

      delete[] inX;
      delete spec;
   }

   for( int i = 0; i < numDet; i++){
      cAlpha->cd(i+1);
      q[i]->Draw();
      cAlpha->Update();
      gSystem->ProcessEvents();
   }


   //------------ 3, correction
   int refID = 0;
   printf("========== which detector to be the reference?\n");
   printf(" X =  det-X reference\n");
   printf("-1 =  manual reference\n");
   printf("-2 =  use 60Co, 2 peaks \n");
   printf("-3 =  use 207Bi, 2 peaks\n");
   printf("-9 =  stop \n");
   printf("your choice = ");
   temp = scanf("%d", &refID);
   
   if( refID == -9 ) {
      printf("------ stopped by user.\n");
      return;
   }
   
   //======== fill reference energy
   if( refID >= 0 ){
      int n = energy[refID].size();
      for( int k = 0; k < n; k++) refEnergy.push_back(energy[refID][k]);
   }
   
   if(refID == -1){
      int n = 0;
      float eng = -1;
      do{
         printf("%2d-th peak energy (< 0 to stop):", n);
         temp = scanf("%f", &eng);
         printf("             input: %f \n", eng);
         if( eng >= 0 ) refEnergy.push_back(eng);
         n ++ ;
      }while(eng >= 0);
   }
   
   if( refID == -2 ){
      refEnergy.clear();
      refEnergy.push_back(1173.2);
      refEnergy.push_back(1332.5);
   }
      
   if( refID == -3 ){
      refEnergy.clear();
      refEnergy.push_back(589.7);
      refEnergy.push_back(1063.7);
   }
   
   
   printf("----- adjusting the energy to det-%d......\n", refID);
   for( int k = 0; k < refEnergy.size(); k++) printf("%2d-th peak : %f \n", k,  refEnergy[k]);
   
   const std::vector<double> refEnergy0 = refEnergy; 
   
   for( int i = 0; i < numDet; i ++){
     
     printf("------- refID - %d, nPeaks: %lu \n", i, energy[i].size());
     
     refEnergy = refEnergy0;
     
     if( refID >= 0 && refID == i ){
       a0[i] = 0;
       a1[i] = 1;
       printf("skipped - itself\n");
       continue;
     }
     
     if( energy[i].size() == 0 || q[i]->GetEntries() == 0 ){
       a0[i] = 0;
       a1[i] = 1;
       printf("skipped\n");
       continue;
     }

     if( energy[i].size() < 2 || refEnergy.size() < 2 ){
       a0[i] = 0;
       a1[i] = 1;
       printf("skipped - less than 2 peaks\n");
       continue;
     }
     
     printf("   Energy : ");
     for( int k = 0; k < energy[i].size(); k++){ printf("%.1f, ", energy[i][k]);};printf("\n");
     std::vector<std::vector<double>> output =  FindMatchingPair(energy[i], refEnergy);
 
     std::vector<double> haha1 = output[0];
     std::vector<double> haha2 = output[1];
     
     TGraph * graph = new TGraph(haha1.size(), &haha1[0], &haha2[0] );
     cAlpha->cd(i+1);
     graph->Draw("A*");

     TF1 * fit = new TF1("fit", "pol1" );
     graph->Fit("fit", "q");

     a0[i] = fit->GetParameter(0);
     a1[i] = fit->GetParameter(1);

     printf("%2d | a0: %6.3f, a1: %6.3f (%14.8f) \n", i, a0[i], a1[i], 1./a1[i]);

     gSystem->ProcessEvents();
     
   }
      
   printf("========================== Plot adjusted spectrum ==========================\n"); 
   //====== Plot adjusted spectrum
   TCanvas * cAux = new TCanvas ("cAux", "cAux", 600, 400);
   TH2F * p[numDet];

   for ( int i = 0; i < numDet; i ++){
      if ( q[i]->GetEntries() == 0 )  continue;
      // if (a0[i] == 0 && a1[i] == 1 ) continue;

      TString name;
      name.Form("p%d", i+1);
      p[i] = new TH2F(name, name, numDet, 0, numDet, energyRange[0], refEnergy[0] * 0.9, refEnergy.back() * 1.1);
      
      TString expression;
      expression.Form("(post_rise_energy - pre_rise_energy)/350 * %.8f + %.8f : detID >> p%d" ,a1[i], a0[i], i);
      gate[i].Form("detID == %d", i + 1);
      tree->Draw(expression, gate[i] , "colz", MAXEVENTSIZE);
      
      cAux->Update();
      gSystem->ProcessEvents();
   }   
   
   cAux->cd(1);
   TString hhhName;
   TH2F * hhh = new TH2F("hhh", hhhName, numDet, 0, numDet, energyRange[0], refEnergy[0] * 0.9, refEnergy.back() * 1.1);
   for( int  i = 0; i < numDet; i++){
      if ( q[i]->GetEntries() == 0 )  continue;
      // if (a0[i] == 0 && a1[i] == 1 ) continue;
      hhh->Add(p[i]);
   }
   hhh->Draw("colz");
   cAux->Update();
   gSystem->ProcessEvents();
   
   //----------- 4, pause for saving correction parameters
   cAlpha->Update();
   gSystem->ProcessEvents();
   //printf("0 for stop, 1 for save e-correction & Canvas, 2 for xf - xn correction: ");
   //temp = scanf("%d", &dummy);
   dummy = 1;
   if( dummy == 0 ) return;
   if( dummy == 1 ){
      FILE * paraOut;
      TString filename;
      filename.Form("correction_e.dat");
      paraOut = fopen (filename.Data(), "w+");
      printf("=========== save e-correction parameters to %s \n", filename.Data());
      for( int i = 0; i < numDet; i++){
         fprintf(paraOut, "%14.8f\t%9.6f\n", 1./a1[i], a0[i]);
      }
      fflush(paraOut);
      fclose(paraOut);
      
      //cAlpha->SaveAs("alpha_e_corrected.pdf");
   }  
   
   gSystem->ProcessEvents();
   
   return;
   
}

