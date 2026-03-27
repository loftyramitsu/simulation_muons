#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4GenericMessenger.hh"

#include "detector.hh"

class MyDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    virtual G4VPhysicalVolume *Construct();
    virtual void ConstructSDandField();

    void SetConcreteThickness(G4double thickness);
    G4double GetZgun() const { return fZgun; }

private:
    G4double fConcreteThickness;
    G4double fZgun;

    G4Box           *fSolidConcrete;

    G4LogicalVolume *fLogicScint1;
    G4LogicalVolume *fLogicScint2;
    G4LogicalVolume *fLogicScint3;

    G4GenericMessenger *fMessenger;

    G4Material *fScintMat;
    G4Material *fConcreteMat;
    G4Material *fWorldMat;

    void DefineMaterials();
};

#endif
