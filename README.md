# Simulation d'un détecteur à muons cosmiques — Geant4

Simulation d'un détecteur de muons composé de trois scintillateurs plastiques et d'un bloc de béton d'épaisseur variable. Le programme détecte les muons cosmiques par coïncidence triple et enregistre l'énergie déposée dans chaque scintillateur sous forme d'histogrammes ROOT.

Les muons sont générés avec un **spectre en énergie et une distribution angulaire réalistes**, basés sur les paramétrisations de [Shukla & Sankrith (2016)](https://arxiv.org/abs/1606.06907).

---

## Prérequis

- **Geant4** ≥ 10.7 (compilé avec `ui_all` et `vis_all`)
- **ROOT** (pour l'analyse des histogrammes)
- **CMake** ≥ 3.16
- **C++17**

---

## Compilation

```bash
# Depuis le dossier du projet
cmake -B build
cd build
make -j4
```

L'exécutable `sim` est créé dans le dossier `build/`. Les macros `init_vis.mac`, `run.mac` et `plot.C` sont copiées automatiquement dans `build/`.

---

## Lancer la simulation

### Mode interactif (avec fenêtre 3D)

```bash
./sim
```

La fenêtre OpenGL s'ouvre avec la géométrie affichée. Pour lancer des événements :

```
Idle> /run/beamOn 10
Idle> /run/beamOn 100
```

> **Remarque** : ça marche tout aussi bien avec le petit bouton vert pour un seul évènement :)

### Mode batch (sans fenêtre, pour statistiques)

```bash
./sim run.mac
```

Lance les muons et enregistre les résultats dans `muon_detector.root`. Aucune fenêtre graphique n'est ouverte. Pour changer le nombre de muons envoyés, modifier dans `run.mac` :
```
/run/beamOn 10000
```

---

## Géométrie

```
       muons cosmiques (distribution réaliste)
       ↓ ↓ ↓ ↓ ↓ ↓ ↓ ↓

    ┌────────────────────────────────────┐
    │              BÉTON                 │   z = +2 m  (épaisseur variable)
    └────────────────────────────────────┘

    ┌────────────────────────────────────┐   Scint1  z = +16 cm   (49.5 × 2 × 12.5 cm)
    └────────────────────────────────────┘

    ┌────────────────────────────────────┐   Scint2  z =   0 cm
    └────────────────────────────────────┘

    ┌────────────────────────────────────┐   Scint3  z = -16 cm
    └────────────────────────────────────┘
```

Le monde fait **4 m × 4 m × 4 m**, centré à l'origine. Les muons sont générés depuis un plan horizontal à `z = +3 m`, au-dessus du béton.

---

## Paramètres de la simulation

### Épaisseur du béton

En mode interactif :
```
Idle> /det/setConcreteThickness 200 mm
```

Dans `run.mac` :
```
/det/setConcreteThickness 1000 mm
```

La géométrie se reconstruit automatiquement.

### Modèle de distribution angulaire

Deux modèles sont disponibles, à choisir dans `run.mac` :

```
/gun/angularModel 0    # cos²θ  — approximation Terre plate (défaut)
/gun/angularModel 1    # D(θ)   — Terre sphérique (meilleur à grand angle)
```

| Modèle | Formule | Validité |
|---|---|---|
| `0` — cos²θ | Φ(θ) ∝ cos²θ  (Eq. 11) | θ < 70°, formule classique |
| `1` — D(θ)  | Φ(θ) ∝ D(θ)^{−(n−1)} (Eq. 9) | Tous angles, plus précis au-delà de 70° |

Les deux sont tirés par **rejection sampling** sur θ ∈ [0°, 70°].

### Spectre en énergie

Le spectre suit la paramétrisation de Shukla & Sankrith 2016 (Eq. 2, mesures de Tsukuba) :

```
I(E) ∝ (E₀ + E)^{−n} × (1 + E/ε)^{−1}
```

| Paramètre | Valeur | Rôle |
|---|---|---|
| `n = 3.01` | indice spectral | pente de la loi de puissance |
| `E₀ = 4.29 GeV` | perte d'énergie atmosphérique | ramollit le spectre à basse énergie |
| `ε = 854 GeV` | transition pion/kaon | correction à haute énergie |
| Domaine | 1 — 1000 GeV | gamme physique réaliste |

Le tirage est effectué par rejection sampling avec une proposition log-uniforme, particulièrement efficace pour une loi de puissance. Le rapport μ⁻/μ⁺ = 1.3 est appliqué à chaque événement.

---

## Fichiers de sortie

Après un run batch, le fichier `muon_detector.root` est créé dans `build/`. Il contient :

| Objet ROOT | Contenu |
|---|---|
| `Edep_Scint1` | Énergie déposée dans Scint1 (MeV), 100 bins |
| `Edep_Scint2` | Énergie déposée dans Scint2 (MeV), 100 bins |
| `Edep_Scint3` | Énergie déposée dans Scint3 (MeV), 100 bins |
| `Edep_Coincidence` | Énergie totale des événements en coïncidence triple (MeV) |
| `Edep_S1_vs_S3` | Corrélation 2D Scint1 vs Scint3 (100×100 bins) |

Un événement est compté en **coïncidence triple** si les trois scintillateurs déposent chacun plus de **0.1 MeV**.

---

## Visualiser les histogrammes

Une macro ROOT `plot.C` est fournie. Elle affiche tous les histogrammes en mode `hist` et sauvegarde des PNG :

```bash
cd build
root -l plot.C
```

Fichiers PNG générés :
- `edep_scintillateurs.png` — les 3 scintillateurs côte à côte
- `coincidence.png` — énergie totale en coïncidence triple
- `correlation_2D.png` — carte de corrélation Scint1 vs Scint3

Pour ouvrir le fichier ROOT manuellement :

```bash
root -l muon_detector.root
root [0] Edep_Scint1->Draw("hist")
```

---

## Structure du projet

```
.
├── sim.cc              # Main
├── construction.hh/cc  # Géométrie (scintillateurs, béton, monde)
├── generator.hh/cc     # Générateur de muons cosmiques réaliste
├── physics.hh/cc       # Liste de physique (EM standard + Decay)
├── detector.hh/cc      # Sensitive detector (énergie déposée)
├── event.hh/cc         # EventAction (coïncidence, remplissage histos)
├── run.hh/cc           # RunAction (création et écriture ROOT)
├── action.hh/cc        # Initialisation des actions
├── init_vis.mac        # Macro de visualisation 3D
├── run.mac             # Macro batch (paramètres principaux)
├── plot.C              # Macro ROOT d'analyse
└── CMakeLists.txt
```

---

## Paramètres modifiables

| Paramètre | Fichier | Valeur par défaut |
|---|---|---|
| Épaisseur béton | `run.mac` ou commande UI | 1000 mm |
| Modèle angulaire | `run.mac` (`/gun/angularModel`) | 0 (cos²θ) |
| Seuil coïncidence | `event.hh` (`kThreshold`) | 0.1 MeV |
| Nombre d'événements | `run.mac` | 10000 |
| Bins des histogrammes | `run.cc` | 100 |
| Domaine en énergie | `generator.hh` (`kEmin`, `kEmax`) | 1 — 1000 GeV |
