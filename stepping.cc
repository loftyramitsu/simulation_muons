#include "stepping.hh"
#include "event.hh"
#include "G4RunManager.hh"
#include "G4Track.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"

MySteppingAction::MySteppingAction()
    : G4UserSteppingAction()
{}

MySteppingAction::~MySteppingAction()
{}

void MySteppingAction::UserSteppingAction(const G4Step *step)
{
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return;

    // Uniquement dans Scint2
    G4String volName = step->GetPreStepPoint()
                           ->GetTouchableHandle()
                           ->GetVolume()
                           ->GetLogicalVolume()
                           ->GetName();
    if (volName != "Scint2") return;

    // Accumuler dans l'EventAction (un seul point par evenement en fin de run)
    MyEventAction *evtAction = (MyEventAction*)
        G4RunManager::GetRunManager()->GetUserEventAction();
    if (!evtAction) return;

    G4bool isMuon = (step->GetTrack()->GetDefinition() == G4MuonMinus::Definition() ||
                     step->GetTrack()->GetDefinition() == G4MuonPlus::Definition());

    if (isMuon) evtAction->AddEdepMuon(edep);
    else        evtAction->AddEdepSec(edep);
}
