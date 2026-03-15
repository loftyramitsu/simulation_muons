#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
{
    fParticleGun = new G4ParticleGun(1);

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    fParticleGun->SetParticleDefinition(particleTable->FindParticle("mu-"));
    fParticleGun->SetParticleMomentum(10.*CLHEP::GeV);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., -1.));
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fParticleGun;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    // Lit la position Z du canon depuis la construction
    // (se met à jour automatiquement si ReinitializeGeometry est appelé)
    const auto *det = static_cast<const MyDetectorConstruction*>(
        G4RunManager::GetRunManager()->GetUserDetectorConstruction());

    G4double zGun = det ? det->GetZgun() : 0.3*m;

    fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., zGun));
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
