#include "run.hh"
#include "event.hh"
#include <fstream>

MyRunAction::MyRunAction()
    : G4UserRunAction()
{
    auto *am = G4AnalysisManager::Instance();
    am->SetDefaultFileType("root");
    am->SetFileName("muon_detector");
    am->SetVerboseLevel(0);

    // H1-0,1,2 : énergie déposée par scintillateur (0-100 MeV, 100 bins)
    am->CreateH1("Edep_Scint1",      "Energie deposee Scint1",              100, 0., 100.);
    am->CreateH1("Edep_Scint2",      "Energie deposee Scint2",              100, 0., 100.);
    am->CreateH1("Edep_Scint3",      "Energie deposee Scint3",              100, 0., 100.);

    // H1-3 : énergie Scint2 en coïncidence triple
    am->CreateH1("Edep_Coincidence", "Edep Scint2 en coincidence triple",   100, 0., 100.);

    // H1-4,5 : muon vs secondaires dans Scint2
    am->CreateH1("Edep_muon_only",   "Edep muon primaire",                  100, 0., 100.);
    am->CreateH1("Edep_secondaires", "Edep particules secondaires",         100, 0., 100.);

    // H2-0 : corrélation Scint1 vs Scint3
    am->CreateH2("Edep_S1_vs_S3",
        "Correlation Scint1 vs Scint3",
        100, 0., 100.,
        100, 0., 100.);
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

    // Écrire le seuil dans un fichier texte lu par plot.C
    G4double thrVal = MyEventAction::kThreshold / CLHEP::MeV;
    std::ofstream thrFile("threshold.txt");
    thrFile << thrVal << std::endl;
    thrFile.close();

    am->Write();
    am->CloseFile();
}
