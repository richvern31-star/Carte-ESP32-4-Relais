/*
   ============================================================
   Serveur Web ESP32-S3  —  VERSION COMPACTE OPTIMISÉE
   ------------------------------------------------------------
   - Commande de N sorties numériques (définies dans PINS[])
   - Interface Web en grille (2 boutons par ligne), adaptée aux smartphones
   - Affichage de l'état de chaque sortie
   - Code factorisé grâce à un tableau de broches et des boucles,
     ce qui évite de dupliquer le code pour chaque sortie.
   - Le tableau JavaScript "pins" utilisé côté navigateur est
     généré AUTOMATIQUEMENT à partir de PINS[] : il n'y a
     donc plus besoin de le modifier à la main si PINS[] change.

   ------------------------------------------------------------
   OPTIMISATIONS RÉSEAU (version >8 sorties sans blocage) :
   1. Génération des "cases" HTML dans un buffer String unique,
      envoyé en UN SEUL client.print() -> beaucoup moins de
      paquets TCP, page plus rapide.
   2. En-tête "Connection: close" -> libère le socket rapidement.
   3. Intervalle AJAX porté de 200 ms à 500 ms côté navigateur.
   4. Verrou JS "requeteEnCours" -> empêche l'empilement de
      requêtes AJAX si une réponse tarde.

   ------------------------------------------------------------
   NOUVELLES OPTIONS DE ROBUSTESSE AJOUTÉES :
   1. Timeout de lecture client : évite qu'un client silencieux
      ou une requête incomplète ne bloque loop() indéfiniment.
   2. Reconnexion WiFi automatique en cas de coupure (non-bloquante avec millis()).
   3. Protection contre une ligne de requête anormalement longue
      (sécurité anti-débordement mémoire).
   4. Noms/labels personnalisés par sortie : tableau NOMS[] en
      parallèle de PINS[], affiché sur chaque case à la place de
      "Sortie Dx" (ex: "Lampe salon", "Prise cuisine"...).
   5. Utilisation de F() et reserve() pour empêcher la fragmentation de la RAM.
   6. Analyseur de commande optimisé (évite les allocations de String répétées).
   
      adresse IP trouvée  (affichée dans le moniteur série au démarrage)
   ============================================================


 REMARQUE ESP32-S3 : Choix de GPIO sûres et sans risques de conflit au boot.
👍 Broches recommandées pour vos sorties (PINS)
 Les broches (GPIO) à utiliser sur les versions ESP32
//⚠️---------------(Adapté a ESP32C3 Dev Module)-------------------------------
//const byte PINS[] = { 1, 2, 3, 4, 5, 6, 7, 8};   // Tableau des broches GPIO  de 1 a 10 et 20 21  Pin 8 allume la led bleue
//⚠️---------------(Adapté a ESP32 S3-N16R8----------------------------
//const byte PINS[] = {1, 2, 9, 10, 11, 12, 13, 14};   // Tableau broches GPIO libre 1, 2, 9, 10, 11, 12, 13, 14, 21, 38, 47, 48 ❌ NE PAS UTILISER 3, 35, 36, 37, 45, 46
//⚠️---------------(Adapté a ESP32-Dev Module---------------------------
const byte PINS[] = {4, 16, 17, 18, 19, 21, 22, 23};   // Tableau des broches GPIO libres 4, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33(❌ NE PAS UTILISER 6, 7 )

//------------------------------------------------------------
GPIO Utilisable
//------------------------------------------------------------

📌ESP32-C3
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21 — correct.

GPIO 11 à 17 sont réservés à la mémoire flash SPI interne → à ne jamais utiliser (vous les excluez bien).
GPIO 18 et 19 sont utilisés pour l'USB-JTAG natif ; vous les excluez, ce qui est prudent si vous comptez utiliser le port USB natif en debug/série. Sinon ils restent utilisables.
GPIO 0 : broche de strapping (mode boot). Vous l'excluez aussi, ce qui est une approche prudente pour un débutant, même si en pratique elle est souvent utilisée (bouton BOOT sur les devkits).

📌ESP32-S3 N16R8 
Sur la variante N16R8 (16 Mo flash + 8 Mo PSRAM en mode Octal SPI), ce sont les broches GPIO33 à GPIO37 qui sont dédiées à la PSRAM octale (pas seulement 33-34). Donc la liste correcte est :

Ne pas utiliser : 0, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37

Détails :

GPIO 22 à 25 : n'existent tout simplement pas sur le chip ESP32-S3 (numérotation sautée).
GPIO 26 à 32 : réservés à la flash SPI.
GPIO 33 à 37 : réservés à la PSRAM en mode octal (spécifique aux modules N16R8/N8R8, pas aux modules sans PSRAM octale).
GPIO 0, 45, 46 : broches de strapping, utilisables mais avec précaution (mode boot / tension flash).

📌ESP32-D WROOM (WROOM-32D)

De 0 à 39, entrée seule 34, 35, 36, 39 , ne pas utiliser 0, 6, 7, 8, 9, 10, 11 ✓ (6-11 = flash SPI intégrée au module).

Quelques nuances à connaître en plus :

GPIO 1 et GPIO 3 : utilisées par l'UART0 (TX/RX programmation et moniteur série). Elles restent utilisables en GPIO générique, mais posent problème si vous avez besoin du port série en même temps.
GPIO 2 et GPIO 12 : broches de strapping (mode boot / tension flash). Généralement utilisables, mais à éviter pour des usages critiques ou si un état haut/bas au démarrage pourrait perturber le boot.
//------------------------------------------------------------

Récapitulatif

ESP32-C3
Catégorie	                                                      GPIO
Utilisables	                                     🟢  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21
Réservées flash SPI interne (interdit)	          🔴 11, 12, 13, 14, 15, 16, 17
USB-JTAG natif (à éviter si USB natif utilisé)	 🟡 18, 19
Strapping (boot) — Non utilisable                🔴 0


ESP32-S3 N16R8 (16 Mo Flash + 8 Mo PSRAM)
Catégorie	                                                         GPIO
Utilisables	                                      🟢 1–21, 38–44, 47, 48
N'existent pas sur le chip	                           22, 23, 24, 25
Réservées flash SPI	                             🔴 26, 27, 28, 29, 30, 31, 32
Réservées PSRAM Octal (spécifique N16R8)	        🔴 33, 34, 35, 36, 37
Strapping (boot) — Non utilisable                 🔴0, 45, 46


ESP32-D WROOM-32D  Choisir ESP32 Dev Module  (4 Mo Flash)
Catégorie	                                                         GPIO
Utilisables (bidirectionnel)	                     🟢 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33 -- 2 led bleue pour Dev Module
Entrée seule	                                    🟡34, 35, 36, 39
Réservées flash SPI intégrée (interdit)	         🔴6, 7, 8, 9, 10, 11
UART0 (TX/RX programmation) — conflit possible	   🟡1, 3
Strapping (boot) — Non utilisable               	🔴0
//------------------------------------------------------------



// 🔴 Repere 1
//------------------------------------------------------------
// Déclaration des sorties : il suffit de modifier ce tableau
// (ajouter/retirer un numéro de broche) pour changer le nombre
// de sorties gérées par le programme, sans toucher au reste du code.
// Le tableau JS côté navigateur sera régénéré automatiquement
// à partir de ce même tableau (voir fonction genererTableauJSPins).
//
// 🔴 Repere 2
//------------------------------------------------------------
// OPTION 4 : Noms personnalisés affichés dans l'interface Web.
// Ce tableau doit avoir EXACTEMENT le même nombre d'éléments,
// et dans le même ordre, que PINS[] ci-dessus (le nom de l'index i
// correspond à la broche PINS[i]). Modifiez simplement le texte
// entre guillemets pour renommer une sortie.


----------------------------------------
Résumé des changements PINS[] et NOMS[]:
----------------------------------------
Repère 2 : PINS[] et NOMS[] sont remplacés par un seul tableau SORTIES[] 
de type struct Sortie { byte pin; const char* nom; }. Chaque pin et son 
nom sont désormais sur la même ligne.

setup() : 
la vérification NB_NOMS != NB_SORTIES a été supprimée 
(elle n'a plus de raison d'être, un seul tableau ne peut plus se désynchroniser). 
pinMode/digitalWrite utilisent maintenant SORTIES[i].pin.

traiterCommande(), 
genererTableauJSPins(), 
genererCasesHTML(), 
envoyerEtat() : 
tous les accès PINS[i] → SORTIES[i].pin, et NOMS[i] → SORTIES[i].nom.

Le reste du programme (HTML, CSS, JS, gestion WiFi, mDNS, timeouts) n'a pas été touché.

Maintenant, pour changer un pin, tu modifies une seule ligne, par exemple :
{14, "Cuisine"},   // au lieu de {27, "Cuisine"},

//------------------------------------------------------------

// 🔴 Repere 3
  // On vérifie que le tableau NOMS[] a bien été mis à jour avec
  // le même nombre d'éléments que PINS[]. Sans cette vérification, un
  // oubli lors de l'ajout/retrait d'une sortie provoquerait un accès
  // hors tableau (comportement indéfini) lors de l'affichage de la page.

// 🔴 Repere 4
//============================================================
// verifierWiFi 
// ------------------------------------------------------------
// Vérifie à chaque tour de loop() que la carte est toujours
// connectée au réseau WiFi. En cas de coupure (box redémarrée,
// carte hors de portée temporairement...), tente une reconnexion
// automatique de manière non bloquante en respectant un intervalle.
//============================================================

// 🔴 Repere 5
//============================================================
// traiterCommande — Analyse des commandes HTTP (Optimisée)
// ------------------------------------------------------------
// Analyse la commande reçue en évitant les allocations dynamiques
// lourdes en mémoire RAM (suppression de la création de Strings temporaires).
//============================================================

// 🔴 Repere 6
//============================================================
// genererTableauJSPins — Écrit la ligne JS "let pins=[...]"
// en la construisant dynamiquement à partir de PINS[].
// Ainsi, si vous modifiez PINS[] côté Arduino, le JavaScript
// envoyé au navigateur reste automatiquement synchronisé,
// sans devoir éditer le code JS à la main.
//============================================================

/ 🔴 Repere 7
//============================================================
// genererCasesHTML — Construit en mémoire, dans une seule String,
// tout le bloc HTML des "cases" (une par sortie), afin de l'envoyer
// en UN SEUL client.print() plutôt qu'une dizaine par sortie.
// C'est ce qui règle le ralentissement observé au-delà de 8 sorties.
//============================================================

// 🔴 Repere 8
  // --- PARTIE 1 : début du document HTML, jusqu'à juste avant
  // le tableau "pins" qui doit être généré dynamiquement.
  // Ce bloc est statique, donc regroupé en une seule chaîne
  // stockée en mémoire flash (F()) plutôt qu'en dizaines de println().
  
// 🔴 Repere 9
//============================================================
// Réponse AJAX : renvoie l'état de toutes les sorties (ex: "1;0;1;0")
// Utilisée par le JavaScript pour éviter de recharger la page
// MODIFICATION : Envoie aussi la puissance et qualité réseau lues en temps réel.
//============================================================
Ajout de 4 boutons poussoir avec le mapping demandé :

BP 15 → Relais 25 (Salon)
BP 16 → Relais 26 (Cuisine)
BP 22 → Relais 32 (Jardin)
BP 23 → Relais 33 (Portail)

Un tableau BOUTONS[] qui associe chaque GPIO de bouton à son relais.
En setup(), les 4 GPIO des boutons sont configurés en INPUT_PULLUP — donc câblage attendu : 
bouton entre la broche GPIO et la masse (GND). Au repos la broche lit HIGH, elle passe à LOW quand on appuie.
Une fonction gererBoutons() appelée à chaque tour de loop(), 
avant la gestion WiFi — donc les boutons fonctionnent instantanément, 
même si le WiFi est coupé ou lent.
Un anti-rebond logiciel non bloquant (50 ms) pour éviter les faux déclenchements 
liés aux vibrations mécaniques du bouton.
Chaque appui bascule (toggle) l'état du relais correspondant, comme un interrupteur va-et-vient 
— donc ça reste cohérent avec l'état affiché sur la page web (qui se met à jour automatiquement via l'AJAX existant).

Un point important : si tu câbles les boutons différemment 
(par exemple avec une résistance pull-down vers le +3.3V plutôt que vers la masse), 
il faudra inverser la logique LOW/HIGH dans gererBoutons()

//============================================================
// ⚠️----Pour la Programmation de la carte avec ou sans Alim Extérieure-------⚠️
// Brancher le FTDI (cavalier sur 3V3) sur le port de programmation P5
// Maintenir le BP IO0 et appuyer sur EN jusqu'a l'extinction de la led verte du FTDI puis relacher EN et IO0
// Lancer la programmation
// A la fin de la programmation appuyer sur EN (Reset) pour relancer la carte
//============================================================

Visioneuse de fichier README.md
Plusieurs types de programmes peuvent afficher un fichier .md correctement mis en forme (titres, tableaux, listes...) :

Éditeurs de code (avec aperçu intégré)
VS Code — ouvre le fichier, puis Ctrl+Shift+V (ou clic droit → "Open Preview") pour voir le rendu
Notepad++ avec un plugin Markdown, ou Sublime Text

Sur GitHub / GitLab
Si tu mets ce fichier dans un dépôt Git (GitHub, GitLab...), il s'affiche automatiquement en page d'accueil du projet, entièrement formaté (c'est l'usage classique d'un README)

Applications dédiées Markdown
Typora (Windows/Mac/Linux) — éditeur WYSIWYG, très lisible
Obsidian — gratuit, pratique si tu as d'autres notes de projet
MarkText — gratuit et open source

Directement dans un navigateur
Extensions Chrome/Firefox comme "Markdown Viewer" qui rendent le .md en HTML si tu l'ouvres en local
Sinon, un fichier .md ouvert directement dans un navigateur s'affiche en texte brut (pas de mise en forme) sans extension

Sans rien installer
Le glisser dans un site comme dillinger.io ou stackedit.io (éditeurs Markdown en ligne)

*/


  