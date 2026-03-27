#include "physics.hh"

MyPhysicsList::MyPhysicsList()
    : G4VModularPhysicsList()
{
    RegisterPhysics(new G4EmStandardPhysics());
    RegisterPhysics(new G4DecayPhysics());
}

MyPhysicsList::~MyPhysicsList()
{}
