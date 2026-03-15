// ============================================================
//  plot.C  —  affichage des histogrammes en mode "hist"
//  Usage depuis ROOT : .x plot.C
//        ou en ligne : root -l plot.C
// ============================================================

void plot()
{
    TFile *f = TFile::Open("muon_detector.root");
    if (!f || f->IsZombie()) {
        std::cerr << "Impossible d'ouvrir muon_detector.root" << std::endl;
        return;
    }

    // Style global
    gStyle->SetOptStat(1110);   // entries, mean, RMS
    gStyle->SetOptTitle(1);
    gStyle->SetHistLineWidth(2);
    gStyle->SetHistLineColor(kBlue+1);

    // -------------------------------------------------------
    //  Canvas 1 : les 3 scintillateurs
    // -------------------------------------------------------
    TCanvas *c1 = new TCanvas("c1", "Energie deposee par scintillateur", 1200, 400);
    c1->Divide(3, 1);

    const char *h1names[] = {"Edep_Scint1", "Edep_Scint2", "Edep_Scint3"};
    int colors[] = {kBlue+1, kRed+1, kGreen+2};

    for (int i = 0; i < 3; i++) {
        c1->cd(i+1);
        gPad->SetLeftMargin(0.14);
        TH1D *h = (TH1D*)f->Get(h1names[i]);
        if (!h) { std::cerr << "Histo " << h1names[i] << " non trouve\n"; continue; }
        h->SetLineColor(colors[i]);
        h->SetLineWidth(2);
        h->SetFillColorAlpha(colors[i], 0.25);
        h->Draw("hist");   // ← mode "hist" : barres sans marqueurs
    }
    c1->SaveAs("edep_scintillateurs.png");

    // -------------------------------------------------------
    //  Canvas 2 : coïncidence triple
    // -------------------------------------------------------
    TCanvas *c2 = new TCanvas("c2", "Coincidence triple", 600, 400);
    TH1D *hCoinc = (TH1D*)f->Get("Edep_Coincidence");
    if (hCoinc) {
        hCoinc->SetLineColor(kMagenta+1);
        hCoinc->SetLineWidth(2);
        hCoinc->SetFillColorAlpha(kMagenta+1, 0.25);
        hCoinc->Draw("hist");
    }
    c2->SaveAs("coincidence.png");

    // -------------------------------------------------------
    //  Canvas 3 : corrélation 2D
    // -------------------------------------------------------
    TCanvas *c3 = new TCanvas("c3", "Correlation Scint1 vs Scint3", 600, 500);
    TH2D *h2 = (TH2D*)f->Get("Edep_S1_vs_S3");
    if (h2) {
        gStyle->SetPalette(kBird);
        h2->Draw("colz");   // 2D en couleur
    }
    c3->SaveAs("correlation_2D.png");

    std::cout << "\nFichiers PNG enregistres." << std::endl;
}
