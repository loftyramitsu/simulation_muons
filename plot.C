#include <iostream>
// ============================================================
//  plot.C  -  affichage des histogrammes
//  Usage depuis build/ : root -l plot.C
//
//  Les PNG sont sauvegardés dans ../graphes/
//  avec le suffixe _<model>_<epaisseur>cm.png
// ============================================================

void styleH1(TH1 *h, int color, const char *xtitle, const char *ytitle = "Counts") {
    h->SetLineColor(color);
    h->SetLineWidth(2);
    h->SetFillColorAlpha(color, 0.2);
    h->GetXaxis()->SetTitle(xtitle);
    h->GetYaxis()->SetTitle(ytitle);
    h->SetStats(1);
}

// Lit run.mac et extrait le modèle angulaire et l'épaisseur
TString GetSuffix()
{
    int    model     = 0;
    double thickness = 1000.;

    std::ifstream mac("run.mac");
    if (!mac.is_open()) {
        std::cerr << "Attention : run.mac introuvable, suffixe par defaut\n";
    } else {
        std::string line;
        while (std::getline(mac, line)) {
            if (line.empty() || line[0] == '#') continue;
            if (line.find("/gun/angularModel") != std::string::npos) {
                std::istringstream ss(line);
                std::string cmd; ss >> cmd >> model;
            }
            if (line.find("/det/setConcreteThickness") != std::string::npos) {
                std::istringstream ss(line);
                std::string cmd, unit; ss >> cmd >> thickness >> unit;
                if (unit == "m")  thickness *= 1000.;
                if (unit == "cm") thickness *= 10.;
                thickness /= 10.;
            }
        }
        mac.close();
    }

    TString modelStr = (model == 0) ? "costheta" : "dtheta";
    TString thickStr = Form("%.0fcm", thickness);
    return TString("_") + modelStr + TString("_") + thickStr;
}

void plot()
{
    TFile *f = TFile::Open("muon_detector.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Impossible d'ouvrir muon_detector.root" << std::endl;
        return;
    }

    // Lecture du seuil
    double threshold_MeV = 0.1;
    std::ifstream thrFile("threshold.txt");
    if (thrFile.is_open()) { thrFile >> threshold_MeV; thrFile.close(); }

    TString suffix = GetSuffix();

    gSystem->mkdir("../graphes", kTRUE);
    TString outDir = "../graphes/";

    gStyle->SetOptStat(1110);
    gStyle->SetOptTitle(1);
    gStyle->SetTitleFont(42, "XYZ");
    gStyle->SetLabelFont(42, "XYZ");

    // -------------------------------------------------------
    //  Canvas 1 : énergie déposée dans les 3 scintillateurs
    // -------------------------------------------------------
    TCanvas *c1 = new TCanvas("c1", "Energie deposee par scintillateur", 1200, 400);
    c1->Divide(3, 1);

    const char *h1names[] = {"Edep_Scint1", "Edep_Scint2", "Edep_Scint3"};
    int colors[] = {kBlue+1, kRed+1, kGreen+2};

    for (int i = 0; i < 3; i++) {
        c1->cd(i+1);
        gPad->SetLeftMargin(0.14);
        TH1D *h = (TH1D*)f->Get(h1names[i]);
        if (!h) continue;
        styleH1(h, colors[i], "E_{dep} (MeV)");
        h->Draw("hist");
    }
    c1->SaveAs(outDir + "edep_scintillateurs" + suffix + ".png");

    // -------------------------------------------------------
    //  Canvas 2 : coïncidence triple — Scint2
    // -------------------------------------------------------
    TCanvas *c2 = new TCanvas("c2", "Coincidence triple - Scint2", 600, 400);
    TH1D *hCoinc = (TH1D*)f->Get("Edep_Coincidence");
    if (hCoinc) {
        styleH1(hCoinc, kMagenta+1, "E_{dep} Scint2 (MeV)");
        hCoinc->SetTitle("Edep Scint2 en coincidence triple");
        hCoinc->Draw("hist");
    }
    c2->SaveAs(outDir + "coincidence" + suffix + ".png");

    // -------------------------------------------------------
    //  Canvas 3 : muon vs secondaires dans Scint2 (échelle log)
    // -------------------------------------------------------
    TCanvas *c3 = new TCanvas("c3", "Muon vs secondaires - Scint2", 700, 500);
    gPad->SetLogy();
    gPad->SetLeftMargin(0.12);
    gStyle->SetOptStat(0);

    TH1D *hMu  = (TH1D*)f->Get("Edep_muon_only");
    TH1D *hSec = (TH1D*)f->Get("Edep_secondaires");

    if (hMu && hSec) {
        hMu->SetLineColor(kBlue+1);  hMu->SetLineWidth(2);  hMu->SetFillStyle(0);
        hSec->SetLineColor(kRed+1);  hSec->SetLineWidth(2); hSec->SetFillStyle(0);

        double globalMax = std::max(hMu->GetMaximum(), hSec->GetMaximum());
        hSec->SetTitle("Muon vs secondaires - Scint2");
        hSec->GetXaxis()->SetTitle("E_{dep} Scint2 par evenement (MeV)");
        hSec->GetYaxis()->SetTitle("Counts");
        hSec->SetMaximum(globalMax * 3.0);
        hSec->Draw("hist");
        hMu->Draw("hist same");

        TLine *lseuil = new TLine(threshold_MeV, 1, threshold_MeV, globalMax * 3.0);
        lseuil->SetLineColor(kGray+1);
        lseuil->SetLineStyle(2);
        lseuil->SetLineWidth(1);
        lseuil->Draw();

        TLegend *leg = new TLegend(0.45, 0.72, 0.88, 0.88);
        leg->SetBorderSize(0); leg->SetFillStyle(0);
        leg->AddEntry(hMu,    Form("Muon primaire  (N=%.0f)", (double)hMu->GetEntries()),  "l");
        leg->AddEntry(hSec,   Form("Secondaires    (N=%.0f)", (double)hSec->GetEntries()), "l");
        leg->AddEntry(lseuil, Form("Seuil coincidence (%.2g MeV)", threshold_MeV),         "l");
        leg->Draw();
    }
    c3->SaveAs(outDir + "muon_vs_secondaires" + suffix + ".png");

    // -------------------------------------------------------
    //  Canvas 4 : corrélation 2D Scint1 vs Scint3
    // -------------------------------------------------------
    TCanvas *c4 = new TCanvas("c4", "Correlation Scint1 vs Scint3", 600, 500);
    TH2D *h2 = (TH2D*)f->Get("Edep_S1_vs_S3");
    if (h2) {
        gStyle->SetPalette(kBird);
        h2->GetXaxis()->SetTitle("E_{dep} Scint1 (MeV)");
        h2->GetYaxis()->SetTitle("E_{dep} Scint3 (MeV)");
        h2->Draw("colz");
    }
    c4->SaveAs(outDir + "correlation_2D" + suffix + ".png");

    std::cout << "\nFichiers PNG sauvegardes dans " << outDir << std::endl;
    std::cout << "Suffixe utilise : " << suffix << std::endl;
}
