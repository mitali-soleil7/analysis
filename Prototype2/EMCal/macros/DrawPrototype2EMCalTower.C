// $Id: $                                                                                             

/*!
 * \file Draw.C
 * \brief 
 * \author Jin Huang <jhuang@bnl.gov>
 * \version $Revision:   $
 * \date $Date: $
 */

#include <cmath>
#include <TFile.h>
#include <TString.h>
#include <TLine.h>
#include <TTree.h>
#include <TLatex.h>
#include <TGraphErrors.h>
#include <cassert>
#include "SaveCanvas.C"
#include "SetOKStyle.C"
using namespace std;

//#include "Prototype2_DSTReader.h"

TFile * _file0 = NULL;
TTree * T = NULL;
TString cuts = "";

void
DrawPrototype2EMCalTower( //
    const TString infile = "data/TB_DST.root_DSTReader.root", //
    bool plot_all = false)
{
  SetOKStyle();
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1111);
  TVirtualFitter::SetDefaultFitter("Minuit2");
  gSystem->Load("libg4eval.so");
  gSystem->Load("libqa_modules.so");
  gSystem->Load("libPrototype2.so");

  gROOT->LoadMacro("Prototype2_DSTReader.C+");

  if (!_file0)
    {
      TString chian_str = infile;
      chian_str.ReplaceAll("ALL", "*");

      TChain * t = new TChain("T");
      const int n = t->Add(chian_str);

      cout << "Loaded " << n << " root files with " << chian_str << endl;
      assert(n > 0);

      T = t;

      _file0 = new TFile;
      _file0->SetName(infile);
    }

  assert(_file0);

  T->SetAlias("ActiveTower_LG",
      "TOWER_LG_CEMC[].get_binphi()<8 && TOWER_LG_CEMC[].get_bineta()<8");
  T->SetAlias("EnergySum_LG",
      "1*Sum$(TOWER_LG_CEMC[].get_energy() * ActiveTower_LG)");

  T->SetAlias("ActiveTower_HG",
      "TOWER_HG_CEMC[].get_binphi()<8 && TOWER_HG_CEMC[].get_bineta()<8");
  T->SetAlias("EnergySum_HG",
      "1*Sum$(TOWER_HG_CEMC[].get_energy() * ActiveTower_HG)");

  T->SetAlias("C2_Inner_e", "1*TOWER_RAW_C2[0].energy");

//  "TOWER_CALIB_CEMC.energy * ( Sum$( TOWER_CALIB_CEMC.get_column()==2 && TOWER_CALIB_CEMC.get_row()==1

  T->SetAlias("Average_column",
      "Sum$(TOWER_CALIB_CEMC.get_column() * TOWER_CALIB_CEMC.get_energy())/Sum$(TOWER_CALIB_CEMC.get_energy())");
  T->SetAlias("Average_row",
      "Sum$(TOWER_CALIB_CEMC.get_row() * TOWER_CALIB_CEMC.get_energy())/Sum$(TOWER_CALIB_CEMC.get_energy())");

  T->SetAlias("Average_HODO_VERTICAL",
      "Sum$(TOWER_CALIB_HODO_VERTICAL.towerid * (abs(TOWER_CALIB_HODO_VERTICAL.energy)>30) * abs(TOWER_CALIB_HODO_VERTICAL.energy))/Sum$((abs(TOWER_CALIB_HODO_VERTICAL.energy)>30) * abs(TOWER_CALIB_HODO_VERTICAL.energy))");
  T->SetAlias("Valid_HODO_VERTICAL",
      "Sum$(abs(TOWER_CALIB_HODO_VERTICAL.energy)>30) > 0");
  T->SetAlias("Average_HODO_HORIZONTAL",
      "Sum$(TOWER_CALIB_HODO_HORIZONTAL.towerid * (abs(TOWER_CALIB_HODO_HORIZONTAL.energy)>30) * abs(TOWER_CALIB_HODO_HORIZONTAL.energy))/Sum$((abs(TOWER_CALIB_HODO_HORIZONTAL.energy)>30) * abs(TOWER_CALIB_HODO_HORIZONTAL.energy))");
  T->SetAlias("Valid_HODO_HORIZONTAL",
      "Sum$(abs(TOWER_CALIB_HODO_HORIZONTAL.energy)>30) > 0");

  T->SetAlias("Energy_Sum_col1_row2_3x3",
      "Sum$( (abs(TOWER_CALIB_CEMC.get_column()-2)<=1 && abs(TOWER_CALIB_CEMC.get_row()-1)<=1 ) * TOWER_CALIB_CEMC.get_energy())");
  T->SetAlias("Energy_Sum_col1_row2_5x5",
      "Sum$( (abs(TOWER_CALIB_CEMC.get_column()-2)<=2 && abs(TOWER_CALIB_CEMC.get_row()-1)<=2 ) * TOWER_CALIB_CEMC.get_energy())");

