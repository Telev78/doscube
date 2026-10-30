# DosCube

**DosCube** est un Rubik's Cube 3×3×3 en 3D pour MS-DOS, écrit en C++ avec Borland C++ 3.1. Le cube s'affiche en mode graphique VGA 320×200 en 256 couleurs et se manipule entièrement à la souris : on l'oriente dans l'espace, on tourne les tranches en faisant glisser le pointeur, on le mélange, puis on essaie de le résoudre le plus vite possible, avec un chrono à la milliseconde et un compteur de coups.

Le jeu a été pensé pour tourner sur des machines modestes, jusqu'au 286 sans coprocesseur.

## Fonctionnalités

* **Cube 3D complet** : 26 pièces, rendu en mode 13h avec élimination des faces cachées et tri en profondeur.
* **Rotation de la vue en trackball** : le cube suit le pointeur quelle que soit la face présentée, sans inversion de sens.
* **Toutes les tranches** : les six faces extérieures et les trois tranches centrales se tournent d'un simple glissement de souris, avec animation.
* **Mélange aléatoire** : 30 à 40 rotations aléatoires, sur n'importe quelle tranche, animées rapidement.
* **Détection de victoire** : le jeu reconnaît un cube résolu, quelle que soit son orientation (les tranches centrales déplacent les centres).
* **Compteur de coups** : il ne compte que les coups joués après le mélange.
* **Chrono de précision** : affiché au format `mm:ss,mmm`, lu sur le compteur matériel du PIT (résolution inférieure à la microseconde), il démarre au premier coup et s'arrête à la victoire.
* **Annuler / refaire** : historique de 512 coups parcourable dans les deux sens, à la souris ou au clavier. Un nouveau coup après un retour en arrière remplace les coups restants.
* **Trois langues** : anglais, français et allemand, choisies automatiquement d'après le code pays DOS (`INT 21h`, fonction 38h), modifiables par option de ligne de commande ou à la volée avec la touche `L`.
* **Détection du processeur** : sur 286, le jeu raccourcit automatiquement les animations. Réglable avec `/C:`.
* **Aide et à propos** : option `/?` en ligne de commande, et fenêtre intégrée (bouton `?` ou touche `F1`) qui rappelle les commandes.
* **Confirmation avant de quitter** lorsqu'une partie est en cours.
* **Rendu optimisé** : arithmétique entière en point fixe (aucun coprocesseur nécessaire), mises à jour partielles de l'écran, curseur de souris dessiné par le programme pour éviter tout scintillement.

## Configuration requise

