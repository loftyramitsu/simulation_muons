#ifndef DETECTOR_HH
#define DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4THitsMap.hh"
#include "G4Step.hh"
#include "G4SystemOfUnits.hh"

// Noms des collections partagés avec EventAction
namespace SDName {
    inline const G4String Scint1  = "ScintSD1";
    inline const G4String Scint2  = "ScintSD2";
    inline const G4String Scint3  = "ScintSD3";
    inline const G4String EdepKey = "Edep";
}

class MySensitiveDetector : public G4VSensitiveDetector
{
public:
    MySensitiveDetector(G4String name);
    ~MySensitiveDetector();

    virtual void   Initialize(G4HCofThisEvent *hce);
    virtual G4bool ProcessHits(G4Step *step, G4TouchableHistory *history);

private:
    G4THitsMap<G4double> *fEdepMap;
    G4int                 fHCID;
};

#endif