//
  TCut event_sel = "1*1";
  if (plot_all)
    {
      event_sel = "1*1";
      cuts = "_all_event";
    }
  else
    {
//      event_sel = "1*1";
//      cuts = "_all_event";
//      event_sel = "Valid_HODO_HORIZONTAL && Valid_HODO_VERTICAL";
      cuts = "_Valid_HODO";
      event_sel = "Valid_HODO_HORIZONTAL && Valid_HODO_VERTICAL && Average_HODO_HORIZONTAL>1.5 && Average_HODO_HORIZONTAL<4.5 && Average_HODO_VERTICAL>3.5 && Average_HODO_VERTICAL<6.5";
      cuts = "_Valid_HODO_center_col1_row2";
//      event_sel =
//          "C2_Inner_e>100 && Valid_HODO_HORIZONTAL && Valid_HODO_VERTICAL && Average_HODO_HORIZONTAL>1.5 && Average_HODO_HORIZONTAL<4.5 && Average_HODO_VERTICAL>3.5 && Average_HODO_VERTICAL<6.5";
//      cuts = "_Valid_HODO_center_col1_row2_C2";
//      event_sel = "abs(Average_column - 1)<.5 && abs(Average_row - 2) < .5";
//      cuts = "_tower_center_col1_row2";
    }

  cout << "Build event selection of " << (const char *) event_sel << endl;

  T->Draw(">>EventList", event_sel);
  TEventList * elist = gDirectory->GetObjectChecked("EventList", "TEventList");
  cout << elist->GetN() << " / " << T->GetEntriesFast() << " events selected"
      << endl;

  T->SetEventList(elist);

  int rnd = rand();
  gDirectory->mkdir(Form("dir_%d", rnd));
  gDirectory->cd(Form("dir_%d", rnd));
//  if (plot_all)
    EMCDistribution_SUM("Energy_Sum_col1_row2_5x5");

  int rnd = rand();
  gDirectory->mkdir(Form("dir_%d", rnd));
  gDirectory->cd(Form("dir_%d", rnd));
  if (plot_all)
    EMCDistribution_Fast();

  int rnd = rand();
  gDirectory->mkdir(Form("dir_%d", rnd));
  gDirectory->cd(Form("dir_%d", rnd));
  if (plot_all)
    EMCDistribution_Fast("RAW");

  int rnd = rand();
  gDirectory->mkdir(Form("dir_%d", rnd));
  gDirectory->cd(Form("dir_%d", rnd));
  if (plot_all)
    EMCDistribution_Fast("CALIB", true);

  int rnd = rand();
  gDirectory->mkdir(Form("dir_%d", rnd));
  gDirectory->cd(Form("dir_%d", rnd));
  if (plot_all)
    EMCDistribution_ADC();

