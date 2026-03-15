#include "generator.hh"
#include "G4UImanager.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
    : fAngularModel(0)
{
    fParticleGun = new G4ParticleGun(1);

    // -------------------------------------------------------
    //  Supprimer les messages "was defined in terms of..."
    //  La commande /gun/verbose 0 est la méthode portable.
    // -------------------------------------------------------
    G4UImanager::GetUIpointer()->ApplyCommand("/gun/verbose 0");

    // Messenger : choix du modèle angulaire depuis run.mac
    fMessenger = new G4GenericMessenger(this, "/gun/",
        "Parametres du generateur de muons cosmiques");
    fMessenger->DeclareMethod("angularModel",
        &MyPrimaryGenerator::SetAngularModel,
        "Modele angulaire: 0=cos2theta (Terre plate), 1=D(theta) (Terre spherique)");
}

MyPrimaryGenerator::~MyPrimaryGenerator()
{
    delete fMessenger;
    delete fParticleGun;
}

// -------------------------------------------------------
//  Spectre en énergie — Eq.2, Shukla & Sankrith (2016)
//  I(E) ∝ (E0 + E)^{-n} * (1 + E/ε)^{-1}   (E en GeV)
// -------------------------------------------------------
G4double MyPrimaryGenerator::EnergyPDF(G4double E) const
{
    return std::pow(kE0 + E, -kN) / (1.0 + E / kEpsilon);
}

// -------------------------------------------------------
//  Tirage de l'énergie cinétique par rejection sampling
//
//  Proposition q(E) ∝ 1/E (log-uniforme sur [Emin, Emax])
//  → le poids est w(E) = I(E)/q(E) ∝ I(E)·E
//  → max de w à E=Emin (I(E)·E est décroissant sur tout le domaine)
// -------------------------------------------------------
G4double MyPrimaryGenerator::SampleEnergy() const
{
    const G4double logEmin = std::log(kEmin);
    const G4double logEmax = std::log(kEmax);
    const G4double wmax    = EnergyPDF(kEmin) * kEmin;

    G4double E, w;
    do {
        E = std::exp(logEmin + G4UniformRand() * (logEmax - logEmin));
        w = EnergyPDF(E) * E;
    } while (G4UniformRand() * wmax > w);

    return E;   // GeV
}

// -------------------------------------------------------
//  D(θ) — Eq.7, rapport de trajet incliné / vertical
//  D(θ) = sqrt( (R/d)² cos²θ + 2(R/d) + 1 ) − (R/d)·cosθ
// -------------------------------------------------------
G4double MyPrimaryGenerator::D_theta(G4double theta) const
{
    G4double c  = std::cos(theta);
    G4double Rd = kRsurd;
    return std::sqrt(Rd * Rd * c * c + 2.0 * Rd + 1.0) - Rd * c;
}

// -------------------------------------------------------
//  PDFs angulaires (avec jacobien sinθ)
//
//  Modèle 0 — cos²θ (Terre plate, Eq.11)
//    f(θ) ∝ cos²θ · sinθ
//    Maximum analytique : θ* = arctan(1/√2) ≈ 35.26°
//
//  Modèle 1 — D(θ) (Terre sphérique, Eq.9)
//    f(θ) ∝ D(θ)^{-(n-1)} · sinθ
// -------------------------------------------------------
G4double MyPrimaryGenerator::ThetaPDF_cos2(G4double theta) const
{
    return std::pow(std::cos(theta), 2) * std::sin(theta);
}

G4double MyPrimaryGenerator::ThetaPDF_D(G4double theta) const
{
    return std::pow(D_theta(theta), -(kN - 1.0)) * std::sin(theta);
}

// -------------------------------------------------------
//  Tirage de θ par rejection sampling
//  Le maximum est calculé numériquement (1000 points, négligeable en temps)
// -------------------------------------------------------
G4double MyPrimaryGenerator::SampleTheta() const
{
    const int Nsteps = 1000;
    G4double  fmax   = 0.;
    for (int i = 0; i <= Nsteps; i++) {
        G4double t = kThetaMax * i / Nsteps;
        G4double f = (fAngularModel == 0) ? ThetaPDF_cos2(t) : ThetaPDF_D(t);
        if (f > fmax) fmax = f;
    }

    G4double theta, f;
    do {
        theta = G4UniformRand() * kThetaMax;
        f = (fAngularModel == 0) ? ThetaPDF_cos2(theta) : ThetaPDF_D(theta);
    } while (G4UniformRand() * fmax > f);

    return theta;
}

G4double MyPrimaryGenerator::SamplePhi() const
{
    return G4UniformRand() * CLHEP::twopi;
}

// -------------------------------------------------------
//  Génération du vertex primaire
// -------------------------------------------------------
void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleTable *table = G4ParticleTable::GetParticleTable();

    // Rapport mu-/mu+ ≈ 1.3 au niveau de la mer
    G4ParticleDefinition *particle =
        (G4UniformRand() < 1.3 / 2.3)
        ? table->FindParticle("mu-")
        : table->FindParticle("mu+");
    fParticleGun->SetParticleDefinition(particle);

    // ---------------------------------------------------
    //  Énergie : SetParticleEnergy = énergie CINÉTIQUE
    //  (≠ SetParticleMomentum qui causait le bug de spectre)
    //  Pour E >> m_mu=105MeV : Ekin ≈ Etot, c'est exact à >99% dès 1 GeV
    // ---------------------------------------------------
    G4double Ekin_GeV = SampleEnergy();
    fParticleGun->SetParticleEnergy(Ekin_GeV * CLHEP::GeV);

    // ---------------------------------------------------
    //  Direction : muon descendant
    //  θ = angle depuis la verticale descendante (-Z)
    // ---------------------------------------------------
    G4double theta = SampleTheta();
    G4double phi   = SamplePhi();

    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(
        std::sin(theta) * std::cos(phi),
        std::sin(theta) * std::sin(phi),
        -std::cos(theta)
    ));

    // ---------------------------------------------------
    //  Position : plan horizontal à z = +3.0 m
    //  (bien au-dessus du béton dont la face sup est à +2.5 m max)
    //  Surface de génération 8m×8m : couvre les muons inclinés à 70°
    //  qui peuvent dériver de ±(3m - 0m) × tan(70°) ≈ ±8m en XY
    //  par rapport au centre des scintillateurs.
    //  Le world fait 4m×4m → on reste dans [-4,+4]m.
    // ---------------------------------------------------
    G4double xyHalf = 3.5 * CLHEP::m;
    G4double x0 = (G4UniformRand() - 0.5) * 2.0 * xyHalf;
    G4double y0 = (G4UniformRand() - 0.5) * 2.0 * xyHalf;

    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, 3.0 * CLHEP::m));
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
