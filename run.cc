#include "run.hh"

MyRunAction::MyRunAction()
    : G4UserRunAction()
{
    auto *am = G4AnalysisManager::Instance();
    am->SetDefaultFileType("root");
    am->SetFileName("muon_detector");
    am->SetVerboseLevel(0);

    // Energie deposee par scintillateur : 0-100 MeV, 100 bins
    am->CreateH1("Edep_Scint1",
        "Energie deposee Scint1",
        100, 0., 100.);

    am->CreateH1("Edep_Scint2",
        "Energie deposee Scint2",
        100, 0., 100.);

    am->CreateH1("Edep_Scint3",
        "Energie deposee Scint3",
        100, 0., 100.);

    am->CreateH1("Edep_Coincidence",
        "Edep Scint2 en coincidence triple",
        100, 0., 100.);

    // Muon seul vs secondaires (delta rays, e+/e-, gamma) — pas a pas
    // Meme plage 0-100 MeV pour superposition directe sur plot.C
    am->CreateH1("Edep_muon_only",
        "Edep muon primaire",
        100, 0., 100.);

    am->CreateH1("Edep_secondaires",
        "Edep particules secondaires",
        100, 0., 100.);

    // H2 : correlation Scint1 vs Scint3
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
    am->Write();
    am->CloseFile();
}
