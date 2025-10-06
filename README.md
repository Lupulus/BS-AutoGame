# BS-AutoGame (Emul-AutoGame)

## 📋 Description

BS-AutoGame est une application Qt développée en C++ qui utilise OpenCV pour l'automatisation de jeux. L'application fournit une interface graphique complète avec plusieurs modules pour la gestion de scénarios, la configuration de paramètres, et la surveillance des logs.

## 🏗️ Architecture du Projet

### Structure des Fichiers

```
BS-AutoGame/
├── main.cpp                 # Point d'entrée de l'application
├── cgui.cpp/h/ui           # Interface graphique principale
├── chelp.cpp/h/ui          # Module d'aide
├── clogconsole.cpp/h/ui    # Console de logs
├── cparam.cpp/h/ui         # Configuration des paramètres
├── cscenario.cpp/h         # Gestion des scénarios
├── cstart.cpp/h            # Module de démarrage
├── ressources.qrc          # Fichier de ressources Qt
├── ressources/             # Dossier des ressources
│   ├── images/            # Images et icônes
│   ├── adb/               # Outils ADB (Android Debug Bridge)
│   └── opencv_world*.dll  # Bibliothèques OpenCV
└── build/                 # Dossier de compilation
```

### Modules Principaux

- **CGUI** : Interface graphique principale
- **CHelp** : Système d'aide intégré
- **CLogConsole** : Console pour l'affichage des logs
- **CParam** : Gestion des paramètres de configuration
- **CScenario** : Gestion et exécution des scénarios d'automatisation
- **CStart** : Module de démarrage et initialisation

## 🛠️ Prérequis

### Logiciels Requis

1. **Qt 6.8.0** avec MinGW 64-bit
2. **OpenCV 4.9.0** compilé avec MinGW
3. **CMake** (pour la compilation d'OpenCV)
4. **Git** (pour le contrôle de version)

### Modules Qt Utilisés

- `Qt Core`
- `Qt GUI`
- `Qt Widgets`
- `Qt Multimedia`
- `Qt MultimediaWidgets`
- `Qt Concurrent`

## 📦 Installation

### 1. Installation de Qt

1. Téléchargez Qt 6.8.0 depuis [qt.io](https://www.qt.io/download)
2. Installez Qt avec le compilateur MinGW 64-bit
3. Ajoutez le chemin Qt à votre PATH système :
   ```
   D:\Qt\6.8.0\mingw_64\bin
   ```

### 2. Compilation d'OpenCV

⚠️ **Important** : OpenCV doit être compilé avec CMake et MinGW pour être compatible avec Qt.

1. Téléchargez OpenCV 4.9.0 depuis [opencv.org](https://opencv.org/releases/)
2. Suivez la procédure détaillée : [How to setup Qt and OpenCV on Windows](https://wiki.qt.io/How_to_setup_Qt_and_openCV_on_Windows)
3. Compilez OpenCV avec CMake en utilisant MinGW
4. Assurez-vous que les chemins ne contiennent **aucun espace**

### 3. Configuration des Variables d'Environnement

Ajoutez les chemins suivants à votre variable PATH système :
```
D:\Qt\6.8.0\mingw_64\bin
C:\Users\[VotreNom]\Desktop\Dossier-General\DevCoding\OpenCV-4.9.0MinGW\install\bin
```

### 4. Clonage du Projet

```bash
git clone [URL_DU_REPOSITORY]
cd BS-AutoGame
```

## 🔧 Compilation

### Méthode 1 : Qt Creator (Recommandée)

1. Ouvrez Qt Creator
2. Ouvrez le fichier `BS-AutoGame.pro`
3. Configurez le kit de développement (Qt 6.8.0 MinGW 64-bit)
4. Compilez le projet (Ctrl+B)

### Méthode 2 : Ligne de Commande

```bash
# Génération du Makefile
qmake BS-AutoGame.pro

# Compilation
mingw32-make

# Ou pour une compilation en mode release
mingw32-make release
```

## 🚀 Exécution

### Prérequis d'Exécution

Assurez-vous que les DLL suivantes sont présentes dans le dossier `ressources/` :
- `opencv_world4100.dll` (version release)
- `opencv_world4100d.dll` (version debug)

### Lancement de l'Application

```bash
# Depuis le dossier de compilation
./Emul-AutoGame.exe

# Ou double-cliquez sur l'exécutable dans l'explorateur
```

## 📁 Structure des Ressources

### Images
- `background1.jpg` : Image de fond de l'application
- `icon.ico` : Icône de l'application
- `Scena-airforce/` : Dossier contenant les ressources pour les scénarios

### Outils ADB
- `adb.exe` : Android Debug Bridge
- `AdbWinApi.dll` : API Windows pour ADB
- `AdbWinUsbApi.dll` : API USB Windows pour ADB

## ⚙️ Configuration

### Fichier de Projet (.pro)

Le fichier `BS-AutoGame.pro` contient :
- Configuration C++17
- Modules Qt requis
- Chemins vers OpenCV
- Configuration de l'icône de l'application

### Chemins OpenCV

Les chemins OpenCV sont configurés dans le fichier `.pro` :
```qmake
INCLUDEPATH += "C:\Users\Clement\Desktop\Dossier-General\DevCoding\OpenCV-4.9.0MinGW\install\include"
LIBS += $$files("C:/Users/Clement/Desktop/Dossier-General/DevCoding/OpenCV-4.9.0MinGW/install/x64/mingw/lib/*.a")
LIBS += $$files("C:/Users/Clement/Desktop/Dossier-General/DevCoding/OpenCV-4.9.0MinGW/install/x64/mingw/bin/*.lib")
```

## 🐛 Dépannage

### Problèmes Courants

1. **Erreur de compilation OpenCV**
   - Vérifiez que OpenCV est compilé avec MinGW
   - Assurez-vous que les chemins ne contiennent pas d'espaces
   - Vérifiez les variables d'environnement PATH

2. **DLL manquantes à l'exécution**
   - Copiez les DLL OpenCV dans le dossier de l'exécutable
   - Vérifiez que les DLL Qt sont accessibles via PATH

3. **Erreurs de linkage**
   - Vérifiez la compatibilité des versions (Qt 6.8.0 + OpenCV 4.9.0)
   - Assurez-vous d'utiliser le même compilateur (MinGW 64-bit)

### Logs et Débogage

L'application dispose d'un module de console de logs (`CLogConsole`) pour le débogage en temps réel.

## 🤝 Contribution

1. Forkez le projet
2. Créez une branche pour votre fonctionnalité (`git checkout -b feature/AmazingFeature`)
3. Committez vos changements (`git commit -m 'Add some AmazingFeature'`)
4. Poussez vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrez une Pull Request

## 📝 Licence

Ce projet est sous licence [LICENCE_TYPE]. Voir le fichier `LICENSE` pour plus de détails.

## 📞 Support

Pour toute question ou problème :
- Ouvrez une issue sur GitHub
- Consultez la documentation Qt : [doc.qt.io](https://doc.qt.io/)
- Consultez la documentation OpenCV : [docs.opencv.org](https://docs.opencv.org/)

## 🔄 Versions

- **Qt** : 6.8.0
- **OpenCV** : 4.9.0
- **C++** : C++17
- **Compilateur** : MinGW 64-bit

---

**Note** : Ce projet nécessite une configuration spécifique de l'environnement de développement. Suivez attentivement les instructions d'installation pour éviter les problèmes de compilation et d'exécution.