#include "event.hh"
#include "run.hh"
#include "G4RunManager.hh"
#include <iomanip>
#include <string>

MyEventAction::MyEventAction(MyRunAction *runAction)
    : G4UserEventAction(), fRunAction(runAction)
{
    fHCID[0] = fHCID[1] = fHCID[2] = -1;
    fEdepMuon = fEdepSec = 0.;
}

MyEventAction::~MyEventAction()
{}

void MyEventAction::BeginOfEventAction(const G4Event *event)
{
    fEdepMuon = fEdepSec = 0.;
    G4int eventID     = event->GetEventID();
    G4int totalEvents = G4RunManager::GetRunManager()
                            ->GetCurrentRun()->GetNumberOfEventToBeProcessed();

    if (totalEvents <= 0) return;

    // Afficher tous les 1%
    G4int step = std::max(1, totalEvents / 100);
    if (eventID % step != 0) return;

    G4int percent = (G4int)(100.0 * eventID / totalEvents);

    const G4int barWidth = 40;
    G4int filled = barWidth * percent / 100;

    std::string bar = "[";
    for (G4int i = 0; i < barWidth; i++) {
        if      (i < filled)  bar += "=";
        else if (i == filled) bar += ">";
        else                  bar += " ";
    }
    bar += "]";

    G4cout << "\r  " << bar
           << " " << std::setw(3) << percent << "%"
           << "  (" << eventID << "/" << totalEvents << ")"
           << std::flush;

    if (eventID == totalEvents - 1)
        G4cout << G4endl;
}

// Lit l'énergie déposée depuis la HitsMap du SD idx
G4double MyEventAction::GetEdep(const G4Event *event, G4int idx) const
{
    auto *sdm = G4SDManager::GetSDMpointer();

    G4String names[3] = { SDName::Scint1, SDName::Scint2, SDName::Scint3 };
    if (fHCID[idx] < 0)
        fHCID[idx] = sdm->GetCollectionID(names[idx] + "/" + SDName::EdepKey);

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

    // Histos 0-2 : énergie déposée par scintillateur
    for (G4int i = 0; i < 3; i++) {
        if (edep[i] > kThreshold)
            am->FillH1(i, edep[i] / CLHEP::MeV);
    }

    // Histo 3 : coïncidence triple — énergie de Scint2
    if (edep[1] > kThreshold)
        am->FillH1(3, edep[1] / CLHEP::MeV);

    // Histo 4 : corrélation 2D Scint1 vs Scint3
    am->FillH2(0, edep[0] / CLHEP::MeV, edep[2] / CLHEP::MeV);

    // Histos 4-5 : muon vs secondaires (depuis SteppingAction)
    if (fEdepMuon > 0.) am->FillH1(4, fEdepMuon / CLHEP::MeV);
    if (fEdepSec  > 0.) am->FillH1(5, fEdepSec  / CLHEP::MeV);
}