//  Prototype2_DSTReader * r = new Prototype2_DSTReader(T);
  if (!plot_all)
    T->Process("Prototype2_DSTReader.C+",
        TString(_file0->GetName())
            + TString("_DrawPrototype2EMCalTower_Prototype2_DSTReader_") + cuts
            + TString(".dat"));
}

void
EMCDistribution_SUM(TString sTOWER = "Energy_Sum_col1_row2_5x5")
{
  TH1 * EnergySum_LG_full = new TH1F("EnergySum_LG_full",
      ";Full range Tower Energy Sum (GeV);Count / bin", 300, 0, 40);
  TH1 * EnergySum_LG = new TH1F("EnergySum_LG",
      ";Full range Tower Energy Sum (GeV);Count / bin", 300, 0, 40);
//  TH1 * EnergySum_HG = new TH1F("EnergySum_HG",
//      ";Low range Tower Energy Sum (ADC);Count / bin", 50, 0, 500);

  TH1 * C2_Inner_full = new TH1F("C2_Inner_full",
      ";C2 Inner Signal (ADC);Count / bin", 200, 0, 2000);
  TH1 * C2_Inner = new TH1F("C2_Inner", ";C2 Inner Signal (ADC);Count / bin",
      200, 0, 2000);

  EnergySum_LG_full->SetLineColor(kBlue + 3);
  EnergySum_LG_full->SetLineWidth(2);

  EnergySum_LG->SetLineColor(kGreen + 3);
  EnergySum_LG->SetLineWidth(3);
  EnergySum_LG->SetMarkerColor(kGreen + 3);

  C2_Inner_full->SetLineColor(kBlue + 3);
  C2_Inner_full->SetLineWidth(2);

  C2_Inner->SetLineColor(kGreen + 3);
  C2_Inner->SetLineWidth(3);
  C2_Inner->SetMarkerColor(kGreen + 3);

  TCut c2 = "C2_Inner_e>100";

  T->Draw(sTOWER + ">>EnergySum_LG_full", "", "goff");
  T->Draw(sTOWER + ">>EnergySum_LG", c2, "goff");
  T->Draw("C2_Inner_e>>C2_Inner_full", "", "goff");
  T->Draw("C2_Inner_e>>C2_Inner", c2, "goff");

  TText * t;
  TCanvas *c1 = new TCanvas("EMCDistribution_SUM_" + sTOWER + cuts,
      "EMCDistribution_SUM_" + sTOWER + cuts, 1800, 600);
  c1->Divide(3, 1);
  int idx = 1;
  TPad * p;

  p = (TPad *) c1->cd(idx++);
  c1->Update();
  p->SetLogy();
  p->SetGridx(0);
  p->SetGridy(0);

  C2_Inner_full->DrawClone();
  C2_Inner->DrawClone("same");

  p = (TPad *) c1->cd(idx++);
  c1->Update();
  p->SetLogy();
  p->SetGridx(0);
  p->SetGridy(0);

  TH1 * h = (TH1 *) EnergySum_LG_full->DrawClone();
  h->GetXaxis()->SetRangeUser(0, h->GetMean() + 5 * h->GetRMS());
  (TH1 *) EnergySum_LG->DrawClone("same");

  p = (TPad *) c1->cd(idx++);
  c1->Update();
//  p->SetLogy();
  p->SetGridx(0);
  p->SetGridy(0);

  TH1 * h_full = (TH1 *) EnergySum_LG_full->DrawClone();
  TH1 * h = (TH1 *) EnergySum_LG->DrawClone("same");

  TF1 * fgaus_g = new TF1("fgaus_LG_g", "gaus", h->GetMean() - 1 * h->GetRMS(),
      h->GetMean() + 4 * h->GetRMS());
  fgaus_g->SetParameters(1, h->GetMean() - 2 * h->GetRMS(),
      h->GetMean() + 2 * h->GetRMS());
  h->Fit(fgaus_g, "MR0N");

  TF1 * fgaus = new TF1("fgaus_LG", "gaus",
      fgaus_g->GetParameter(1) - 1 * fgaus_g->GetParameter(2),
      fgaus_g->GetParameter(1) + 4 * fgaus_g->GetParameter(2));
  fgaus->SetParameters(fgaus_g->GetParameter(0), fgaus_g->GetParameter(1),
      fgaus_g->GetParameter(2));
  h->Fit(fgaus, "MR");

  h->Sumw2();
  h_full->Sumw2();
  h_full->GetXaxis()->SetRangeUser(h->GetMean() - 4 * h->GetRMS(),
      h->GetMean() + 4 * h->GetRMS());

  h->SetLineWidth(2);
  h->SetMarkerStyle(kFullCircle);

  h_full->SetTitle(
      Form("#DeltaE/<E> = %.1f%%",
          100 * fgaus->GetParameter(2) / fgaus->GetParameter(1)));

//  p = (TPad *) c1->cd(idx++);
//  c1->Update();
//  p->SetLogy();
//  p->SetGridx(0);
//  p->SetGridy(0);
//
//  TH1 * h = (TH1 *) EnergySum_LG->DrawClone();
//  h->GetXaxis()->SetRangeUser(0,500);
//  h->SetLineWidth(2);
//  h->SetLineColor(kBlue + 3);
////  h->Sumw2();
//  h->GetXaxis()->SetRangeUser(0, h->GetMean() + 5 * h->GetRMS());
//
//  p = (TPad *) c1->cd(idx++);
//  c1->Update();
////  p->SetLogy();
//  p->SetGridx(0);
//  p->SetGridy(0);
//
//  TH1 * h = (TH1 *) EnergySum_LG->DrawClone();
//  h->GetXaxis()->SetRangeUser(0,500);
//
//  TF1 * fgaus = new TF1("fgaus_HG", "gaus", 0, 100);
//  fgaus->SetParameters(1, h->GetMean() - 2 * h->GetRMS(),
//      h->GetMean() + 2 * h->GetRMS());
//  h->Fit(fgaus, "M");
//
//  h->Sumw2();
//  h->GetXaxis()->SetRangeUser(h->GetMean() - 4 * h->GetRMS(),
//      h->GetMean() + 4 * h->GetRMS());
//
//  h->SetLineWidth(2);
//  h->SetMarkerStyle(kFullCircle);
//
//  h->SetTitle(
//      Form("#DeltaE/<E> = %.1f%%",
//          100 * fgaus->GetParameter(2) / fgaus->GetParameter(1)));

  SaveCanvas(c1,
      TString(_file0->GetName()) + TString("_DrawPrototype2EMCalTower_")
          + TString(c1->GetName()), false);

}