| Élément | Requis |
|---|---|
| Système | MS-DOS 3.0 ou supérieur, ou compatible (DOSBox, FreeDOS...). Avant la version 3.0, la langue ne peut pas être détectée et le jeu reste en anglais |
| Processeur | 80286 ou supérieur. Le code est compilé en instructions 286, aucun coprocesseur n'est nécessaire |
| Affichage | Carte **VGA** (ou MCGA) compatible avec le mode 13h (320×200, 256 couleurs) |
| Souris | Pilote compatible Microsoft (`INT 33h`), **2 boutons** minimum |
| Mémoire | Quelques centaines de Ko de mémoire conventionnelle (le tampon d'écran seul occupe 64 Ko) |

Le jeu a été testé sous DOSBox et sous 86Box en émulant un 286, un 386SX et un 486. Si le jeu détecte à tort un 286 (par exemple sous un gestionnaire de mémoire comme EMM386, qui place le processeur en mode virtuel), il suffit de forcer la vitesse avec `/C:FAST`.

## Comment jouer

### Souris

| Action | Effet |
|---|---|
| **Clic droit maintenu + déplacement** | Oriente le cube : la face tournée vers vous suit le pointeur |
| **Clic gauche maintenu sur une face, puis glisser** | Tourne de 90° la tranche qui contient la pièce visée, dans le sens du glissement |
| Bouton **Mélanger** (en bas à droite) | Lance un mélange aléatoire |
| Boutons **←** et **→** (en bas au centre) | Annule / refait le coup précédent / suivant |
| Bouton **?** (en haut à droite) | Ouvre la fenêtre d'aide et à propos |
| Bouton **X** (en haut à droite) | Quitte le jeu |

La tranche tournée dépend de l'endroit où l'on clique et de la direction du glissement. Cliquer sur un coin ou une arête et glisser le long d'une direction tourne la couche extérieure correspondante, glisser dans l'autre direction tourne la tranche centrale. Cliquer sur le centre d'une face tourne une tranche centrale.

### Clavier

| Touche | Effet |
|---|---|
| `M` | Mélanger |
| `U` ou `←` | Annuler le dernier coup |
| `R` ou `→` | Refaire le coup annulé |
| `L` | Changer de langue (EN → FR → DE) |
| `F1` ou `?` | Aide et à propos |
| `Échap` | Quitter (avec confirmation si une partie est en cours) |

### Déroulement d'une partie

1. Cliquez sur **Mélanger** (ou appuyez sur `M`). Le cube se mélange en quelques secondes.
2. Jouez votre premier coup : le chrono démarre et le compteur de coups commence à compter.
3. Annuler et refaire ne comptent pas comme des coups, et le chrono continue de tourner. Il est mis en pause quand la fenêtre d'aide ou la confirmation de sortie est ouverte.
4. Quand les six faces sont chacune d'une seule couleur, le chrono s'arrête et le nombre de coups s'affiche en jaune. Les boutons annuler / refaire sont alors désactivés jusqu'au prochain mélange.

### Options de ligne de commande

```
DOSCUBE [/L:EN|FR|DE] [/C:AUTO|SLOW|FAST] [/?]
```

| Option | Effet |
|---|---|
| `/L:EN`, `/L:FR`, `/L:DE` | Force la langue (par défaut : d'après le code pays DOS, anglais sinon) |
| `/C:AUTO` | Détecte le processeur : animations courtes sur 286 (**par défaut**) |
| `/C:SLOW` | Force les animations courtes |
| `/C:FAST` | Force les animations normales |
| `/?`, `-?`, `/H` | Affiche l'aide dans la langue courante et quitte |

## Compilation

### Prérequis

1. Copiez le dossier d'installation de **Borland C++ 3.1** (`BORLANDC`) à la racine du projet :
   ```text
   DOSCUBE/
   ├── BORLANDC/
   │   ├── BIN/
   │   ├── INCLUDE/
   │   ├── LIB/
   │   └── BGI/
   ├── PROJECT/
   ├── SRC/
   └── ...
   ```
2. Le makefile référence `C:\BORLANDC` et des chemins relatifs (`..\SRC`, `..\BUILD`) : le dossier racine du projet doit donc être monté comme lecteur `C:` dans DOSBox, et la compilation se lance depuis le dossier `PROJECT\`.

### Avec Visual Studio Code

Le projet fournit des tâches VS Code qui lancent DOSBox avec la bonne configuration. Adaptez d'abord les chemins dans votre fichier `.vscode/settings.json` local :

```json
{
    "dosbox.windows.installPath": "C:\\Chemin\\Vers\\Votre\\Dossier\\DOSBox",
    "dosbox.windows.executableName": "dosbox.exe",
    "dosbox.windows.configPath": "${config:dosbox.windows.installPath}\\vscode.conf",
    "dosbox.linux.binaryName": "dosbox",
    "dosbox.linux.configPath": "${userHome}/.config/dosbox/vscode.conf"
}
```

* `Ctrl+Shift+B` (ou *Terminal > Run Build Task*) lance la tâche de build par défaut.
* Les autres tâches (exécution, nettoyage du dossier de build...) sont dans la palette de commandes : `Ctrl+Shift+P`, puis `Tâches : exécuter la tâche` (ou `Tasks: Run Task`).

L'exécutable est produit dans `build\doscube.exe`.

### Avec l'IDE Borland C++

Un fichier de projet est disponible dans `PROJECT/` :

1. Lancez `BC.EXE` dans DOSBox ou sur une machine DOS, puis ouvrez le projet de `PROJECT/`.
2. Lancez la compilation (*Compile > Build all*).

### Avec le makefile

Depuis `PROJECT\`, la commande `make` produit la version Release (modèle medium, optimisation vitesse). La commande `make -DDEBUG` produit une version de débogage avec contrôle du dépassement de pile (`-N`). `make clean` supprime les objets et l'exécutable. Les dépendances sur les fichiers d'en-tête sont gérées automatiquement.

## Organisation du code

| Fichier | Rôle |
|---|---|
| `main.cpp` | Boucle principale, gestion de la souris et du clavier, déroulement d'une partie |
| `cube.cpp` / `cube.h` | Modèle du cube (26 pièces), rotations des tranches, mélange, détection de victoire |
| `control.cpp` / `control.h` | Entrées : souris, clavier, boutons, et interprétation des glissements sur une face (computeMove) |
| `video.cpp` / `video.h` |	Mode graphique 13h : détection VGA, passage et restauration du mode, tampon d’écran, synchronisation verticale |
| `renderer.cpp` / `renderer.h` | Rendu 3D en entiers, remplissage des faces, boutons, texte, curseur, orientation de la vue |
| `mouse.cpp` / `mouse.h` | Pilote de souris (`INT 33h`) |
| `history.cpp` / `history.h` | Historique des coups (annuler / refaire) |
| `timer.cpp` / `timer.h` | Chrono à la milliseconde (PIT et ticks du BIOS) |
| `lang.cpp` / `lang.h` | Textes en trois langues, choix de la langue par code pays DOS |
| `options.cpp` / `options.h` | Options de ligne de commande, aide |
| `cpu.cpp` / `cpu.h` | Détection de la classe du processeur |
| `about.cpp` / `about.h` | Fenêtres « À propos / aide » et confirmation de sortie |

## Crédits

* **Développement :** Telev, avec l'aide d'un agent IA.
* **Licence :** le code source est publié sous [The Unlicense](LICENSE) (domaine public)