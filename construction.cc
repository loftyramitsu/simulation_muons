#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
    : G4VUserDetectorConstruction(),
      fConcreteThickness(1.0*m),
      fZgun(0.),
      fSolidConcrete(nullptr),
      fLogicScint1(nullptr), fLogicScint2(nullptr), fLogicScint3(nullptr),
      fMessenger(nullptr),
      fScintMat(nullptr), fConcreteMat(nullptr), fWorldMat(nullptr)
{
    fMessenger = new G4GenericMessenger(this, "/det/",
        "Commandes du detecteur");
    fMessenger->DeclareMethodWithUnit("setConcreteThickness", "mm",
        &MyDetectorConstruction::SetConcreteThickness,
        "Epaisseur du beton (mm)");
}

MyDetectorConstruction::~MyDetectorConstruction()
{
    delete fMessenger;
}

void MyDetectorConstruction::SetConcreteThickness(G4double thickness)
{
    fConcreteThickness = thickness;
    if (fSolidConcrete) {
        fSolidConcrete->SetZHalfLength(0.5 * fConcreteThickness);
        G4RunManager::GetRunManager()->GeometryHasBeenModified();
        G4UImanager *ui = G4UImanager::GetUIpointer();
        ui->ApplyCommand("/run/initialize");
        ui->ApplyCommand("/vis/scene/notifyHandlers");
        ui->ApplyCommand("/vis/drawVolume");
        ui->ApplyCommand("/vis/viewer/set/viewpointThetaPhi 70 20");
        ui->ApplyCommand("/vis/viewer/set/targetPoint 0 0 0.5 m");
        ui->ApplyCommand("/vis/viewer/zoomTo 3");
        ui->ApplyCommand("/vis/viewer/set/autoRefresh true");
        ui->ApplyCommand("/vis/scene/add/trajectories smooth");
        ui->ApplyCommand("/vis/scene/endOfEventAction accumulate 1");
        ui->ApplyCommand("/vis/viewer/flush");
    }
}

void MyDetectorConstruction::DefineMaterials()
{
    G4NistManager *nist = G4NistManager::Instance();
    fWorldMat    = nist->FindOrBuildMaterial("G4_AIR");
    fConcreteMat = nist->FindOrBuildMaterial("G4_CONCRETE");
    fScintMat    = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    // Pas de propriétés optiques : les photons scintillateurs ne sont pas simulés.
    // L'énergie déposée est collectée directement via les HitsMaps.
}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    DefineMaterials();

    // -------------------------------------------------------
    //  Dimensions
    // -------------------------------------------------------
    const G4double L = 49.5*cm;   // longueur scintillateur (X)
    const G4double l = 12.5*cm;   // largeur traversée muon (Z)
    const G4double e =  2.0*cm;   // épaisseur (Y)
    const G4double h = 44.5*cm;   // écartement Scint1-Scint3

    const G4double concreteHalfXY = 1.5*m;
    const G4double concreteHalfZ  = 0.5 * fConcreteThickness;

    // -------------------------------------------------------
    //  World
    // -------------------------------------------------------
    G4Box *solidWorld = new G4Box("World", 4.*m, 4.*m, 4.*m);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(
        solidWorld, fWorldMat, "World");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(
        0, G4ThreeVector(0., 0., 0.),
        logicWorld, "World", nullptr, false, 0, true);

    // -------------------------------------------------------
    //  Scintillateurs
    // -------------------------------------------------------
    G4Box *solidScint = new G4Box("Scint", L/2., e/2., l/2.);

    const G4double z_scint1 = +(h - l) / 2.;   // +16 cm
    const G4double z_scint2 =  0.;
    const G4double z_scint3 = -(h - l) / 2.;   // -16 cm

    fLogicScint1 = new G4LogicalVolume(solidScint, fScintMat, "Scint1");
    fLogicScint2 = new G4LogicalVolume(solidScint, fScintMat, "Scint2");
    fLogicScint3 = new G4LogicalVolume(solidScint, fScintMat, "Scint3");

    new G4PVPlacement(0, G4ThreeVector(0., 0., z_scint1),
        fLogicScint1, "Scint1", logicWorld, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0., 0., z_scint2),
        fLogicScint2, "Scint2", logicWorld, false, 1, true);
    new G4PVPlacement(0, G4ThreeVector(0., 0., z_scint3),
        fLogicScint3, "Scint3", logicWorld, false, 2, true);

    // -------------------------------------------------------
    //  Béton
    // -------------------------------------------------------
    fSolidConcrete = new G4Box("Concrete",
        concreteHalfXY, concreteHalfXY, concreteHalfZ);
    G4LogicalVolume *logicConcrete = new G4LogicalVolume(
        fSolidConcrete, fConcreteMat, "Concrete");
    new G4PVPlacement(0, G4ThreeVector(0., 0., 2.*m),
        logicConcrete, "Concrete", logicWorld, false, 0, true);

    fZgun = z_scint1 + l/2. + 0.15*m;

    G4cout << "\n=== Geometrie muon ===" << G4endl;
    G4cout << "  Scint (L x e x l) : " << L/cm << " x " << e/cm
           << " x " << l/cm << " cm" << G4endl;
    G4cout << "  Z Scint1 (haut)    : " << z_scint1/cm << " cm" << G4endl;
    G4cout << "  Z Scint2           : " << z_scint2/cm << " cm" << G4endl;
    G4cout << "  Z Scint3 (bas)     : " << z_scint3/cm << " cm" << G4endl;
    G4cout << "===========================\n" << G4endl;

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    G4SDManager *sdm = G4SDManager::GetSDMpointer();

    MySensitiveDetector *sd1 = new MySensitiveDetector("ScintSD1");
    MySensitiveDetector *sd2 = new MySensitiveDetector("ScintSD2");
    MySensitiveDetector *sd3 = new MySensitiveDetector("ScintSD3");

    sdm->AddNewDetector(sd1);
    sdm->AddNewDetector(sd2);
    sdm->AddNewDetector(sd3);

    fLogicScint1->SetSensitiveDetector(sd1);
    fLogicScint2->SetSensitiveDetector(sd2);
    fLogicScint3->SetSensitiveDetector(sd3);
}