void
EMCDistribution_Fast(TString gain = "CALIB", bool full_gain = false)
{
  TString hname = "EMCDistribution_" + gain
      + TString(full_gain ? "_FullGain" : "") + cuts;

  TH2 * h2 = NULL;
  if (gain.Contains("CALIB"))
    {
      if (full_gain)
        {
          h2 = new TH2F(hname,
              Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 100, .05,
              25, 64, -.5, 63.5);
          QAHistManagerDef::useLogBins(h2->GetXaxis());
        }
      else
        {
          h2 = new TH2F(hname,
              Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 260, -.2,
              5, 64, -.5, 63.5);
        }
      T->Draw(
          "TOWER_" + gain + "_CEMC[].get_bineta() + 8* TOWER_" + gain
              + "_CEMC[].get_binphi():TOWER_" + gain + "_CEMC[].get_energy()>>"
              + hname, "", "goff");
    }
  else if (gain.Contains("RAW"))
    {
      if (full_gain)
        {
          h2 = new TH2F(hname,
              Form(";Calibrated Tower Energy Sum (ADC);Count / bin"), 100,
              .05 * 100, 25 * 100, 64, -.5, 63.5);
          QAHistManagerDef::useLogBins(h2->GetXaxis());
        }
      else
        {
          h2 = new TH2F(hname,
              Form(";Calibrated Tower Energy Sum (ADC);Count / bin"), 260,
              -.2 * 100, 5 * 100, 64, -.5, 63.5);
        }
      T->Draw(
          "TOWER_" + gain + "_CEMC[].get_bineta() + 8* TOWER_" + gain
              + "_CEMC[].get_binphi():TOWER_" + gain
              + "_CEMC[].get_energy()*(-1)>>" + hname, "", "goff");
    }

  TText * t;
  TCanvas *c1 = new TCanvas(
      "EMCDistribution_" + gain + TString(full_gain ? "_FullGain" : "") + cuts,
      "EMCDistribution_" + gain + TString(full_gain ? "_FullGain" : "") + cuts,
      1800, 950);
  c1->Divide(8, 8, 0., 0.01);
  int idx = 1;
  TPad * p;

  for (int iphi = 8 - 1; iphi >= 0; iphi--)
    {
      for (int ieta = 0; ieta < 8; ieta++)
        {
          p = (TPad *) c1->cd(idx++);
          c1->Update();

          p->SetLogy();
          if (full_gain)
            {
              p->SetLogx();
            }
          p->SetGridx(0);
          p->SetGridy(0);

          TString hname = Form("hEnergy_ieta%d_iphi%d", ieta, iphi)
              + TString(full_gain ? "_FullGain" : "");

          TH1 * h = h2->ProjectionX(hname, ieta + 8 * iphi + 1,
              ieta + 8 * iphi + 1); // axis bin number is encoded as ieta+8*iphi+1

          h->SetLineWidth(0);
          h->SetLineColor(kBlue + 3);
          h->SetFillColor(kBlue + 3);

          h->GetXaxis()->SetTitleSize(.09);
          h->GetXaxis()->SetLabelSize(.08);
          h->GetYaxis()->SetLabelSize(.08);

          h->Draw();

          if (full_gain)
            h->Fit("x*gaus", "M");
          else
            h->Fit("landau", "M");

          double peak = -1;

          TF1 * fit = ((TF1 *) (h->GetListOfFunctions()->At(0)));
          if (fit)
            {

              fit->SetLineColor(kRed);
              peak = fit->GetParameter(1);

            }

          cout << Form("Finished <Col%d Row%d> = %.1f", ieta, iphi, peak)
              << endl;

          TText *t = new TText(.9, .9,
              Form("<Col%d Row%d> = %.1f", ieta, iphi, peak));
          t->SetTextAlign(33);
          t->SetTextSize(.15);
          t->SetNDC();
          t->Draw();
        }
    }

  SaveCanvas(c1,
      TString(_file0->GetName()) + TString("_DrawPrototype2EMCalTower_")
          + TString(c1->GetName()), false);

}

