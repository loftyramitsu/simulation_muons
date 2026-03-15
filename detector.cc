#include "detector.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"

MySensitiveDetector::MySensitiveDetector(G4String name)
    : G4VSensitiveDetector(name),
      fEdepMap(nullptr),
      fHCID(-1)
{
    // Déclare la collection de hits
    collectionName.insert(SDName::EdepKey);
}

MySensitiveDetector::~MySensitiveDetector()
{}

void MySensitiveDetector::Initialize(G4HCofThisEvent *hce)
{
    // Crée une nouvelle HitsMap pour cet événement (thread-local)
    fEdepMap = new G4THitsMap<G4double>(SensitiveDetectorName,
                                        collectionName[0]);
    if (fHCID < 0)
        fHCID = G4SDManager::GetSDMpointer()
                    ->GetCollectionID(SensitiveDetectorName + "/" + SDName::EdepKey);

    hce->AddHitsCollection(fHCID, fEdepMap);
}

G4bool MySensitiveDetector::ProcessHits(G4Step *step, G4TouchableHistory *)
{
    G4double edep = step->GetTotalEnergyDeposit();
    if (edep == 0.) return false;

    // Accumule dans la map (clé 0 = unique par scintillateur)
    fEdepMap->add(0, edep);
    return true;
}
