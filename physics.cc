#include "physics.hh"

MyPhysicsList::MyPhysicsList()
    : G4VModularPhysicsList()
{
    RegisterPhysics(new G4EmStandardPhysics());
    RegisterPhysics(new G4DecayPhysics());
    // Optique si nécessaire (peut être désactivé pour aller plus vite)
    // RegisterPhysics(new G4OpticalPhysics());
}

MyPhysicsList::~MyPhysicsList()
{}
