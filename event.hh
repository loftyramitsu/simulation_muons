#ifndef EVENT_HH
#define EVENT_HH

#include "G4UserEventAction.hh"
#include "G4Event.hh"
#include "G4THitsMap.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

#include "detector.hh"

class MyRunAction;

class MyEventAction : public G4UserEventAction
{
public:
    MyEventAction(MyRunAction *runAction);
    ~MyEventAction();

    virtual void BeginOfEventAction(const G4Event *event);
    virtual void EndOfEventAction(const G4Event *event);

    // Seuil de coïncidence
    static constexpr G4double kThreshold = 0.1 * CLHEP::MeV;

private:
    MyRunAction *fRunAction;

    // IDs des collections (-1 = non encore résolus)
    mutable G4int fHCID[3];

    G4double GetEdep(const G4Event *event, G4int idx) const;
};

#endif
