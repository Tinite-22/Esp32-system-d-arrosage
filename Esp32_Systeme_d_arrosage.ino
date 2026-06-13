#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// =========================================================================
// 1. CONFIGURATION DU RÉSEAU ET DU BOT
// =========================================================================
const char* ssid = "";
const char* password = "";

const String BOT_TOKEN = "";
const String CHAT_ID_AUTORISE = "";

// =========================================================================
// 2. CONFIGURATION DU MATÉRIEL (Broches / GPIO)
// =========================================================================
const int brocheRelais = 26;

// =========================================================================
// 3. OBJETS DE COMMUNICATION ET VARIABLES GLOBALES
// =========================================================================
WiFiClientSecure clientSecurise;
UniversalTelegramBot bot(BOT_TOKEN, clientSecurise);

unsigned long derniereVerification = 0;
const int delaiVerification = 1000;

// --- NOUVELLES VARIABLES POUR L'ARROSAGE NON BLOQUANT ---
bool arrosageEnCours = false;
unsigned long tempsDebutArrosage = 0;
const unsigned long dureeArrosage = 5000;  // Durée en millisecondes (5 secondes)

// =========================================================================
// 4. FONCTION POUR TRAITER LES MESSAGES REÇUS
// =========================================================================
void traiterNouveauxMessages(int nombreDeMessages) {

  for (int i = 0; i < nombreDeMessages; i++) {

    String chat_id = bot.messages[i].chat_id;
    String texteRecu = bot.messages[i].text;
    String nomExpediteur = bot.messages[i].from_name;

    Serial.print("Message de ");
    Serial.print(nomExpediteur);
    Serial.print(" (ID: ");
    Serial.print(chat_id);
    Serial.print(") : ");
    Serial.println(texteRecu);

    // --- LE FILTRE DE SÉCURITÉ ---
    if (chat_id != CHAT_ID_AUTORISE) {
      bot.sendMessage(chat_id, "⛔ Accès refusé. Tu n'as pas l'autorisation de contrôler ce système.", "");
      Serial.println("ALERTE : Tentative d'accès non autorisée bloquée !");
      continue;
    }
    // ------------------------------

    if (texteRecu == "Jarvis") {
      String messageBienvenue = "Salut patron ! Le système est sécurisé.\n";
      messageBienvenue += "Que voulez-vous que je fasse?";
      bot.sendMessage(chat_id, messageBienvenue, "");
    }

    else if (texteRecu == "Lance l'arrosage") {
      // On vérifie qu'un arrosage n'est pas DÉJÀ en cours
      if (!arrosageEnCours) {
        bot.sendMessage(chat_id, "Arrosage lancé 💧...", "");

        digitalWrite(brocheRelais, LOW);
        Serial.println("Relais ALLUMÉ");

        // On enregistre le moment exact où on a allumé l'eau et on change l'état
        arrosageEnCours = true;
        tempsDebutArrosage = millis();
      } else {
        bot.sendMessage(chat_id, "L'arrosage est déjà en cours ! ⏳", "");
      }
    }

    else {
      bot.sendMessage(chat_id, "Commande inconnue..", "");
    }
  }
}

// =========================================================================
// 5. INITIALISATION
// =========================================================================
void setup() {
  Serial.begin(115200);

  pinMode(brocheRelais, OUTPUT);
  digitalWrite(brocheRelais, HIGH);

  Serial.print("Connexion au Wi-Fi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connecté !");

  clientSecurise.setInsecure();
}

// =========================================================================
// 6. BOUCLE PRINCIPALE
// =========================================================================
void loop() {

  // 1. GESTION DE LA RÉCEPTION DES MESSAGES TELEGRAM
  if (millis() - derniereVerification > delaiVerification) {
    int nombreDeMessages = bot.getUpdates(bot.last_message_received + 1);

    while (nombreDeMessages) {
      Serial.println("Nouveau message !");
      traiterNouveauxMessages(nombreDeMessages);
      nombreDeMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    derniereVerification = millis();
  }

  // 2. GESTION DE L'ARROSAGE EN TÂCHE DE FOND
  // Si l'arrosage est en cours ET que le temps écoulé dépasse la durée prévue
  if (arrosageEnCours && (millis() - tempsDebutArrosage >= dureeArrosage)) {

    digitalWrite(brocheRelais, HIGH);  // On coupe l'eau
    Serial.println("Relais ÉTEINT");

    // On prévient le patron que c'est fini
    bot.sendMessage(CHAT_ID_AUTORISE, "Arrosage terminé ✅!", "");

    // On réinitialise la variable pour autoriser un prochain arrosage
    arrosageEnCours = false;
  }
}