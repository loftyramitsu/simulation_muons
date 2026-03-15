#ifndef GENERATOR_HH
#define GENERATOR_HH

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4ParticleTable.hh"
#include "G4RunManager.hh"
#include "G4GenericMessenger.hh"
#include "Randomize.hh"
#include <cmath>

class MyPrimaryGenerator : public G4VUserPrimaryGeneratorAction
{
public:
    MyPrimaryGenerator();
    ~MyPrimaryGenerator();

    virtual void GeneratePrimaries(G4Event *anEvent);

    // Appelé par le messenger /gun/angularModel
    // 0 = cos²θ  (Terre plate, Eq.11, Shukla 2016)
    // 1 = D(θ)   (Terre sphérique, Eq.9, Shukla 2016)
    void SetAngularModel(G4int model) { fAngularModel = model; }

private:
    G4ParticleGun      *fParticleGun;
    G4GenericMessenger *fMessenger;
    G4int               fAngularModel;

    // -----------------------------------------------------------------------
    //  Paramètres du spectre en énergie (Shukla & Sankrith 2016, Table 1)
    //  Mesure de Tsukuba (θ=0°, niveau de la mer)
    //  I(E) ∝ (E0 + E)^{-n} * (1 + E/ε)^{-1}   avec E en GeV
    // -----------------------------------------------------------------------
    static constexpr G4double kN        = 3.01;    // indice spectral
    static constexpr G4double kE0       = 4.29;    // GeV — perte énergie atm.
    static constexpr G4double kEpsilon  = 854.0;   // GeV — transition pion/kaon
    static constexpr G4double kEmin     = 1.0;     // GeV — seuil bas
    static constexpr G4double kEmax     = 1000.0;  // GeV — seuil haut

    // Paramètre Terre sphérique R/d (Table 2, Shukla 2016)
    static constexpr G4double kRsurd    = 174.0;

    // Angle zénithal max — 70° limite de validité de l'approx. Terre plate
    static constexpr G4double kThetaMax = 70.0 * CLHEP::deg;

    // PDFs et fonctions de tirage
    G4double EnergyPDF     (G4double E_GeV) const;
    G4double SampleEnergy  ()               const;

    G4double D_theta       (G4double theta) const;
    G4double ThetaPDF_cos2 (G4double theta) const;
    G4double ThetaPDF_D    (G4double theta) const;
    G4double SampleTheta   ()               const;
    G4double SamplePhi     ()               const;
};

#endif
