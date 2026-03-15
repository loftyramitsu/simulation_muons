#include "run.hh"

MyRunAction::MyRunAction()
    : G4UserRunAction()
{
    auto *am = G4AnalysisManager::Instance();
    am->SetDefaultFileType("root");
    am->SetFileName("muon_detector");
    am->SetVerboseLevel(0);
    am->SetNtupleMerging(true);

    // -------------------------------------------------------
    //  Histogrammes 1D — 100 bins
    //  Le titre est au format ROOT : "titre;axeX;axeY"
    //  L'option d'affichage "hist" est encodée dans le nom
    //  via SetH1Ascii / ou imposée dans la macro plot.C ci-dessous.
    //  Geant4 ne supporte pas SetDrawOption directement,
    //  on le fixe dans plot.C (voir fin de fichier).
    // -------------------------------------------------------
    am->CreateH1("Edep_Scint1",
        "Energie deposee Scint1;E_{dep} (MeV);Counts",
        100, 0., 20.);   // MeV — muon de 10 GeV dépose ~2 MeV/cm dans plastique

    am->CreateH1("Edep_Scint2",
        "Energie deposee Scint2;E_{dep} (MeV);Counts",
        100, 0., 20.);

    am->CreateH1("Edep_Scint3",
        "Energie deposee Scint3;E_{dep} (MeV);Counts",
        100, 0., 20.);

    am->CreateH1("Edep_Coincidence",
        "Energie totale coincidence triple;E_{dep} total (MeV);Counts",
        100, 0., 60.);

    // H2 : corrélation Scint1 vs Scint3 — 100×100 bins
    am->CreateH2("Edep_S1_vs_S3",
        "Correlation Scint1 vs Scint3;E_{dep} Scint1 (MeV);E_{dep} Scint3 (MeV)",
        100, 0., 20.,
        100, 0., 20.);
}

MyRunAction::~MyRunAction()
{}

void MyRunAction::BeginOfRunAction(const G4Run *)
{
    G4AnalysisManager::Instance()->OpenFile();
}

void MyRunAction::EndOfRunAction(const G4Run *)
{
    auto *am = G4AnalysisManager::Instance();
    am->Write();
    am->CloseFile();
}