void
EMCDistribution(TString gain = "CALIB", bool log_scale = false)
{

  TText * t;
  TCanvas *c1 = new TCanvas(
      "EMCDistribution_" + gain + TString(log_scale ? "_Log" : "") + cuts,
      "EMCDistribution_" + gain + TString(log_scale ? "_Log" : "") + cuts, 1800,
      1000);
  c1->Divide(8, 8, 0., 0.01);
  int idx = 1;
  TPad * p;

  for (int iphi = 8 - 1; iphi >= 0; iphi--)
    {
      for (int ieta = 0; ieta < 8; ieta++)
        {
          p = (TPad *) c1->cd(idx++);
          c1->Update();

          p->SetLogy();
          p->SetGridx(0);
          p->SetGridy(0);

          TString hname = Form("hEnergy_ieta%d_iphi%d", ieta, iphi)
              + TString(log_scale ? "_Log" : "");

          TH1 * h = NULL;

          if (log_scale)
            h = new TH1F(hname,
                Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 300,
                5e-3, 3096);
          else
//            h = new TH1F(hname,
//                Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 196,
//                1900, 2096);
            h = new TH1F(hname,
                Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 596,
                -96, 500);

          h->SetLineWidth(0);
          h->SetLineColor(kBlue + 3);
          h->SetFillColor(kBlue + 3);
          h->GetXaxis()->SetTitleSize(.09);
          h->GetXaxis()->SetLabelSize(.08);
          h->GetYaxis()->SetLabelSize(.08);

          if (log_scale)
            QAHistManagerDef::useLogBins(h->GetXaxis());

          T->Draw(
              "TOWER_" + gain + "_CEMC[].get_energy_power_law_exp()>>" + hname,
              Form(
                  "TOWER_%s_CEMC[].get_bineta()==%d && TOWER_%s_CEMC[].get_binphi()==%d",
                  gain.Data(), ieta, gain.Data(), iphi), "");

          TText *t = new TText(.9, .9, Form("Col%d Row%d", ieta, iphi));
          t->SetTextAlign(33);
          t->SetTextSize(.15);
          t->SetNDC();
          t->Draw();

//          return;
        }
    }

  SaveCanvas(c1,
      TString(_file0->GetName()) + TString("_DrawPrototype2EMCalTower_")
          + TString(c1->GetName()), false);

}

