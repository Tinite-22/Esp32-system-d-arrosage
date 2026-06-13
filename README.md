# ESP32: Système d'Arrosage Connecté 

![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif)
![API: Telegram](https://img.shields.io/badge/API-Telegram-2CA5E0?style=for-the-badge&logo=telegram)
![Language: C++](https://img.shields.io/badge/Language-C++-green?style=for-the-badge&logo=c%2B%2B)
![Status: Stable](https://img.shields.io/badge/Status-Stable-brightgreen?style=for-the-badge)

Ce projet transforme un microcontrôleur ESP32 en un assistant d'arrosage intelligent nommé **Jarvis**. Connecté à l'API Telegram via une liaison Wi-Fi sécurisée, le système permet de déclencher une pompe ou une électrovanne à distance. 

L'avantage principal de ce projet réside dans le fait que l'Esp32 fonction en STA mode donc il se connecte à internete ce qui fait que tant que le système est connecté à internete, peut importe votre position dans le monde, vous pouvez lancer un arrosage.

## Fonctionnalités

- **Interface Interactive "Jarvis" :** Répond de manière personnalisée aux commandes de l'utilisateur.
- **Arrosage Temporisé Non Bloquant :** Utilisation de la fonction native `millis()` pour gérer un cycle d'arrosage de 5 secondes sans jamais geler le microcontrôleur.
- **Filtre de Sécurité Avancé :** Seul l'administrateur possédant l'identifiant unique (`CHAT_ID_AUTORISE`) peut interagir avec le système. Les tentatives extérieures sont immédiatement bloquées et logguées sur le port série.
- **Protection Anti-Spam :** Empêche le lancement d'un second cycle d'arrosage si un est déjà en cours d'exécution.

---

## Configuration Matérielle

| Composant | Broche ESP32 (GPIO) | Mode | État par Défaut | Description |
| :--- | :--- | :--- | :--- | :--- |
| **Module Relais** | `GPIO 26` | Sortie (`OUTPUT`) | `HIGH` (Éteint) | Commande la pompe à eau ou l'électrovanne. |

---

## Configuration du Bot Telegram

Pour l'initialisation du système, vous devez générer des identifiants uniques auprès des services de Telegram :

### Étape 1 : Créer le Bot (Obtenir le `BOT_TOKEN`)
1. Ouvrez Telegram et lancez une discussion avec le compte officiel **@BotFather**.
2. Envoyez la commande `/newbot` et suivez les instructions pas à pas.
3. Copiez le **Token d'accès API** fourni à la fin du processus (ex: `123456789:ABCdefGhIJKlmNo...`).
4. Collez cette valeur dans le code au niveau de la variable `BOT_TOKEN`.

### Étape 2 : Récupérer votre identifiant (Obtenir le `CHAT_ID_AUTORISE`)
1. Sur Telegram, ouvrez une discussion avec le bot **@userinfobot** (ou **@RawDataBot**) et cliquez sur *Démarrer*.
2. Notez la suite de chiffres affichée sous la ligne `Id:`.
3. Collez cet identifiant dans votre code au niveau de la variable `CHAT_ID_AUTORISE`.
4. **Important :** Cherchez ensuite votre propre bot créé à l'étape 1 et envoyez-lui le message `/start` pour lui donner l'autorisation de vous écrire.

---

## Explication du Code (Lignes Essentielles)

### 1. Le Filtre de Sécurité
```cpp
if (chat_id != CHAT_ID_AUTORISE) {
  bot.sendMessage(chat_id, "⛔ Accès refusé...", "");
  Serial.println("ALERTE : Tentative d'accès non autorisée bloquée !");
  continue;
}

```

> **Pourquoi c'est essentiel :** Telegram est un réseau public. Sans ces lignes, n'importe qui trouvant le nom de votre bot pourrait inonder votre maison. Ce filtre compare instantanément l'expéditeur à votre identifiant enregistré et rejette immédiatement l'intrus.

### 2. L'allumage en "Active Low"

```cpp
digitalWrite(brocheRelais, LOW); // Allume l'arrosage

```

> **Pourquoi c'est essentiel :** La majorité des modules relais pour Arduino/ESP32 fonctionnent en logique inversée. Envoyer un signal `LOW` (0V) ferme le circuit mécanique pour alimenter la pompe, tandis que `HIGH` (3.3V) ouvre le circuit pour couper le courant.

### 3. L'Arrosage Asynchrone (Non Bloquant)

```cpp
if (arrosageEnCours && (millis() - tempsDebutArrosage >= dureeArrosage)) {
  digitalWrite(brocheRelais, HIGH); // Coupe l'eau
  arrosageEnCours = false;
}

```

> **Pourquoi c'est essentiel :** Utiliser un traditionnel `delay(5000)` bloquerait l'ESP32 pendant 5 secondes complètes, rendant le bot incapable de lire les nouveaux messages ou de traiter des urgences. Ici, le code enregistre l'heure de départ avec `millis()` et passe son chemin. La boucle `loop()` tourne des milliers de fois par seconde, et dès que l'écart temporel atteint `dureeArrosage` (5000ms), le relais s'éteint automatiquement.

---

## Commandes Utilisateur

Envoyez les mots-clés suivants depuis votre application Telegram :

* `Jarvis` : Permet de tester la liaison et de recevoir un message de bienvenue de votre assistant.
* `Lance l'arrosage` : Déclenche le cycle d'arrosage automatique de 5 secondes.

---
