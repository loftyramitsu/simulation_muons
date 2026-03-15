#include "event.hh"
#include "run.hh"

MyEventAction::MyEventAction(MyRunAction *runAction)
    : G4UserEventAction(), fRunAction(runAction)
{
    fHCID[0] = fHCID[1] = fHCID[2] = -1;
}

MyEventAction::~MyEventAction()
{}

void MyEventAction::BeginOfEventAction(const G4Event *)
{}

// Lit l'énergie déposée depuis la HitsMap du SD idx
G4double MyEventAction::GetEdep(const G4Event *event, G4int idx) const
{
    auto *sdm = G4SDManager::GetSDMpointer();

    // Résolution paresseuse des IDs (thread-safe : exécuté une fois par thread)
    G4String names[3] = { SDName::Scint1, SDName::Scint2, SDName::Scint3 };
    if (fHCID[idx] < 0) {
        fHCID[idx] = sdm->GetCollectionID(names[idx] + "/" + SDName::EdepKey);
    }

    auto *hce = event->GetHCofThisEvent();
    if (!hce) return 0.;

    auto *hitsMap = static_cast<G4THitsMap<G4double>*>(hce->GetHC(fHCID[idx]));
    if (!hitsMap) return 0.;

    G4double total = 0.;
    for (auto &kv : *hitsMap->GetMap())
        total += *(kv.second);
    return total;
}

void MyEventAction::EndOfEventAction(const G4Event *event)
{
    G4double edep[3];
    for (G4int i = 0; i < 3; i++)
        edep[i] = GetEdep(event, i);

    auto *am = G4AnalysisManager::Instance();

    // Histogrammes 1D : énergie déposée par scintillateur
    am->FillH1(0, edep[0] / CLHEP::MeV);
    am->FillH1(1, edep[1] / CLHEP::MeV);
    am->FillH1(2, edep[2] / CLHEP::MeV);

    // Coïncidence triple
    if (edep[0] > kThreshold && edep[1] > kThreshold && edep[2] > kThreshold) {
        G4double tot = edep[0] + edep[1] + edep[2];
        am->FillH1(3, tot / CLHEP::MeV);
    }

    // Histogramme 2D : corrélation Scint1 vs Scint3
    am->FillH2(0, edep[0] / CLHEP::MeV, edep[2] / CLHEP::MeV);
}
