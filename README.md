# Simulation d'un détecteur à muons cosmiques — Geant4

Simulation d'un télescope à muons composé de trois scintillateurs plastiques et d'un bloc de béton d'épaisseur variable. Le programme détecte les muons cosmiques par coïncidence triple et enregistre l'énergie déposée dans chaque scintillateur sous forme d'histogrammes ROOT.

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
mkdir build && cd build
cmake ..
make -j4
```

L'exécutable `sim` est créé dans le dossier `build/`. Les macros `init_vis.mac` et `run.mac` sont copiées automatiquement dans `build/`.

---

## Lancer la simulation

### Mode interactif (avec fenêtre 3D)

```bash
./sim
```

La fenêtre OpenGL s'ouvre avec la géométrie affichée. Pour lancer des événements :

```
Idle> /run/beamOn 1
Idle> /run/beamOn 10
```

> **Conseil** : utiliser `/run/beamOn 1` pour voir une trajectoire à la fois sans clignotement.

### Mode batch (sans fenêtre, pour statistiques)

```bash
./sim run.mac
```

Lance 1000 muons et enregistre les résultats dans `muon_detector.root`. Aucune fenêtre graphique n'est ouverte.

---

## Géométrie

```
        muon (↓ -Z, 10 GeV)
             │
    ┌────────┴────────┐   Scint1  z = +16 cm   (49.5 × 2 × 12.5 cm)
    └─────────────────┘
    
    ┌────────────────────────────────────┐
    │              BÉTON                 │   z = +2 m  (épaisseur variable)
    └────────────────────────────────────┘
    
    ┌────────────────────────────────────┐   Scint2  z =   0 cm
    └────────────────────────────────────┘
    
    ┌────────────────────────────────────┐   Scint3  z = -16 cm
    └────────────────────────────────────┘
```

Le monde fait **4 m × 4 m × 4 m**, centré à l'origine.

---

## Changer l'épaisseur du béton

### En mode interactif

```
Idle> /det/setConcreteThickness 200 mm
```

### Dans `run.mac`

Modifier la ligne :
```
/det/setConcreteThickness 500 mm
```

La géométrie se reconstruit automatiquement.

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
├── generator.hh/cc     # Canon à muons (mu-, 10 GeV, vertical)
├── physics.hh/cc       # Liste de physique (EM standard + Decay)
├── detector.hh/cc      # Sensitive detector (énergie déposée)
├── event.hh/cc         # EventAction (coïncidence, remplissage histos)
├── run.hh/cc           # RunAction (création et écriture ROOT)
├── action.hh/cc        # Initialisation des actions
├── init_vis.mac        # Macro de visualisation 3D
├── run.mac             # Macro batch
├── plot.C              # Macro ROOT d'analyse
└── CMakeLists.txt
```

---

## Paramètres modifiables

| Paramètre | Fichier | Valeur par défaut |
|---|---|---|
| Épaisseur béton | `run.mac` ou commande UI | 1000 mm |
| Énergie du muon | `generator.cc` | 10 GeV |
| Seuil coïncidence | `event.hh` (`kThreshold`) | 0.1 MeV |
| Nombre d'événements | `run.mac` | 1000 |
| Bins des histogrammes | `run.cc` | 100 |
