#include "generator.hh"
#include "G4UImanager.hh"

MyPrimaryGenerator::MyPrimaryGenerator()
    : fAngularModel(0)
{
    fParticleGun = new G4ParticleGun(1);
    G4UImanager::GetUIpointer()->ApplyCommand("/gun/verbose 0");

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

G4double MyPrimaryGenerator::EnergyPDF(G4double E) const
{
    return std::pow(kE0 + E, -kN) / (1.0 + E / kEpsilon);
}

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
    return E;
}

G4double MyPrimaryGenerator::D_theta(G4double theta) const
{
    G4double c  = std::cos(theta);
    G4double Rd = kRsurd;
    return std::sqrt(Rd * Rd * c * c + 2.0 * Rd + 1.0) - Rd * c;
}

G4double MyPrimaryGenerator::ThetaPDF_cos2(G4double theta) const
{
    return std::pow(std::cos(theta), 2) * std::sin(theta);
}

G4double MyPrimaryGenerator::ThetaPDF_D(G4double theta) const
{
    return std::pow(D_theta(theta), -(kN - 1.0)) * std::sin(theta);
}

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
//  Verifie que la trajectoire (xt,yt,zt=z_scint1 + direction)
//  intercepte bien Scint2 et Scint3.
//
//  Geometrie (dalle verticale, muon descend selon -Z) :
//    Scint : X in [-L/2, L/2], Y in [-e/2, e/2], Z traverse l=12.5cm
//    z_scint1 = +16 cm, z_scint2 = 0, z_scint3 = -16 cm
//
//  Pour aller de z_scint1 a z_scintN, le muon derive de :
//    dx = (z_scint1 - z_scintN) * tan(theta) * cos(phi)
//    dy = (z_scint1 - z_scintN) * tan(theta) * sin(phi)
//  Le point d'arrivee est (xt+dx, yt+dy) et doit rester
//  dans [-L/2, L/2] x [-e/2, e/2].
// -------------------------------------------------------
G4bool MyPrimaryGenerator::IsInAcceptance(
    G4double xt, G4double yt,
    G4double theta, G4double phi) const
{
    const G4double halfX = 49.5*CLHEP::cm / 2.0;
    const G4double halfY =  2.0*CLHEP::cm / 2.0;
    const G4double z1    =  0.16*CLHEP::m;
    const G4double z2    =  0.;
    const G4double z3    = -0.16*CLHEP::m;

    G4double tanT   = std::tan(theta);
    G4double cosPhi = std::cos(phi);
    G4double sinPhi = std::sin(phi);

    // Verifier Scint2
    G4double dz2 = z1 - z2;
    G4double x2  = xt + dz2 * tanT * cosPhi;
    G4double y2  = yt + dz2 * tanT * sinPhi;
    if (std::abs(x2) > halfX || std::abs(y2) > halfY) return false;

    // Verifier Scint3
    G4double dz3 = z1 - z3;
    G4double x3  = xt + dz3 * tanT * cosPhi;
    G4double y3  = yt + dz3 * tanT * sinPhi;
    if (std::abs(x3) > halfX || std::abs(y3) > halfY) return false;

    return true;
}

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleTable *table = G4ParticleTable::GetParticleTable();

    G4ParticleDefinition *particle =
        (G4UniformRand() < 1.3 / 2.3)
        ? table->FindParticle("mu-")
        : table->FindParticle("mu+");
    fParticleGun->SetParticleDefinition(particle);

    G4double Ekin_GeV = SampleEnergy();
    fParticleGun->SetParticleEnergy(Ekin_GeV * CLHEP::GeV);

    // ---------------------------------------------------
    //  Tirage dans le cone d'acceptance :
    //  On tire (theta, phi, xt, yt) jusqu'a ce que la
    //  trajectoire intercepte les 3 scintillateurs.
    //  En pratique converge en quelques essais.
    // ---------------------------------------------------
    const G4double scintHalfX = 49.5*CLHEP::cm / 2.0;
    const G4double scintHalfY =  2.0*CLHEP::cm / 2.0;
    const G4double z_scint1   =  0.16*CLHEP::m;

    G4double theta, phi, xt, yt;
    G4int    attempts = 0;

    do {
        theta = SampleTheta();
        phi   = SamplePhi();
        xt    = (G4UniformRand() - 0.5) * 2.0 * scintHalfX;
        yt    = (G4UniformRand() - 0.5) * 2.0 * scintHalfY;
        attempts++;
        // Securite : si trop d'essais (muon tres incline), on accepte quand meme
        // pour ne pas boucler indefiniment (rare pour theta < 70 deg)
        if (attempts > 1000) break;
    } while (!IsInAcceptance(xt, yt, theta, phi));

    G4double sinT = std::sin(theta);
    G4double cosT = std::cos(theta);

    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(
        sinT * std::cos(phi),
        sinT * std::sin(phi),
        -cosT
    ));

    // Remonter la trajectoire jusqu'au plan z = +3 m
    const G4double z_start = 3.0 * CLHEP::m;
    G4double t_back = (z_start - z_scint1) / cosT;

    G4double x0 = xt - t_back * sinT * std::cos(phi);
    G4double y0 = yt - t_back * sinT * std::sin(phi);

    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z_start));
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