void
EMCDistribution_ADC(bool log_scale = true)
{
  TString gain = "RAW";

  TText * t;
  TCanvas *c1 = new TCanvas(
      "EMCDistribution_ADC_" + gain + TString(log_scale ? "_Log" : "") + cuts,
      "EMCDistribution_ADC_" + gain + TString(log_scale ? "_Log" : "") + cuts,
      1800, 1000);
  c1->Divide(8, 8, 0., 0.01);
  int idx = 1;
  TPad * p;

  for (int iphi = 8 - 1; iphi >= 0; iphi--)
    {
      for (int ieta = 0; ieta < 8; ieta++)
        {
          p = (TPad *) c1->cd(idx++);
          c1->Update();

          if (log_scale)
            {
              p->SetLogz();
            }
          p->SetGridx(0);
          p->SetGridy(0);

          TString hname = Form("hEnergy_ieta%d_iphi%d", ieta, iphi)
              + TString(log_scale ? "_Log" : "");

          TH1 * h = NULL;

          if (log_scale)
            h = new TH2F(hname,
                Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 24, -.5,
                23.5,
//                128+64, 0, 3096);
                550, 1500, 2050);
//          else
//            h = new TH2F(hname,
//                Form(";Calibrated Tower Energy Sum (GeV);Count / bin"), 100,
//                -.050, .5,128,0,2048);

          h->SetLineWidth(0);
          h->SetLineColor(kBlue + 3);
          h->SetFillColor(kBlue + 3);
          h->GetXaxis()->SetTitleSize(.09);
          h->GetXaxis()->SetLabelSize(.08);
          h->GetYaxis()->SetLabelSize(.08);

//          if (log_scale)
//            QAHistManagerDef::useLogBins(h->GetYaxis());

          TString sdraw = "TOWER_" + gain
              + "_CEMC[].signal_samples[]:fmod(Iteration$,24)>>" + hname;
          TString scut =
              Form(
                  "TOWER_%s_CEMC[].get_bineta()==%d && TOWER_%s_CEMC[].get_binphi()==%d",
                  gain.Data(), ieta, gain.Data(), iphi);

          cout << "T->Draw(\"" << sdraw << "\",\"" << scut << "\");" << endl;

          T->Draw(sdraw, scut, "colz");

          TText *t = new TText(.9, .9, Form("Col%d Row%d", ieta, iphi));
          t->SetTextAlign(33);
          t->SetTextSize(.15);
          t->SetNDC();
          t->Draw();

//          return;
        }
    }

  SaveCanvas(c1,
      TString(_file0->GetName()) + TString("_DrawPrototype2EMCalTower_")
          + TString(c1->GetName()), false);

}
