# Module Relais 4 Canaux — ESP32 
#(Amazon ou AliExpress  ESP32_Relay X4_V1.1 (303E32AC411))

Contrôleur de relais WiFi pour ESP32, piloté via une page web intégrée (serveur local) et par 4 boutons poussoir physiques câblés en local.

## Présentation

Ce programme transforme une carte **ESP32-WROOM (ESP-32D Dev Module)** en serveur web local capable de piloter **4 relais** (ex : Salon, Cuisine, Jardin, Portail) :

- depuis un navigateur (PC, smartphone) sur le réseau local, via une interface web responsive avec boutons ON/OFF ;
- directement en local via **4 boutons poussoir physiques**, indépendamment du WiFi.

L'état des relais est synchronisé en temps réel sur la page web via une requête AJAX toutes les 500 ms, et affiche aussi la qualité du signal WiFi (RSSI en dBm et en %).

## Matériel

| Élément | GPIO | Rôle |
|---|---|---|
| Relais 1 | 25 | Salon |
| Relais 2 | 26 | Cuisine |
| Relais 3 | 32 | Jardin |
| Relais 4 | 33 | Portail |
| Bouton poussoir 1 | 15 | Bascule Relais 25 (Salon) |
| Bouton poussoir 2 | 16 | Bascule Relais 26 (Cuisine) |
| Bouton poussoir 3 | 22 | Bascule Relais 32 (Jardin) |
| Bouton poussoir 4 | 23 | Bascule Relais 33 (Portail) |

**Câblage des boutons poussoir** : chaque bouton relie sa broche GPIO à la masse (**GND**). Les entrées sont configurées en `INPUT_PULLUP` (résistance de tirage interne activée), donc :
- au repos → la broche lit **HIGH** ;
- bouton appuyé → la broche lit **LOW**.

⚠️ Aucune résistance externe n'est nécessaire.

### GPIO utilisables sur cette carte

`1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33`
(GPIO 34, 35, 36, 39 : entrée seule uniquement)

À éviter : `0, 6, 7, 8, 9, 10, 11` (non utilisables) — GPIO `20, 24, 28, 29, 30, 31, 37, 38` non présents sur cette puce.

## Fonctionnalités

- **Page web embarquée** générée dynamiquement (HTML/CSS/JS servis directement par l'ESP32, aucune dépendance externe).
- **Pilotage individuel** de chaque relais (ON/OFF) depuis la page web.
- **Boutons globaux** : Tout Allumer / Tout Éteindre.
- **Pilotage physique local** : 4 boutons poussoir qui basculent (toggle) directement le relais associé, sans passer par le WiFi — fonctionne même si le réseau est coupé.
- **Anti-rebond logiciel non bloquant** (50 ms) sur les boutons poussoir.
- **Actualisation automatique** de l'état affiché (AJAX, toutes les 500 ms), quelle que soit la source du changement (web ou bouton physique).
- **Accès mDNS** : la carte est joignable via `http://<nom>.local` en plus de son adresse IP.
- **Reconnexion WiFi automatique** et non bloquante en cas de coupure.
- **Popup d'informations réseau** (IP, adresse MAC, RSSI, qualité du signal) accessible depuis la page web.
- **Diagnostic série** : taper `i` dans le moniteur série affiche les informations réseau à tout moment.

## Configuration réseau

Les identifiants WiFi (SSID / mot de passe) sont définis dans un fichier séparé **`arduino_secrets.h`** (non fourni, à créer à la racine du projet) :

```cpp
#define SECRET_SSID "NomDeVotreReseauWiFi"
#define SECRET_PASS "VotreMotDePasseWiFi"
```

| Paramètre | Valeur par défaut | Description |
|---|---|---|
| Nom mDNS | `RichardV` | Accès via `http://RichardV.local` |
| Port serveur | `80` | Port HTTP standard |
| Timeout requête client | `2000 ms` | Délai max pour recevoir une requête complète |
| Intervalle reconnexion WiFi | `10000 ms` | Délai minimal entre deux tentatives |
| Anti-rebond boutons | `50 ms` | Stabilisation du signal avant validation de l'appui |

## Utilisation

1. Créer et renseigner le fichier `arduino_secrets.h` avec vos identifiants WiFi.
2. Téléverser le programme sur la carte ESP32 (IDE Arduino, gestionnaire de carte Espressif Systems).
3. Ouvrir le moniteur série (115200 bauds) pour récupérer l'adresse IP attribuée, ou utiliser directement `http://RichardV.local`.
4. Ouvrir cette adresse dans un navigateur sur le même réseau WiFi.
5. Piloter les relais depuis la page web, ou directement via les boutons poussoir physiques.

## Ajouter ou modifier une sortie

Il suffit de modifier le tableau `SORTIES[]` dans le code — le nombre de sorties (`NB_SORTIES`), le tableau JavaScript des pins et l'affichage HTML sont générés automatiquement à partir de ce tableau, sans rien à synchroniser ailleurs.

```cpp
const Sortie SORTIES[] = {
  {25, "Salon"},
  {26, "Cuisine"},
  {32, "Jardin"},
  {33, "Portail"}
};
```

De même pour les boutons poussoir, via le tableau `BOUTONS[]` :

```cpp
Bouton BOUTONS[] = {
  {15, 25, HIGH, HIGH, 0},   // BP GPIO15 -> Relais GPIO25
  {16, 26, HIGH, HIGH, 0},   // BP GPIO16 -> Relais GPIO26
  {22, 32, HIGH, HIGH, 0},   // BP GPIO22 -> Relais GPIO32
  {23, 33, HIGH, HIGH, 0}    // BP GPIO23 -> Relais GPIO33
};
```

## Prérequis

- Carte ESP32-WROOM (profil **ESP-32D Dev Module** dans l'IDE Arduino).
- Gestionnaire de cartes ESP32 par Espressif Systems (version testée : **3.3.11**).
- Bibliothèques standard `WiFi.h` et `ESPmDNS.h` (incluses avec le core ESP32).

## Notes

- Les relais sont forcés à l'état bas (éteints) au démarrage de la carte.
- La lecture des boutons poussoir et le pilotage des relais associés fonctionnent indépendamment de la connexion WiFi : ils restent opérationnels même en cas de coupure réseau.
