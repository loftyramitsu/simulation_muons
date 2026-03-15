#include "construction.hh"

MyDetectorConstruction::MyDetectorConstruction()
    : G4VUserDetectorConstruction(),
      fConcreteThickness(1.0*m),   // d=1m par défaut (comme le fichier de référence)
      fLogicScint1(nullptr),
      fLogicScint2(nullptr),
      fLogicScint3(nullptr),
      fMessenger(nullptr),
      fScintMat(nullptr),
      fConcreteMat(nullptr),
      fWorldMat(nullptr),
      fZgun(0.)
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
    G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void MyDetectorConstruction::DefineMaterials()
{
    G4NistManager *nist = G4NistManager::Instance();
    fWorldMat    = nist->FindOrBuildMaterial("G4_AIR");
    fConcreteMat = nist->FindOrBuildMaterial("G4_CONCRETE");
    fScintMat    = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    DefineMaterials();

    // -------------------------------------------------------
    // Dimensions des scintillateurs (fichier de référence)
    //   L = 49.5 cm  (longueur)
    //   l = 12.5 cm  (largeur)
    //   e = 2 cm     (épaisseur)
    //   Box Geant4 = demi-dimensions → (L/2, e/2, l/2)
    //   Axe Z = axe vertical (muon descend selon -Z)
    //   Axe Y = épaisseur du scintillateur
    // -------------------------------------------------------
    const G4double L = 49.5*cm;
    const G4double l = 12.5*cm;
    const G4double e = 2.0*cm;

    // Espacement vertical entre scintillateurs
    // h = 44.5 cm → distance entre Scint1 et Scint3
    // Scint1 à z = +(h-l)/2, Scint2 à z=0, Scint3 à z = -(h-l)/2
    const G4double h = 44.5*cm;

    // -------------------------------------------------------
    // Dimensions du béton (épaisseur variable via messenger)
    //   largeur/hauteur fixées à 1.5m × 1.5m (fichier de référence)
    //   profondeur = fConcreteThickness (variable)
    // -------------------------------------------------------
    const G4double concreteHalfXY = 1.5*m;
    const G4double concreteHalfZ  = 0.5 * fConcreteThickness;

    // -------------------------------------------------------
    // World : 4m × 4m × 4m centré à l'ORIGINE (comme référence)
    // -------------------------------------------------------
    G4Box *solidWorld = new G4Box("World", 4.*m, 4.*m, 4.*m);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(
        solidWorld, fWorldMat, "World");
    G4VPhysicalVolume *physWorld = new G4PVPlacement(
        0, G4ThreeVector(0., 0., 0.),
        logicWorld, "World", nullptr, false, 0, true);

    // -------------------------------------------------------
    // Scintillateurs
    // Box : demi-dimensions (L/2, e/2, l/2)
    //   X = L/2 = 24.75 cm
    //   Y = e/2 = 1 cm
    //   Z = l/2 = 6.25 cm  (épaisseur traversée par le muon)
    //
    // Positions Z (repère monde, centré à l'origine) :
    //   Scint1 : z = +(h-l)/2 = +(44.5-12.5)/2 = +16 cm
    //   Scint2 : z =  0
    //   Scint3 : z = -(h-l)/2 = -16 cm
    // -------------------------------------------------------
    G4Box *solidScint = new G4Box("Scint", L/2., e/2., l/2.);

    const G4double z_scint1 = +(h - l) / 2.;   // +16 cm
    const G4double z_scint2 =  0.;              //   0 cm
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
    // Béton : centré à z = +2m (fichier de référence)
    //   La valeur z=+2m est conservée ; seule l'épaisseur varie.
    //   Attention : si fConcreteThickness > 2m le béton déborderait
    //   du world → le messenger le limite à 3.5m max.
    // -------------------------------------------------------
    G4Box *solidConcrete = new G4Box("Concrete",
        concreteHalfXY, concreteHalfXY, concreteHalfZ);
    G4LogicalVolume *logicConcrete = new G4LogicalVolume(
        solidConcrete, fConcreteMat, "Concrete");
    new G4PVPlacement(0, G4ThreeVector(0., 0., 2.*m),
        logicConcrete, "Concrete", logicWorld, false, 0, true);

    // -------------------------------------------------------
    // Position du canon : 15 cm au-dessus de Scint1
    // Les scintillateurs sont à z < 0 ; le béton est à z = +2m.
    // Le muon descend selon -Z, donc il part de z > z_scint1.
    // -------------------------------------------------------
    fZgun = z_scint1 + l/2. + 0.15*m;  // 15 cm au-dessus de la face haute de Scint1

    // Résumé console
    G4cout << "\n=== Géométrie muon ===" << G4endl;
    G4cout << "  Scint (L×e×l)   : " << L/cm  << " x " << e/cm
           << " x " << l/cm << " cm" << G4endl;
    G4cout << "  Z Scint1 (haut) : " << z_scint1/cm << " cm" << G4endl;
    G4cout << "  Z Scint2        : " << z_scint2/cm << " cm" << G4endl;
    G4cout << "  Z Scint3 (bas)  : " << z_scint3/cm << " cm" << G4endl;
    G4cout << "  Z Béton (centre): 200 cm (épaisseur "
           << fConcreteThickness/cm << " cm)" << G4endl;
    G4cout << "  Z départ muon   : " << fZgun/cm << " cm" << G4endl;
    G4cout << "======================\n" << G4endl;

    return physWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
    MySensitiveDetector *sd1 = new MySensitiveDetector("ScintSD1");
    MySensitiveDetector *sd2 = new MySensitiveDetector("ScintSD2");
    MySensitiveDetector *sd3 = new MySensitiveDetector("ScintSD3");

    G4SDManager::GetSDMpointer()->AddNewDetector(sd1);
    G4SDManager::GetSDMpointer()->AddNewDetector(sd2);
    G4SDManager::GetSDMpointer()->AddNewDetector(sd3);

    fLogicScint1->SetSensitiveDetector(sd1);
    fLogicScint2->SetSensitiveDetector(sd2);
    fLogicScint3->SetSensitiveDetector(sd3);
}
