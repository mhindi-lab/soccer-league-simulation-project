#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

const string RESET = "\033[0m";
const string BOLD = "\033[1m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string RED = "\033[31m";
const string CYAN = "\033[36m";
const string MAGENTA = "\033[35m";
const string BLUE = "\033[34m";
const string WHITE = "\033[37m";

// -----------------------------------------
//  STRUCTS
// -----------------------------------------
struct Player {
    string name = "";
    string position = "";
    int goals = 0;
    int assists = 0;
    int yellowCards = 0;
    int redCards = 0;
    bool suspended = false;
    bool injured = false;
    int injuryGamesLeft = 0;
    int totalInjuries = 0;
    int totalRedCards = 0;
    int marketValue = 0;
    bool listedForSale = false;
    int askingPrice = 0;
    bool isSubstitute = false;
};

struct Team {
    string name = "";
    string stadium = "";
    string city = "";
    int strength = 0;
    vector<Player> players;
    int points = 0;
    int wins = 0;
    int draws = 0;
    int losses = 0;
    int goalsScored = 0;
    int goalsConceded = 0;
    int budget = 0;
};

struct Match {
    int homeIdx = 0;
    int awayIdx = 0;
    int homeGoals = 0;
    int awayGoals = 0;
    bool played = false;
    bool isCup = false;
};

struct SeasonRecord {
    int season = 0;
    string leagueWinner = "";
    string cupWinner = "";
    string topScorer = "";
    int topScorerGoals = 0;
};

struct CommentaryEvent {
    int minute = 0;
    string text = "";
    string color = WHITE;
};

// -----------------------------------------
//  GLOBALS
// -----------------------------------------
vector<Team> teams;
vector<Match> leagueSchedule;
vector<SeasonRecord> history;

int currentSeason = 1;
string leagueName = "Custom League";
bool fastDisplay = false;
bool backToMainMenu = false;
bool leagueFinishedThisSeason = false;
bool cupFinishedThisSeason = false;

// -----------------------------------------
//  HELPERS
// -----------------------------------------
void printDivider(char c = '=', int len = 60) {
    cout << CYAN;
    for (int i = 0; i < len; i++) cout << c;
    cout << RESET << "\n";
}

void printHeader(const string& t) {
    printDivider('=');
    cout << BOLD << CYAN << "  " << t << RESET << "\n";
    printDivider('=');
}

int rng(int lo, int hi) {
    if (hi < lo) return lo;
    return lo + rand() % (hi - lo + 1);
}

bool playerAvailable(const Player& p) {
    return !p.suspended && !p.injured;
}

void pressEnter() {
    cout << CYAN << "\n  [ Press ENTER to continue... ]" << RESET;
    cin.ignore(1000, '\n');
}

void typeText(const string& text, int delayMs = 15) {
    if (fastDisplay) {
        cout << text;
        return;
    }

    for (char ch : text) {
        cout << ch << flush;
#ifdef _WIN32
        Sleep(delayMs);
#else
        usleep(delayMs * 1000);
#endif
    }
}

void typeLine(const string& text, int delayMs = 15) {
    typeText(text + "\n", delayMs);
}

bool seasonAlreadySaved() {
    return !history.empty() && history.back().season == currentSeason;
}

void addEvent(vector<CommentaryEvent>& events, int minute, const string& text, const string& color) {
    CommentaryEvent e;
    e.minute = minute;
    e.text = text;
    e.color = color;
    events.push_back(e);
}

void printChronologicalEvents(vector<CommentaryEvent>& events) {
    sort(events.begin(), events.end(), [](const CommentaryEvent& a, const CommentaryEvent& b) {
        if (a.minute != b.minute) return a.minute < b.minute;
        return a.text < b.text;
        });

    for (size_t i = 0; i < events.size(); i++) {
        cout << events[i].color;
        typeLine("  " + to_string(events[i].minute) + "'  " + events[i].text);
        cout << RESET;
    }
}

string getChampionMessage(const string& teamName) {
    if (teamName == "Man City") return "Manchester is BLUE! Man City rule the league again!";
    if (teamName == "Arsenal") return "North London celebrates - Arsenal are champions!";
    if (teamName == "Liverpool") return "Anfield erupts - Liverpool are champions again!";
    if (teamName == "Chelsea") return "Blue flags fly high - Chelsea are back on top!";
    if (teamName == "Aston Villa") return "Villa Park roars - Aston Villa lift the title!";
    if (teamName == "Newcastle") return "The Toon Army celebrates - Newcastle are champions!";
    if (teamName == "Tottenham") return "North London dares to dream - Spurs are champions!";
    if (teamName == "Man United") return "Manchester is RED! United are back on the throne!";
    if (teamName == "West Ham") return "East London celebrates - West Ham are champions!";
    if (teamName == "Brighton") return "A historic day on the south coast - Brighton are champions!";

    if (teamName == "Real Madrid") return "The kings of Spain rise again - Real Madrid are champions!";
    if (teamName == "Barcelona") return "Catalonia celebrates - Barcelona are back on the throne!";
    if (teamName == "Atletico Madrid") return "The red-and-white warriors conquer Spain!";
    if (teamName == "Real Sociedad") return "A Basque masterclass - Real Sociedad are champions!";
    if (teamName == "Athletic Bilbao") return "San Mames explodes - Athletic Bilbao are champions!";
    if (teamName == "Villarreal") return "The Yellow Submarine sails to the title!";
    if (teamName == "Real Betis") return "Seville goes green and white - Betis are champions!";
    if (teamName == "Sevilla") return "Sevilla stand tallest in Spain!";
    if (teamName == "Getafe") return "A miracle in Madrid - Getafe are champions!";
    if (teamName == "Osasuna") return "Pamplona erupts - Osasuna make history!";

    if (teamName == "Inter Milan") return "The Nerazzurri reign supreme - Inter are champions!";
    if (teamName == "Napoli") return "Naples is dancing - Napoli are champions!";
    if (teamName == "AC Milan") return "The Rossoneri are back on top of Italy!";
    if (teamName == "Juventus") return "Turin celebrates - Juventus return to glory!";
    if (teamName == "Atalanta") return "Bergamo dreams come true - Atalanta are champions!";
    if (teamName == "AS Roma") return "Rome turns red and gold - Roma win the league!";
    if (teamName == "Lazio") return "Sky blue glory - Lazio are champions!";
    if (teamName == "Fiorentina") return "Florence blooms purple - Fiorentina are champions!";
    if (teamName == "Torino") return "Torino shock Italy - they are champions!";
    if (teamName == "Bologna") return "Bologna complete a historic triumph!";

    if (teamName == "PSG") return "Paris shines brightest - PSG are champions again!";
    if (teamName == "Monaco") return "Monaco conquer France in style!";
    if (teamName == "Marseille") return "The Velodrome erupts - Marseille are champions!";
    if (teamName == "Lille") return "Lille stun France and lift the title!";
    if (teamName == "Lyon") return "Lyon are back where they belong - champions!";
    if (teamName == "Lens") return "An unforgettable season - Lens are champions!";
    if (teamName == "Rennes") return "Rennes rise to the very top!";
    if (teamName == "Nice") return "The Riviera celebrates - Nice are champions!";
    if (teamName == "Strasbourg") return "A dream season ends in glory for Strasbourg!";
    if (teamName == "Nantes") return "Nantes shock the nation - champions!";

    if (teamName == "Bayern Munich") return "Bayern do it again - kings of Germany!";
    if (teamName == "Bayer Leverkusen") return "Leverkusen prove it was no fluke - champions again!";
    if (teamName == "Borussia Dortmund") return "The Yellow Wall roars - Dortmund are champions!";
    if (teamName == "RB Leipzig") return "Leipzig rise to the Bundesliga summit!";
    if (teamName == "Eintracht Frankfurt") return "Frankfurt make history - champions!";
    if (teamName == "Freiburg") return "A fairy tale in Freiburg - league winners!";
    if (teamName == "Union Berlin") return "Berlin belongs to Union - champions!";
    if (teamName == "Wolfsburg") return "Wolfsburg roar back to the top!";
    if (teamName == "Borussia M'gladbach") return "Gladbach are champions once more!";
    if (teamName == "Mainz") return "Mainz complete an unbelievable title run!";
    if (teamName == "Al Ahly") return "The Red Devils dominate Egypt again - Al Ahly are champions!";
    if (teamName == "Zamalek") return "The White Knights rise again - Zamalek are champions!";
    if (teamName == "Pyramids FC") return "A new force in Egypt - Pyramids are champions!";
    if (teamName == "Future FC") return "Future arrives early - they are champions!";
    if (teamName == "Al Masry") return "Port Said celebrates - Al Masry are champions!";
    if (teamName == "Ismaily") return "The Dervishes return to glory - Ismaily are champions!";
    if (teamName == "ENPPI") return "A shock triumph - ENPPI are champions!";
    if (teamName == "Smouha") return "Alexandria celebrates - Smouha are champions!";
    if (teamName == "Ceramica Cleopatra") return "A historic run - Ceramica Cleopatra are champions!";
    if (teamName == "National Bank") return "A stunning upset - National Bank are champions!";
    if (teamName == "Pharco") return "Pharco rise to the top of Egypt!";
    if (teamName == "El Gouna") return "A Red Sea miracle - El Gouna are champions!";
    if (teamName == "Talaea El Gaish") return "The army stands tall - El Gaish are champions!";
    if (teamName == "Al Mokawloon") return "The Contractors build a title-winning season!";
    if (teamName == "Baladeyet El Mahalla") return "Mahalla celebrates - champions at last!";
    if (teamName == "ZED FC") return "ZED shock Egypt - they are champions!";
    if (teamName == "Modern Sport") return "Modern football, modern champions!";
    if (teamName == "Haras El Hodood") return "The guards return - Haras El Hodood are champions!";
    return teamName + " are crowned champions after a brilliant season!";
}

// -----------------------------------------
//  AUTO-LIST: 3 injuries OR 3 red cards
// -----------------------------------------
void checkAutoList(Team& team, Player& p) {
    if (p.listedForSale) return;

    if (p.totalInjuries >= 3 || p.totalRedCards >= 3) {
        p.listedForSale = true;
        p.askingPrice = max(1, p.marketValue * 80 / 100);
        cout << YELLOW;
        typeLine("  [AUTO-LIST] " + p.name + " (" + team.name + ") auto-listed for "
            + to_string(p.askingPrice) + "M (20% discount)!");
        cout << RESET;
    }
}

// -----------------------------------------
//  RESET PER-SEASON STATS
// -----------------------------------------
void resetSeasonStats() {
    for (auto& t : teams) {
        t.points = t.wins = t.draws = t.losses = 0;
        t.goalsScored = t.goalsConceded = 0;

        for (auto& p : t.players) {
            p.goals = p.assists = p.yellowCards = p.redCards = 0;
            p.suspended = false;
            p.injured = false;
            p.injuryGamesLeft = 0;
        }
    }

    leagueSchedule.clear();
    leagueFinishedThisSeason = false;
    cupFinishedThisSeason = false;
    backToMainMenu = false;
}

// -----------------------------------------
//  BUILD SQUAD
// -----------------------------------------
void buildSquad(Team& t, const vector<pair<string, string>>& squad) {
    int values[] = { 5,8,8,8,8,8,12,12,12,20,20,4,4,4,4,4 };

    for (int i = 0; i < (int)squad.size(); i++) {
        Player p;
        p.name = squad[i].first;
        p.position = squad[i].second;
        p.marketValue = (i < 16) ? values[i] : 4;
        p.isSubstitute = (i >= 11);
        t.players.push_back(p);
    }
}

// -----------------------------------------
//  PRESET LEAGUES
// -----------------------------------------
void loadPresetLeague(int choice) {
    teams.clear();

    auto add = [&](string nm, string st, string ci, int str, int bud,
        vector<pair<string, string>> sq) {
            Team t;
            t.name = nm;
            t.stadium = st;
            t.city = ci;
            t.strength = str;
            t.budget = bud;
            buildSquad(t, sq);
            teams.push_back(t);
        };

    if (choice == 1) {
        leagueName = "Premier League";
        add("Man City", "Etihad Stadium", "Manchester", 10, 210, {
            {"Ederson","GK"},{"Rico Lewis","DEF"},{"Ruben Dias","DEF"},{"Manuel Akanji","DEF"},
            {"Josko Gvardiol","DEF"},{"Rodri","MID"},{"Mateo Kovacic","MID"},{"Phil Foden","MID"},
            {"Savinho","STR"},{"Erling Haaland","STR"},{"Jeremy Doku","STR"},
            {"Stefan Ortega","GK"},{"John Stones","DEF"},{"Bernardo Silva","MID"},{"Oscar Bobb","MID"},{"Omar Marmoush","STR"} });

        add("Arsenal", "Emirates Stadium", "London", 9, 190, {
            {"David Raya","GK"},{"Ben White","DEF"},{"William Saliba","DEF"},{"Gabriel","DEF"},
            {"Jurrien Timber","DEF"},{"Declan Rice","MID"},{"Martin Odegaard","MID"},{"Mikel Merino","MID"},
            {"Bukayo Saka","STR"},{"Kai Havertz","STR"},{"Gabriel Martinelli","STR"},
            {"Neto","GK"},{"Riccardo Calafiori","DEF"},{"Leandro Trossard","STR"},{"Ethan Nwaneri","MID"},{"Gabriel Jesus","STR"} });

        add("Liverpool", "Anfield", "Liverpool", 9, 185, {
            {"Alisson","GK"},{"Conor Bradley","DEF"},{"Virgil van Dijk","DEF"},{"Ibrahima Konate","DEF"},
            {"Andrew Robertson","DEF"},{"Alexis Mac Allister","MID"},{"Ryan Gravenberch","MID"},{"Dominik Szoboszlai","MID"},
            {"Mohamed Salah","STR"},{"Darwin Nunez","STR"},{"Luis Diaz","STR"},
            {"Giorgi Mamardashvili","GK"},{"Joe Gomez","DEF"},{"Curtis Jones","MID"},{"Harvey Elliott","MID"},{"Diogo Jota","STR"} });

        add("Chelsea", "Stamford Bridge", "London", 8, 200, {
            {"Robert Sanchez","GK"},{"Reece James","DEF"},{"Levi Colwill","DEF"},{"Wesley Fofana","DEF"},
            {"Marc Cucurella","DEF"},{"Moises Caicedo","MID"},{"Enzo Fernandez","MID"},{"Cole Palmer","MID"},
            {"Noni Madueke","STR"},{"Nicolas Jackson","STR"},{"Pedro Neto","STR"},
            {"Filip Jorgensen","GK"},{"Benoit Badiashile","DEF"},{"Romeo Lavia","MID"},{"Christopher Nkunku","STR"},{"Jadon Sancho","STR"} });

        add("Aston Villa", "Villa Park", "Birmingham", 8, 155, {
            {"Emiliano Martinez","GK"},{"Matty Cash","DEF"},{"Ezri Konsa","DEF"},{"Pau Torres","DEF"},
            {"Lucas Digne","DEF"},{"Boubacar Kamara","MID"},{"Youri Tielemans","MID"},{"John McGinn","MID"},
            {"Morgan Rogers","STR"},{"Ollie Watkins","STR"},{"Leon Bailey","STR"},
            {"Robin Olsen","GK"},{"Diego Carlos","DEF"},{"Amadou Onana","MID"},{"Jacob Ramsey","MID"},{"Jhon Duran","STR"} });

        add("Newcastle", "St. James Park", "Newcastle", 8, 150, {
            {"Nick Pope","GK"},{"Tino Livramento","DEF"},{"Fabian Schar","DEF"},{"Sven Botman","DEF"},
            {"Lewis Hall","DEF"},{"Bruno Guimaraes","MID"},{"Sandro Tonali","MID"},{"Joelinton","MID"},
            {"Anthony Gordon","STR"},{"Alexander Isak","STR"},{"Harvey Barnes","STR"},
            {"Martin Dubravka","GK"},{"Dan Burn","DEF"},{"Sean Longstaff","MID"},{"Jacob Murphy","STR"},{"Callum Wilson","STR"} });

        add("Tottenham", "Tottenham Hotspur Stadium", "London", 7, 145, {
            {"Guglielmo Vicario","GK"},{"Pedro Porro","DEF"},{"Cristian Romero","DEF"},{"Micky van de Ven","DEF"},
            {"Destiny Udogie","DEF"},{"Yves Bissouma","MID"},{"Pape Sarr","MID"},{"James Maddison","MID"},
            {"Dejan Kulusevski","STR"},{"Dominic Solanke","STR"},{"Heung-min Son","STR"},
            {"Fraser Forster","GK"},{"Ben Davies","DEF"},{"Lucas Bergvall","MID"},{"Brennan Johnson","STR"},{"Wilson Odobert","STR"} });

        add("Man United", "Old Trafford", "Manchester", 7, 165, {
            {"Andre Onana","GK"},{"Noussair Mazraoui","DEF"},{"Lisandro Martinez","DEF"},{"Matthijs de Ligt","DEF"},
            {"Diogo Dalot","DEF"},{"Kobbie Mainoo","MID"},{"Manuel Ugarte","MID"},{"Bruno Fernandes","MID"},
            {"Amad Diallo","STR"},{"Rasmus Hojlund","STR"},{"Alejandro Garnacho","STR"},
            {"Altay Bayindir","GK"},{"Harry Maguire","DEF"},{"Mason Mount","MID"},{"Joshua Zirkzee","STR"},{"Marcus Rashford","STR"} });

        add("West Ham", "London Stadium", "London", 6, 115, {
            {"Alphonse Areola","GK"},{"Aaron Wan-Bissaka","DEF"},{"Max Kilman","DEF"},{"Jean-Clair Todibo","DEF"},
            {"Emerson Palmieri","DEF"},{"Tomas Soucek","MID"},{"Edson Alvarez","MID"},{"Lucas Paqueta","MID"},
            {"Jarrod Bowen","STR"},{"Niclas Fullkrug","STR"},{"Mohammed Kudus","STR"},
            {"Lukasz Fabianski","GK"},{"Vladimir Coufal","DEF"},{"James Ward-Prowse","MID"},{"Carlos Soler","MID"},{"Crysencio Summerville","STR"} });

        add("Brighton", "Amex Stadium", "Brighton", 7, 125, {
            {"Bart Verbruggen","GK"},{"Joel Veltman","DEF"},{"Lewis Dunk","DEF"},{"Jan Paul van Hecke","DEF"},
            {"Pervis Estupinan","DEF"},{"Carlos Baleba","MID"},{"Jack Hinshelwood","MID"},{"Matt ORiley","MID"},
            {"Kaoru Mitoma","STR"},{"Joao Pedro","STR"},{"Georginio Rutter","STR"},
            {"Jason Steele","GK"},{"Igor","DEF"},{"Diego Gomez","MID"},{"Solly March","STR"},{"Simon Adingra","STR"} });
    }
    else if (choice == 2) {
        leagueName = "La Liga";
        add("Real Madrid", "Santiago Bernabeu", "Madrid", 10, 260, {
            {"Thibaut Courtois","GK"},{"Dani Carvajal","DEF"},{"Eder Militao","DEF"},{"Antonio Rudiger","DEF"},
            {"Ferland Mendy","DEF"},{"Aurelien Tchouameni","MID"},{"Federico Valverde","MID"},{"Jude Bellingham","MID"},
            {"Vinicius Junior","STR"},{"Kylian Mbappe","STR"},{"Rodrygo","STR"},
            {"Andriy Lunin","GK"},{"David Alaba","DEF"},{"Eduardo Camavinga","MID"},{"Brahim Diaz","MID"},{"Endrick","STR"} });

        add("Barcelona", "Estadi Olimpic Lluis Companys", "Barcelona", 9, 210, {
            {"Wojciech Szczesny","GK"},{"Jules Kounde","DEF"},{"Ronald Araujo","DEF"},{"Pau Cubarsi","DEF"},
            {"Alejandro Balde","DEF"},{"Pedri","MID"},{"Frenkie de Jong","MID"},{"Dani Olmo","MID"},
            {"Lamine Yamal","STR"},{"Robert Lewandowski","STR"},{"Raphinha","STR"},
            {"Inaki Pena","GK"},{"Eric Garcia","DEF"},{"Gavi","MID"},{"Marc Casado","MID"},{"Ferran Torres","STR"} });

        add("Atletico Madrid", "Civitas Metropolitano", "Madrid", 9, 175, {
            {"Jan Oblak","GK"},{"Nahuel Molina","DEF"},{"Jose Gimenez","DEF"},{"Robin Le Normand","DEF"},
            {"Javi Galan","DEF"},{"Koke","MID"},{"Rodrigo De Paul","MID"},{"Marcos Llorente","MID"},
            {"Antoine Griezmann","STR"},{"Julian Alvarez","STR"},{"Alexander Sorloth","STR"},
            {"Juan Musso","GK"},{"Cesar Azpilicueta","DEF"},{"Conor Gallagher","MID"},{"Pablo Barrios","MID"},{"Samuel Lino","STR"} });

        add("Real Sociedad", "Reale Arena", "San Sebastian", 7, 105, {
            {"Alex Remiro","GK"},{"Hamari Traore","DEF"},{"Igor Zubeldia","DEF"},{"Aritz Elustondo","DEF"},
            {"Aihen Munoz","DEF"},{"Martin Zubimendi","MID"},{"Brais Mendez","MID"},{"Mikel Merino","MID"},
            {"Takefusa Kubo","STR"},{"Mikel Oyarzabal","STR"},{"Ander Barrenetxea","STR"},
            {"Unai Marrero","GK"},{"Jon Aramburu","DEF"},{"Benat Turrientes","MID"},{"Sergio Gomez","MID"},{"Sheraldo Becker","STR"} });

        add("Athletic Bilbao", "San Mames", "Bilbao", 7, 95, {
            {"Unai Simon","GK"},{"Oscar De Marcos","DEF"},{"Dani Vivian","DEF"},{"Yeray Alvarez","DEF"},
            {"Yuri Berchiche","DEF"},{"Mikel Jauregizar","MID"},{"Oihan Sancet","MID"},{"Ander Herrera","MID"},
            {"Inaki Williams","STR"},{"Gorka Guruzeta","STR"},{"Nico Williams","STR"},
            {"Julen Agirrezabala","GK"},{"Aitor Paredes","DEF"},{"Unai Gomez","MID"},{"Inigo Ruiz de Galarreta","MID"},{"Alex Berenguer","STR"} });

        add("Villarreal", "Estadio de la Ceramica", "Villarreal", 7, 100, {
            {"Luiz Junior","GK"},{"Kiko Femenia","DEF"},{"Logan Costa","DEF"},{"Willy Kambwala","DEF"},
            {"Sergi Cardona","DEF"},{"Dani Parejo","MID"},{"Santi Comesana","MID"},{"Alex Baena","MID"},
            {"Yeremy Pino","STR"},{"Ayoze Perez","STR"},{"Gerard Moreno","STR"},
            {"Diego Conde","GK"},{"Juan Foyth","DEF"},{"Pape Gueye","MID"},{"Nicolas Pepe","STR"},{"Thierno Barry","STR"} });

        add("Real Betis", "Estadio Benito Villamarin", "Seville", 7, 90, {
            {"Rui Silva","GK"},{"Hector Bellerin","DEF"},{"Diego Llorente","DEF"},{"Marc Bartra","DEF"},
            {"Ricardo Rodriguez","DEF"},{"Johnny Cardoso","MID"},{"Giovani Lo Celso","MID"},{"Isco","MID"},
            {"Antony","STR"},{"Vitor Roque","STR"},{"Ayoze Perez","STR"},
            {"Fran Vieites","GK"},{"Natan","DEF"},{"Sergi Altimira","MID"},{"Abde Ezzalzouli","STR"},{"Assane Diao","STR"} });

        add("Sevilla", "Ramon Sanchez-Pizjuan", "Seville", 6, 80, {
            {"Orjan Nyland","GK"},{"Jose Angel Carmona","DEF"},{"Loic Bade","DEF"},{"Tanguy Nianzou","DEF"},
            {"Adria Pedrosa","DEF"},{"Djibril Sow","MID"},{"Lucien Agoume","MID"},{"Suso","MID"},
            {"Juanlu Sanchez","STR"},{"Isaac Romero","STR"},{"Dodi Lukebakio","STR"},
            {"Alberto Flores","GK"},{"Marcao","DEF"},{"Saul Niguez","MID"},{"Chidera Ejuke","STR"},{"Peque Fernandez","STR"} });

        add("Getafe", "Coliseum", "Getafe", 5, 55, {
            {"David Soria","GK"},{"Djene","DEF"},{"Domingos Duarte","DEF"},{"Omar Alderete","DEF"},
            {"Diego Rico","DEF"},{"Mauro Arambarri","MID"},{"Luis Milla","MID"},{"Carles Alena","MID"},
            {"Bertug Yildirim","STR"},{"Borja Mayoral","STR"},{"Chrisantus Uche","STR"},
            {"Jiri Letacek","GK"},{"Juan Iglesias","DEF"},{"Nemanja Maksimovic","MID"},{"Peter Gonzalez","STR"},{"Juanmi","STR"} });

        add("Osasuna", "El Sadar", "Pamplona", 5, 50, {
            {"Sergio Herrera","GK"},{"Jesus Areso","DEF"},{"David Garcia","DEF"},{"Jorge Herrando","DEF"},
            {"Juan Cruz","DEF"},{"Lucas Torro","MID"},{"Jon Moncayola","MID"},{"Aimar Oroz","MID"},
            {"Ruben Garcia","STR"},{"Ante Budimir","STR"},{"Bryan Zaragoza","STR"},
            {"Aitor Fernandez","GK"},{"Alejandro Catena","DEF"},{"Moi Gomez","MID"},{"Kike Barja","STR"},{"Raul Garcia","STR"} });
    }
    else if (choice == 3) {
        leagueName = "Serie A";
        add("Inter Milan", "San Siro", "Milan", 10, 195, {
            {"Yann Sommer","GK"},{"Benjamin Pavard","DEF"},{"Francesco Acerbi","DEF"},{"Alessandro Bastoni","DEF"},
            {"Federico Dimarco","DEF"},{"Nicolo Barella","MID"},{"Hakan Calhanoglu","MID"},{"Henrikh Mkhitaryan","MID"},
            {"Denzel Dumfries","MID"},{"Lautaro Martinez","STR"},{"Marcus Thuram","STR"},
            {"Josep Martinez","GK"},{"Yann Bisseck","DEF"},{"Carlos Augusto","DEF"},{"Davide Frattesi","MID"},{"Mehdi Taremi","STR"} });

        add("Napoli", "Stadio Maradona", "Naples", 9, 165, {
            {"Alex Meret","GK"},{"Giovanni Di Lorenzo","DEF"},{"Amir Rrahmani","DEF"},{"Alessandro Buongiorno","DEF"},
            {"Mathias Olivera","DEF"},{"Stanislav Lobotka","MID"},{"Scott McTominay","MID"},{"Andre-Frank Zambo Anguissa","MID"},
            {"Khvicha Kvaratskhelia","STR"},{"Romelu Lukaku","STR"},{"David Neres","STR"},
            {"Elia Caprile","GK"},{"Juan Jesus","DEF"},{"Billy Gilmour","MID"},{"Giacomo Raspadori","STR"},{"Matteo Politano","STR"} });

        add("AC Milan", "San Siro", "Milan", 9, 175, {
            {"Mike Maignan","GK"},{"Emerson Royal","DEF"},{"Malick Thiaw","DEF"},{"Fikayo Tomori","DEF"},
            {"Theo Hernandez","DEF"},{"Tijjani Reijnders","MID"},{"Youssouf Fofana","MID"},{"Ruben Loftus-Cheek","MID"},
            {"Christian Pulisic","STR"},{"Alvaro Morata","STR"},{"Rafael Leao","STR"},
            {"Marco Sportiello","GK"},{"Strahinja Pavlovic","DEF"},{"Ismael Bennacer","MID"},{"Samuel Chukwueze","STR"},{"Noah Okafor","STR"} });

        add("Juventus", "Allianz Stadium", "Turin", 8, 155, {
            {"Michele Di Gregorio","GK"},{"Andrea Cambiaso","DEF"},{"Gleison Bremer","DEF"},{"Federico Gatti","DEF"},
            {"Juan Cabal","DEF"},{"Manuel Locatelli","MID"},{"Khephren Thuram","MID"},{"Douglas Luiz","MID"},
            {"Kenan Yildiz","STR"},{"Dusan Vlahovic","STR"},{"Timothy Weah","STR"},
            {"Carlo Pinsoglio","GK"},{"Daniele Rugani","DEF"},{"Nicolo Fagioli","MID"},{"Weston McKennie","MID"},{"Arkadiusz Milik","STR"} });

        add("Atalanta", "Gewiss Stadium", "Bergamo", 8, 135, {
            {"Marco Carnesecchi","GK"},{"Rafael Toloi","DEF"},{"Berat Djimsiti","DEF"},{"Giorgio Scalvini","DEF"},
            {"Davide Zappacosta","DEF"},{"Marten de Roon","MID"},{"Ederson","MID"},{"Mario Pasalic","MID"},
            {"Ademola Lookman","STR"},{"Mateo Retegui","STR"},{"Charles De Ketelaere","STR"},
            {"Francesco Rossi","GK"},{"Sead Kolasinac","DEF"},{"Teun Koopmeiners","MID"},{"Nicolo Zaniolo","STR"},{"Gianluca Scamacca","STR"} });

        add("AS Roma", "Stadio Olimpico", "Rome", 7, 125, {
            {"Mile Svilar","GK"},{"Zeki Celik","DEF"},{"Gianluca Mancini","DEF"},{"Evan Ndicka","DEF"},
            {"Angelino","DEF"},{"Bryan Cristante","MID"},{"Lorenzo Pellegrini","MID"},{"Leandro Paredes","MID"},
            {"Paulo Dybala","STR"},{"Artem Dovbyk","STR"},{"Matias Soule","STR"},
            {"Mathew Ryan","GK"},{"Mario Hermoso","DEF"},{"Enzo Le Fee","MID"},{"Tommaso Baldanzi","MID"},{"Stephan El Shaarawy","STR"} });

        add("Lazio", "Stadio Olimpico", "Rome", 7, 115, {
            {"Ivan Provedel","GK"},{"Elseid Hysaj","DEF"},{"Patric","DEF"},{"Alessio Romagnoli","DEF"},
            {"Nuno Tavares","DEF"},{"Matteo Guendouzi","MID"},{"Danilo Cataldi","MID"},{"Nicolo Rovella","MID"},
            {"Mattia Zaccagni","STR"},{"Valentin Castellanos","STR"},{"Gustav Isaksen","STR"},
            {"Christos Mandas","GK"},{"Mario Gila","DEF"},{"Adam Marusic","DEF"},{"Luca Pellegrini","DEF"},{"Boulaye Dia","STR"} });

        add("Fiorentina", "Stadio Artemio Franchi", "Florence", 6, 85, {
            {"David de Gea","GK"},{"Dodo","DEF"},{"Lucas Martinez Quarta","DEF"},{"Nikola Milenkovic","DEF"},
            {"Cristiano Biraghi","DEF"},{"Rolando Mandragora","MID"},{"Giacomo Bonaventura","MID"},{"Yacine Adli","MID"},
            {"Riccardo Sottil","STR"},{"Albert Gudmundsson","STR"},{"Moise Kean","STR"},
            {"Pietro Terracciano","GK"},{"Michael Kayode","DEF"},{"Amir Richardson","MID"},{"Jonathan Ikone","STR"},{"Lucas Beltran","STR"} });

        add("Torino", "Stadio Olimpico Grande Torino", "Turin", 5, 65, {
            {"Vanja Milinkovic-Savic","GK"},{"Mergim Vojvoda","DEF"},{"Saul Coco","DEF"},{"Alessandro Buongiorno","DEF"},
            {"Valentino Lazaro","DEF"},{"Samuele Ricci","MID"},{"Ivan Ilic","MID"},{"Karol Linetty","MID"},
            {"Duvan Zapata","STR"},{"Antonio Sanabria","STR"},{"Nemanja Radonjic","STR"},
            {"Luca Gemello","GK"},{"Adam Masina","DEF"},{"Adrien Tameze","MID"},{"Nikola Vlasic","STR"},{"Che Adams","STR"} });

        add("Bologna", "Stadio Renato Dall'Ara", "Bologna", 6, 85, {
            {"Lukasz Skorupski","GK"},{"Stefan Posch","DEF"},{"Sam Beukema","DEF"},{"Jhon Lucumi","DEF"},
            {"Charalampos Lykogiannis","DEF"},{"Remo Freuler","MID"},{"Michel Aebischer","MID"},{"Giovanni Fabbian","MID"},
            {"Dan Ndoye","STR"},{"Santiago Castro","STR"},{"Riccardo Orsolini","STR"},
            {"Federico Ravaglia","GK"},{"Martin Erlic","DEF"},{"Lewis Ferguson","MID"},{"Jesper Karlsson","STR"},{"Thijs Dallinga","STR"} });
    }
    else if (choice == 4) {
        leagueName = "Ligue 1";
        add("PSG", "Parc des Princes", "Paris", 10, 300, {
            {"Lucas Chevalier","GK"},{"Achraf Hakimi","DEF"},{"Marquinhos","DEF"},{"Willian Pacho","DEF"},
            {"Nuno Mendes","DEF"},{"Vitinha","MID"},{"Warren Zaire Emery","MID"},{"Fabian Ruiz","MID"},
            {"Desire Doue","STR"},{"Goncalo Ramos","STR"},{"Ousmane Dembele","STR"},
            {"Matvey Safonov","GK"},{"Lucas Beraldo","DEF"},{"Lee Kang-in","MID"},{"Bradley Barcola","STR"},{"Randal Kolo Muani","STR"} });

        add("Monaco", "Stade Louis II", "Monaco", 8, 135, {
            {"Philipp Kohn","GK"},{"Vanderson","DEF"},{"Thilo Kehrer","DEF"},{"Wilfried Singo","DEF"},
            {"Caio Henrique","DEF"},{"Lamine Camara","MID"},{"Denis Zakaria","MID"},{"Aleksandr Golovin","MID"},
            {"Maghnes Akliouche","STR"},{"Folarin Balogun","STR"},{"Takumi Minamino","STR"},
            {"Radoslaw Majecki","GK"},{"Christian Mawissa","DEF"},{"Mohamed Camara","MID"},{"Eliesse Ben Seghir","MID"},{"George Ilenikhena","STR"} });

        add("Marseille", "Orange Velodrome", "Marseille", 8, 125, {
            {"Geronimo Rulli","GK"},{"Pol Lirola","DEF"},{"Leonardo Balerdi","DEF"},{"Chancel Mbemba","DEF"},
            {"Quentin Merlin","DEF"},{"Geoffrey Kondogbia","MID"},{"Jordan Veretout","MID"},{"Valentin Rongier","MID"},
            {"Mason Greenwood","STR"},{"Elye Wahi","STR"},{"Amine Harit","STR"},
            {"Ruben Blanco","GK"},{"Jonathan Clauss","DEF"},{"Ismaila Sarr","STR"},{"Faris Moumbagna","STR"},{"Neal Maupay","STR"} });

        add("Lille", "Stade Pierre-Mauroy", "Lille", 7, 95, {
            {"Lucas Chevalier","GK"},{"Thomas Meunier","DEF"},{"Bafode Diakite","DEF"},{"Alexsandro","DEF"},
            {"Gabriel Gudmundsson","DEF"},{"Benjamin Andre","MID"},{"Ngalayel Mukau","MID"},{"Angel Gomes","MID"},
            {"Edon Zhegrova","STR"},{"Jonathan David","STR"},{"Hakon Haraldsson","STR"},
            {"Vito Mannone","GK"},{"Tiago Santos","DEF"},{"Ayyoub Bouaddi","MID"},{"Osame Sahraoui","STR"},{"Mohamed Bayo","STR"} });

        add("Lyon", "Groupama Stadium", "Lyon", 7, 105, {
            {"Lucas Perri","GK"},{"Sael Kumbedi","DEF"},{"Duje Caleta-Car","DEF"},{"Jake OBrien","DEF"},
            {"Nicolas Tagliafico","DEF"},{"Corentin Tolisso","MID"},{"Maxence Caqueret","MID"},{"Nemanja Matic","MID"},
            {"Rayan Cherki","STR"},{"Alexandre Lacazette","STR"},{"Ernest Nuamah","STR"},
            {"Anthony Lopes","GK"},{"Clinton Mata","DEF"},{"Johann Lepenant","MID"},{"Ainsley Maitland-Niles","MID"},{"Gift Orban","STR"} });

        add("Lens", "Stade Bollaert-Delelis", "Lens", 7, 90, {
            {"Brice Samba","GK"},{"Jonathan Gradit","DEF"},{"Kevin Danso","DEF"},{"Facundo Medina","DEF"},
            {"Przemyslaw Frankowski","DEF"},{"Salis Abdul Samed","MID"},{"Andy Diouf","MID"},{"Adrien Thomasson","MID"},
            {"Florian Sotoca","STR"},{"MBala Nzola","STR"},{"David Pereira Da Costa","STR"},
            {"Mathew Ryan","GK"},{"Deiver Machado","DEF"},{"Neil El Aynaoui","MID"},{"Anass Zaroury","STR"},{"Wesley Said","STR"} });

        add("Rennes", "Roazhon Park", "Rennes", 6, 80, {
            {"Steve Mandanda","GK"},{"Lorenz Assignon","DEF"},{"Arthur Theate","DEF"},{"Christopher Wooh","DEF"},
            {"Adrien Truffert","DEF"},{"Seko Fofana","MID"},{"Benjamin Bourigeaud","MID"},{"Ludovic Blas","MID"},
            {"Mahamadou Nagida","STR"},{"Arnaud Kalimuendo","STR"},{"Amine Gouiri","STR"},
            {"Gauthier Gallon","GK"},{"Birger Meling","DEF"},{"Azor Matusiwa","MID"},{"Desire Doue","MID"},{"Martin Terrier","STR"} });

        add("Nice", "Allianz Riviera", "Nice", 6, 85, {
            {"Marcin Bulka","GK"},{"Jordan Lotomba","DEF"},{"Jean-Clair Todibo","DEF"},{"Dante","DEF"},
            {"Melvin Bard","DEF"},{"Hicham Boudaoui","MID"},{"Pablo Rosario","MID"},{"Morgan Sanson","MID"},
            {"Jeremie Boga","STR"},{"Terem Moffi","STR"},{"Mohamed-Ali Cho","STR"},
            {"Maxime Dupe","GK"},{"Youcef Atal","DEF"},{"Khephren Thuram","MID"},{"Evann Guessand","STR"},{"Gaetan Laborde","STR"} });

        add("Strasbourg", "Stade de la Meinau", "Strasbourg", 5, 55, {
            {"Karl-Johan Johnsson","GK"},{"Marvin Senaya","DEF"},{"Abakar Sylla","DEF"},{"Lucas Perrin","DEF"},
            {"Thomas Delaine","DEF"},{"Andrey Santos","MID"},{"Habib Diarra","MID"},{"Dion Sahi","MID"},
            {"Dilane Bakwa","STR"},{"Emanuel Emegha","STR"},{"Kevin Gameiro","STR"},
            {"Alaa Bellaarouch","GK"},{"Ismael Doukoure","DEF"},{"Junior Mwanga","MID"},{"Jessy Deminguet","MID"},{"Moise Sahi Dion","STR"} });

        add("Nantes", "Stade de la Beaujoire", "Nantes", 5, 50, {
            {"Alban Lafont","GK"},{"Kelvin Amian","DEF"},{"Jean-Charles Castelletto","DEF"},{"Nathan Zeze","DEF"},
            {"Nicolas Cozza","DEF"},{"Pedro Chirivella","MID"},{"Moussa Sissoko","MID"},{"Douglas Augusto","MID"},
            {"Moses Simon","STR"},{"Mostafa Mohamed","STR"},{"Matthis Abline","STR"},
            {"Remy Descamps","GK"},{"Marcus Coco","DEF"},{"Florent Mollet","MID"},{"Tino Kadewere","STR"},{"Beni Traore","STR"} });
    }
    else if (choice == 5) {
        leagueName = "Bundesliga";
        add("Bayern Munich", "Allianz Arena", "Munich", 10, 225, {
            {"Manuel Neuer","GK"},{"Konrad Laimer","DEF"},{"Dayot Upamecano","DEF"},{"Kim Min-jae","DEF"},
            {"Alphonso Davies","DEF"},{"Joshua Kimmich","MID"},{"Aleksandar Pavlovic","MID"},{"Jamal Musiala","MID"},
            {"Michael Olise","STR"},{"Harry Kane","STR"},{"Serge Gnabry","STR"},
            {"Sven Ulreich","GK"},{"Raphael Guerreiro","DEF"},{"Leon Goretzka","MID"},{"Thomas Muller","MID"},{"Mathys Tel","STR"} });

        add("Bayer Leverkusen", "BayArena", "Leverkusen", 9, 165, {
            {"Lukas Hradecky","GK"},{"Jeremie Frimpong","DEF"},{"Edmond Tapsoba","DEF"},{"Jonathan Tah","DEF"},
            {"Alejandro Grimaldo","DEF"},{"Granit Xhaka","MID"},{"Robert Andrich","MID"},{"Florian Wirtz","MID"},
            {"Jonas Hofmann","STR"},{"Victor Boniface","STR"},{"Patrik Schick","STR"},
            {"Matej Kovar","GK"},{"Piero Hincapie","DEF"},{"Exequiel Palacios","MID"},{"Amine Adli","MID"},{"Martin Terrier","STR"} });

        add("Borussia Dortmund", "Signal Iduna Park", "Dortmund", 8, 155, {
            {"Gregor Kobel","GK"},{"Julian Ryerson","DEF"},{"Niklas Sule","DEF"},{"Nico Schlotterbeck","DEF"},
            {"Ramy Bensebaini","DEF"},{"Emre Can","MID"},{"Marcel Sabitzer","MID"},{"Julian Brandt","MID"},
            {"Karim Adeyemi","STR"},{"Serhou Guirassy","STR"},{"Jamie Gittens","STR"},
            {"Alexander Meyer","GK"},{"Waldemar Anton","DEF"},{"Pascal Gross","MID"},{"Felix Nmecha","MID"},{"Maximilian Beier","STR"} });

        add("RB Leipzig", "Red Bull Arena", "Leipzig", 8, 145, {
            {"Peter Gulacsi","GK"},{"Benjamin Henrichs","DEF"},{"Willi Orban","DEF"},{"Castello Lukeba","DEF"},
            {"David Raum","DEF"},{"Kevin Kampl","MID"},{"Amadou Haidara","MID"},{"Xaver Schlager","MID"},
            {"Antonio Nusa","STR"},{"Lois Openda","STR"},{"Benjamin Sesko","STR"},
            {"Janis Blaswich","GK"},{"Mohamed Simakan","DEF"},{"Nicolas Seiwald","MID"},{"Christoph Baumgartner","MID"},{"Xavi Simons","STR"} });

        add("Eintracht Frankfurt", "Deutsche Bank Park", "Frankfurt", 7, 105, {
            {"Kevin Trapp","GK"},{"Tuta","DEF"},{"Robin Koch","DEF"},{"Arthur Theate","DEF"},
            {"Nathaniel Brown","DEF"},{"Ellyes Skhiri","MID"},{"Hugo Larsson","MID"},{"Mario Gotze","MID"},
            {"Ansgar Knauff","STR"},{"Omar Marmoush","STR"},{"Igor Matanovic","STR"},
            {"Kaua Santos","GK"},{"Rasmus Kristensen","DEF"},{"Can Uzun","MID"},{"Fares Chaibi","MID"},{"Junior Dina Ebimbe","MID"} });

        add("Freiburg", "Europa-Park Stadion", "Freiburg", 6, 75, {
            {"Noah Atubolu","GK"},{"Lukas Kubler","DEF"},{"Matthias Ginter","DEF"},{"Philipp Lienhart","DEF"},
            {"Christian Gunter","DEF"},{"Nicolas Hofler","MID"},{"Maximilian Eggestein","MID"},{"Merlin Rohl","MID"},
            {"Ritsu Doan","STR"},{"Lucas Holer","STR"},{"Vincenzo Grifo","STR"},
            {"Florian Muller","GK"},{"Manuel Gulde","DEF"},{"Yannik Keitel","MID"},{"Michael Gregoritsch","STR"},{"Junior Adamu","STR"} });

        add("Union Berlin", "An der Alten Forsterei", "Berlin", 6, 70, {
            {"Frederik Ronnow","GK"},{"Josip Juranovic","DEF"},{"Danilho Doekhi","DEF"},{"Robin Knoche","DEF"},
            {"Robin Gosens","DEF"},{"Andras Schafer","MID"},{"Alex Kral","MID"},{"Janik Haberer","MID"},
            {"Benedict Hollerbach","STR"},{"Kevin Volland","STR"},{"Sheraldo Becker","STR"},
            {"Lennart Grill","GK"},{"Paul Jaeckel","DEF"},{"Rani Khedira","MID"},{"Morten Thorsby","MID"},{"Jordan Siebatcheu","STR"} });

        add("Wolfsburg", "Volkswagen Arena", "Wolfsburg", 6, 75, {
            {"Koen Casteels","GK"},{"Ridle Baku","DEF"},{"Sebastiaan Bornauw","DEF"},{"Maxence Lacroix","DEF"},
            {"Rogerio","DEF"},{"Maximilian Arnold","MID"},{"Yannick Gerhardt","MID"},{"Lovro Majer","MID"},
            {"Patrick Wimmer","STR"},{"Jonas Wind","STR"},{"Mohamed Amoura","STR"},
            {"Niklas Klinger","GK"},{"Cedric Zesiger","DEF"},{"Mattias Svanberg","MID"},{"Jakub Kaminski","STR"},{"Tiago Tomas","STR"} });

        add("Borussia Mgladbach", "Borussia-Park", "Monchengladbach", 5, 65, {
            {"Jonas Omlin","GK"},{"Stefan Lainer","DEF"},{"Nico Elvedi","DEF"},{"Marvin Friedrich","DEF"},
            {"Luca Netz","DEF"},{"Julian Weigl","MID"},{"Florian Neuhaus","MID"},{"Kouadio Kone","MID"},
            {"Franck Honorat","STR"},{"Tomas Cvancara","STR"},{"Tim Kleindienst","STR"},
            {"Moritz Nicolas","GK"},{"Fabio Chiarodia","DEF"},{"Christoph Kramer","MID"},{"Alassane Plea","STR"},{"Nathan Ngoumou","STR"} });

        add("Mainz", "MEWA Arena", "Mainz", 5, 55, {
            {"Robin Zentner","GK"},{"Silvan Widmer","DEF"},{"Andreas Hanche-Olsen","DEF"},{"Dominik Kohr","DEF"},
            {"Anthony Caci","DEF"},{"Nadiem Amiri","MID"},{"Leandro Barreiro","MID"},{"Kaishu Sano","MID"},
            {"Jae-Sung Lee","STR"},{"Jonathan Burkardt","STR"},{"Brajan Gruda","STR"},
            {"Finn Dahmen","GK"},{"Stefan Bell","DEF"},{"Tom Krauss","MID"},{"Karim Onisiwo","STR"},{"Armindo Sieb","STR"} });
    }
    else if (choice == 6) {
        leagueName = "Egyptian Premier League";

        add("Al Ahly", "Cairo International Stadium", "Cairo", 10, 180, {
            {"Mohamed El Shenawy","GK"},{"Mohamed Hany","DEF"},{"Yasser Ibrahim","DEF"},{"Ramy Rabia","DEF"},
            {"Ali Maaloul","DEF"},{"Marwan Attia","MID"},{"Emam Ashour","MID"},{"Amr El Solia","MID"},
            {"Hussein El Shahat","STR"},{"Wessam Abou Ali","STR"},{"Percy Tau","STR"},
            {"Mostafa Shobeir","GK"},{"Karim Fouad","DEF"},{"Akram Tawfik","MID"},{"Afsha","MID"},{"Kahraba","STR"} });

        add("Zamalek", "Cairo International Stadium", "Cairo", 9, 160, {
            {"Mohamed Awad","GK"},{"Omar Gaber","DEF"},{"Mahmoud El Wensh","DEF"},{"Hossam Abdelmaguid","DEF"},
            {"Ahmed Fatouh","DEF"},{"Nabil Dunga","MID"},{"Mohamed Shehata","MID"},{"Abdallah El Said","MID"},
            {"Ahmed Zizo","STR"},{"Seif El Jaziri","STR"},{"Mostafa Shalaby","STR"},
            {"Mohamed Sobhy","GK"},{"Hamza Mathlouthi","DEF"},{"Youssef Obama","MID"},{"Shikabala","MID"},{"Nasser Mansi","STR"} });

        add("Pyramids FC", "30 June Stadium", "Cairo", 9, 170, {
            {"Ahmed El Shenawy","GK"},{"Mohamed Hamdy","DEF"},{"Ahmed Samy","DEF"},{"Osama Galal","DEF"},
            {"Karim Hafez","DEF"},{"Blati Toure","MID"},{"Walid El Karti","MID"},{"Ramadan Sobhi","MID"},
            {"Mostafa Fathi","STR"},{"Fiston Mayele","STR"},{"Ibrahim Adel","STR"},
            {"Sherif Ekramy","GK"},{"Mahmoud Marei","DEF"},{"Mahmoud Saber","MID"},{"Youssef Obama","MID"},{"Diego Rolan","STR"} });

        add("Al Masry", "Borg El Arab Stadium", "Port Said", 7, 90, {
            {"Mahmoud Gad","GK"},{"Karim El Eraki","DEF"},{"Islam Abou Slima","DEF"},{"Baher El Mohamady","DEF"},
            {"Amr Moussa","DEF"},{"Hassan Ali","MID"},{"Mido Gaber","MID"},{"Ilyas El Jlassi","MID"},
            {"Abderrahim Deghmoum","STR"},{"Ben Youssef","STR"},{"Ahmed Atef","STR"},
            {"Essam Tharwat","GK"},{"Emad Fathy","DEF"},{"Ahmed Hamdy","MID"},{"Ahmed El Sheikh","MID"},{"Mohamed El Shamy","STR"} });

        add("Future FC", "Al Salam Stadium", "Cairo", 7, 85, {
            {"Mahmoud Gennesh","GK"},{"Omar Kamal","DEF"},{"Saad Samir","DEF"},{"Mahmoud Rizk","DEF"},
            {"Mahmoud Shaaban","DEF"},{"Ghanem Mohamed","MID"},{"Karim Nedved","MID"},{"Mohamed Farouk","MID"},
            {"Ahmed Atef","STR"},{"Marwan Mohsen","STR"},{"Abdelkabir El Ouadi","STR"},
            {"Ahmed Yehia","GK"},{"Bassem Ali","DEF"},{"Ali Zaza","MID"},{"Nasser Maher","MID"},{"Hossam Hassan","STR"} });

        add("Ceramica Cleopatra", "Arab Contractors Stadium", "Cairo", 7, 80, {
            {"Amer Amer","GK"},{"Ahmed Hany","DEF"},{"Ragab Nabil","DEF"},{"Ahmed Ramadan","DEF"},
            {"Mohamed Shokry","DEF"},{"Mohamed Toni","MID"},{"Mohamed Adel","MID"},{"Ahmed Kendouci","MID"},
            {"Salah Mohsen","STR"},{"John Ebuka","STR"},{"Ahmed Belhadji","STR"},
            {"Mohamed Bassam","GK"},{"Saad Aglan","DEF"},{"Mohamed Ibrahim","MID"},{"Mahmoud Zalaka","MID"},{"Ammar Hamdy","STR"} });

        add("Smouha", "Alexandria Stadium", "Alexandria", 6, 70, {
            {"El Hany Soliman","GK"},{"Ahmed Bekhit","DEF"},{"Mahmoud Ezzat","DEF"},{"Ahmed Hakam","DEF"},
            {"Sherif Reda","DEF"},{"Mohamed Kenaria","MID"},{"Hussein Faisal","MID"},{"Dodo El Gabbas","MID"},
            {"Hossam Hassan","STR"},{"Fadi Farid","STR"},{"Mostafa Messi","STR"},
            {"Omar Salah","GK"},{"Ahmed Gamal","DEF"},{"Islam Gaber","MID"},{"Shady Hussein","MID"},{"Abdelkabir El Ouadi","STR"} });

        add("ENPPI", "Petro Sport Stadium", "Cairo", 6, 65, {
            {"El Balouty","GK"},{"Ali Fawzi","DEF"},{"Khaled Ahmed","DEF"},{"Ahmed Sabeha","DEF"},
            {"Mohamed Hamdi","DEF"},{"Kabaka","MID"},{"Dowidar","MID"},{"Oufa","MID"},
            {"Rafik Kabou","STR"},{"Ahmed Yasser Rayan","STR"},{"Mostafa Shalaby","STR"},
            {"Mahmoud Gad","GK"},{"Mohamed Hamed","DEF"},{"Ahmed El Agouz","MID"},{"Ali Ehab","MID"},{"Ziad Kamal","STR"} });

        add("Ismaily", "Ismailia Stadium", "Ismailia", 6, 60, {
            {"Ahmed Adel","GK"},{"Bassem Ali","DEF"},{"Mohamed Nasr","DEF"},{"Baher El Mohamady","DEF"},
            {"Abdelkarim Mostafa","DEF"},{"Amr El Saharti","MID"},{"Mohamed Hassan","MID"},{"Abdelrahman Magdy","MID"},
            {"Yaou Anwar","STR"},{"Mohamed El Shamy","STR"},{"Ahmed El Sheikh","STR"},
            {"Kamal El Sayed","GK"},{"Mohamed Desouky","DEF"},{"Mohamed Bayoumi","MID"},{"Omar El Saaiy","MID"},{"Hazem Morsi","STR"} });

        add("Al Ittihad", "Alexandria Stadium", "Alexandria", 6, 60, {
            {"El Mahdy Soliman","GK"},{"Hesham Salah","DEF"},{"Mahmoud Alaa","DEF"},{"Islam Abou Slima","DEF"},
            {"Sabri Rahil","DEF"},{"Nasser Nasser","MID"},{"Khaled El Ghandour","MID"},{"Ahmed Adel","MID"},
            {"Maboulou","STR"},{"Benjamin Boateng","STR"},{"Ibrahim Hassan","STR"},
            {"Sobhy Soliman","GK"},{"Ahmed Ayman","DEF"},{"Karim El Deeb","MID"},{"Islam Samir","MID"},{"Austin Amutu","STR"} });
    }
    else {
        leagueName = "Custom League";
    }

    leagueSchedule.clear();
    leagueFinishedThisSeason = false;
    cupFinishedThisSeason = false;
}

// -----------------------------------------
//  CUSTOM LEAGUE
// -----------------------------------------
void inputCustomLeague() {
    leagueName = "Custom League";
    teams.clear();

    int n;
    printHeader("CUSTOM LEAGUE SETUP");
    cout << "How many teams? (2-16): ";
    cin >> n;

    if (cin.fail()) {
        cin.clear();
        cin.ignore(1000, '\n');
        n = 2;
    }

    n = max(2, min(16, n));
    cin.ignore(1000, '\n');

    for (int t = 0; t < n; t++) {
        Team tm;

        cout << "\n" << BOLD << GREEN << "-- Team " << (t + 1) << " --" << RESET << "\n";
        cout << "Team name    : ";
        getline(cin, tm.name);
        if (tm.name.empty()) tm.name = "Team" + to_string(t + 1);

        cout << "Stadium name : ";
        getline(cin, tm.stadium);
        if (tm.stadium.empty()) tm.stadium = "Stadium";

        cout << "City         : ";
        getline(cin, tm.city);
        if (tm.city.empty()) tm.city = "City";

        cout << "Strength (1-10): ";
        cin >> tm.strength;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            tm.strength = 5;
        }
        tm.strength = max(1, min(10, tm.strength));

        cout << "Budget (millions): ";
        cin >> tm.budget;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(1000, '\n');
            tm.budget = 50;
        }
        cin.ignore(1000, '\n');

        cout << "\n  Enter 11 STARTERS:\n";
        for (int p = 0; p < 11; p++) {
            Player pl;
            cout << "  Starter " << (p + 1) << " name     : ";
            getline(cin, pl.name);
            if (pl.name.empty()) pl.name = "Starter" + to_string(p + 1);

            cout << "  Position (GK/DEF/MID/STR): ";
            getline(cin, pl.position);
            if (pl.position.empty()) pl.position = (p == 0 ? "GK" : p <= 4 ? "DEF" : p <= 7 ? "MID" : "STR");

            pl.marketValue = 10;
            pl.isSubstitute = false;
            tm.players.push_back(pl);
        }

        cout << "\n  Enter 5 SUBSTITUTES:\n";
        for (int p = 0; p < 5; p++) {
            Player pl;
            cout << "  Sub " << (p + 1) << " name          : ";
            getline(cin, pl.name);
            if (pl.name.empty()) pl.name = "Sub" + to_string(p + 1);

            cout << "  Position (GK/DEF/MID/STR): ";
            getline(cin, pl.position);
            if (pl.position.empty()) pl.position = "MID";

            pl.marketValue = 5;
            pl.isSubstitute = true;
            tm.players.push_back(pl);
        }

        teams.push_back(tm);
    }

    leagueSchedule.clear();
    leagueFinishedThisSeason = false;
    cupFinishedThisSeason = false;
}

// -----------------------------------------
//  SCHEDULE GENERATION
// -----------------------------------------
void generateLeagueSchedule() {
    leagueSchedule.clear();

    size_t n = teams.size();
    vector<int> order;
    for (int i = 0; i < (int)n; i++) order.push_back(i);

    if (n % 2 == 1) {
        order.push_back(-1);
        n++;
    }

    size_t rounds = n - 1;
    size_t half = n / 2;

    for (int round = 0; round < (int)rounds; round++) {
        for (int i = 0; i < (int)half; i++) {
            int a = order[i];
            int b = order[n - 1 - i];
            if (a == -1 || b == -1) continue;

            if (round % 2 == 0) leagueSchedule.push_back({ a, b, 0, 0, false, false });
            else leagueSchedule.push_back({ b, a, 0, 0, false, false });
        }

        vector<int> no;
        no.push_back(order[0]);
        no.push_back(order[n - 1]);
        for (int i = 1; i < (int)n - 1; i++) no.push_back(order[i]);
        order = no;
    }

    size_t fl = leagueSchedule.size();
    for (int i = 0; i < (int)fl; i++) {
        leagueSchedule.push_back({ leagueSchedule[i].awayIdx, leagueSchedule[i].homeIdx, 0, 0, false, false });
    }
}

// -----------------------------------------
//  INJURY UPDATE
// -----------------------------------------
void updateInjuries(Team& t) {
    for (auto& p : t.players) {
        if (p.injured && p.injuryGamesLeft > 0) {
            p.injuryGamesLeft--;
            if (p.injuryGamesLeft == 0) {
                p.injured = false;
                cout << GREEN;
                typeLine("  + " + p.name + " (" + t.name + ") has recovered!");
                cout << RESET;
            }
        }
    }
}

// -----------------------------------------
//  SIMULATE ONE MATCH
// -----------------------------------------
Match simulateOneMatch(int homeIdx, int awayIdx, bool isCup) {
    Team& home = teams[homeIdx];
    Team& away = teams[awayIdx];

    updateInjuries(home);
    updateInjuries(away);

    printDivider('-');
    cout << BOLD;
    typeLine("  " + home.name + " (Home)  vs  " + away.name + " (Away)");
    cout << RESET;
    printDivider('-');

    vector<CommentaryEvent> events;

    auto buildAvail = [&](Team& team, vector<int>& avail, vector<int>& strIdx) {
        int subsUsed = 0;
        const int MAX_SUBS = 5;
        vector<bool> subUsed(team.players.size(), false);

        for (int i = 0; i < (int)team.players.size(); i++) {
            if (team.players[i].isSubstitute) continue;

            if (!playerAvailable(team.players[i])) {
                if (subsUsed < MAX_SUBS) {
                    for (int s = 0; s < (int)team.players.size(); s++) {
                        if (!team.players[s].isSubstitute || !playerAvailable(team.players[s]) || subUsed[s]) continue;

                        addEvent(events, 1, "SUB: " + team.players[s].name + " on for " + team.players[i].name + " (" + team.name + ")", CYAN);

                        avail.push_back(s);
                        subUsed[s] = true;
                        if (team.players[s].position == "STR") strIdx.push_back(s);
                        subsUsed++;
                        break;
                    }
                }
            }
            else {
                avail.push_back(i);
                if (team.players[i].position == "STR") strIdx.push_back(i);
            }
        }

        int remaining = MAX_SUBS - subsUsed;
        int tactical = min(remaining, rng(0, 2));
        int made = 0;

        for (int s = 0; s < (int)team.players.size() && made < tactical; s++) {
            if (!team.players[s].isSubstitute || !playerAvailable(team.players[s]) || subUsed[s]) continue;

            int replaceAt = -1;
            for (int k = (int)avail.size() - 1; k >= 0; k--) {
                int idx = avail[k];
                if (!team.players[idx].isSubstitute && team.players[idx].position != "GK") {
                    replaceAt = k;
                    break;
                }
            }

            if (replaceAt >= 0) {
                int minute = rng(46, 85);
                addEvent(events, minute,
                    "SUB: " + team.players[s].name + " on for " + team.players[avail[replaceAt]].name + " (" + team.name + ")",
                    CYAN);

                avail.erase(avail.begin() + replaceAt);
                avail.push_back(s);
                subUsed[s] = true;
                if (team.players[s].position == "STR") strIdx.push_back(s);
                made++;
            }
        }
        };

    vector<int> homeAvail, homeStr, awayAvail, awayStr;
    buildAvail(home, homeAvail, homeStr);
    buildAvail(away, awayAvail, awayStr);

    int homeEff = min(10, home.strength + 1);
    int awayEff = away.strength;
    int homeGoals = max(0, rng(0, 2) + (homeEff >= 8 ? 1 : 0) + (homeEff >= 10 ? 1 : 0));
    int awayGoals = max(0, rng(0, 2) + (awayEff >= 8 ? 1 : 0) + (awayEff >= 10 ? 1 : 0));

    if (isCup && homeGoals == awayGoals) {
        addEvent(events, 91, "Extra time begins!", YELLOW);
        if (rng(1, 2) == 1) homeGoals++;
        else awayGoals++;
    }

    auto assignGoals = [&](int count, Team& team, vector<int>& str, vector<int>& any, const string& tname) {
        for (int g = 0; g < count; g++) {
            int minute = rng(1, 90);
            int scorer = -1;

            if (!str.empty() && rng(1, 10) <= 7) scorer = str[rng(0, (int)str.size() - 1)];
            else if (!any.empty()) scorer = any[rng(0, (int)any.size() - 1)];

            if (scorer >= 0) {
                team.players[scorer].goals++;

                string assistText = "";
                if (any.size() > 1) {
                    int ai;
                    do {
                        ai = any[rng(0, (int)any.size() - 1)];
                    } while (ai == scorer);
                    team.players[ai].assists++;
                    assistText = " Assist: " + team.players[ai].name + ".";
                }

                addEvent(events, minute,
                    "GOAL for " + tname + " - " + team.players[scorer].name + " scores!" + assistText,
                    GREEN);
            }
        }
        };

    assignGoals(homeGoals, home, homeStr, homeAvail, home.name);
    assignGoals(awayGoals, away, awayStr, awayAvail, away.name);

    auto processEvents = [&](Team& team, vector<int>& avail) {
        for (int idx : avail) {
            Player& p = team.players[idx];
            int minute = rng(1, 90);
            int roll = rng(1, 100);

            if (roll <= 5) {
                p.redCards++;
                p.totalRedCards++;
                p.suspended = true;
                addEvent(events, minute, "RED CARD - " + p.name + " (" + team.name + ") is sent off!", RED);
                checkAutoList(team, p);
            }
            else if (roll <= 22) {
                p.yellowCards++;
                addEvent(events, minute, "Yellow card - " + p.name + " (" + team.name + ")", YELLOW);
            }

            if (!p.injured && rng(1, 100) <= 3) {
                int games = rng(1, 3);
                p.injured = true;
                p.injuryGamesLeft = games;
                p.totalInjuries++;
                addEvent(events, minute,
                    "INJURY - " + p.name + " (" + team.name + ") is out for " + to_string(games) + " match(es)!",
                    MAGENTA);
                checkAutoList(team, p);
            }
        }
        };

    processEvents(home, homeAvail);
    processEvents(away, awayAvail);

    printChronologicalEvents(events);

    cout << BOLD;
    typeLine("\n  RESULT: " + home.name + " " + to_string(homeGoals) + " - " + to_string(awayGoals) + " " + away.name);
    cout << RESET;

    if (!isCup) {
        home.goalsScored += homeGoals;
        home.goalsConceded += awayGoals;
        away.goalsScored += awayGoals;
        away.goalsConceded += homeGoals;

        if (homeGoals > awayGoals) {
            home.wins++;
            home.points += 3;
            away.losses++;
            cout << GREEN;
            typeLine("  >> " + home.name + " wins!");
            cout << RESET;
        }
        else if (awayGoals > homeGoals) {
            away.wins++;
            away.points += 3;
            home.losses++;
            cout << GREEN;
            typeLine("  >> " + away.name + " wins!");
            cout << RESET;
        }
        else {
            home.draws++;
            home.points++;
            away.draws++;
            away.points++;
            cout << YELLOW;
            typeLine("  >> Draw!");
            cout << RESET;
        }
    }
    else {
        int winner = (homeGoals > awayGoals) ? homeIdx : awayIdx;
        cout << GREEN;
        typeLine("  >> " + teams[winner].name + " advances!");
        cout << RESET;
    }

    for (auto& p : home.players) p.suspended = false;
    for (auto& p : away.players) p.suspended = false;

    return { homeIdx, awayIdx, homeGoals, awayGoals, true, isCup };
}

// -----------------------------------------
//  LEAGUE TABLE
// -----------------------------------------
void printTable() {
    int n = (int)teams.size();
    vector<int> order;
    for (int i = 0; i < n; i++) order.push_back(i);

    sort(order.begin(), order.end(), [](int a, int b) {
        if (teams[a].points != teams[b].points) return teams[a].points > teams[b].points;
        int gda = teams[a].goalsScored - teams[a].goalsConceded;
        int gdb = teams[b].goalsScored - teams[b].goalsConceded;
        if (gda != gdb) return gda > gdb;
        return teams[a].goalsScored > teams[b].goalsScored;
        });

    printHeader(leagueName + " TABLE  -  Season " + to_string(currentSeason));
    cout << BOLD << left << setw(5) << "  #" << left << setw(22) << "Team"
        << right << setw(5) << "Pts" << right << setw(4) << "W" << right << setw(4) << "D"
        << right << setw(4) << "L" << right << setw(5) << "GF" << right << setw(5) << "GA"
        << right << setw(5) << "GD" << RESET << "\n";
    printDivider('-');

    for (int i = 0; i < n; i++) {
        Team& t = teams[order[i]];
        int gd = t.goalsScored - t.goalsConceded;
        string col = (i == 0) ? GREEN : (i >= n - 3 ? RED : WHITE);
        string gds = (gd >= 0 ? "+" : "") + to_string(gd);

        cout << col << BOLD << left << setw(5) << ("  " + to_string(i + 1) + ".")
            << left << setw(22) << t.name << right << setw(5) << t.points
            << right << setw(4) << t.wins << right << setw(4) << t.draws << right << setw(4) << t.losses
            << right << setw(5) << t.goalsScored << right << setw(5) << t.goalsConceded
            << right << setw(5) << gds << RESET << "\n";
    }

    printDivider();
}

// -----------------------------------------
//  TOP SCORERS
// -----------------------------------------
void printTopScorers(int topN = 5) {
    struct E { string name, team; int goals, assists; };
    vector<E> ev;

    for (auto& t : teams)
        for (auto& p : t.players)
            if (p.goals > 0) ev.push_back({ p.name, t.name, p.goals, p.assists });

    sort(ev.begin(), ev.end(), [](const E& a, const E& b) {
        return a.goals > b.goals;
        });

    printHeader("TOP SCORERS");
    int show = min((int)ev.size(), topN);

    if (!show) {
        cout << "  No goals yet.\n";
        printDivider();
        return;
    }

    for (int i = 0; i < show; i++) {
        cout << BOLD << YELLOW << "  " << (i + 1) << ".  " << left << setw(22) << ev[i].name
            << "(" << ev[i].team << ")  " << ev[i].goals << "G  " << ev[i].assists << "A\n" << RESET;
    }

    printDivider();
}

// -----------------------------------------
//  INJURY REPORT
// -----------------------------------------
void printInjuryReport() {
    printHeader("INJURY REPORT");
    bool any = false;

    for (auto& t : teams)
        for (auto& p : t.players)
            if (p.injured) {
                cout << MAGENTA << "  [OUT] " << p.name << " (" << t.name << ")  --  "
                    << p.injuryGamesLeft << " match(es) left\n" << RESET;
                any = true;
            }

    if (!any) cout << GREEN << "  All players fit!\n" << RESET;
    printDivider();
}

// -----------------------------------------
//  SQUAD VIEWER
// -----------------------------------------
void viewSquad() {
    printHeader("VIEW SQUAD");
    for (int i = 0; i < (int)teams.size(); i++) cout << "  " << (i + 1) << ". " << teams[i].name << "\n";

    cout << "  Choice (0=cancel): ";
    int ch;
    cin >> ch;
    cin.ignore(1000, '\n');

    if (ch < 1 || ch >(int)teams.size()) return;

    Team& t = teams[ch - 1];
    printHeader(t.name + " - Full Squad");
    cout << BOLD << left << setw(4) << "#" << setw(22) << "Name" << setw(6) << "Pos"
        << setw(9) << "Role" << setw(6) << "G" << setw(6) << "A" << setw(5) << "YC"
        << setw(5) << "RC" << "Status\n" << RESET;
    printDivider('-');

    for (int j = 0; j < (int)t.players.size(); j++) {
        Player& p = t.players[j];
        string role = p.isSubstitute ? "SUB" : "STARTER";
        string status = p.injured ? (MAGENTA + "INJ(" + to_string(p.injuryGamesLeft) + ")" + RESET)
            : p.suspended ? (RED + "SUSP" + RESET) : (GREEN + "FIT" + RESET);
        string tag = p.listedForSale ? (YELLOW + " [LISTED " + to_string(p.askingPrice) + "M]" + RESET) : "";

        cout << left << setw(4) << (j + 1) << setw(22) << p.name << setw(6) << p.position
            << setw(9) << role << setw(6) << p.goals << setw(6) << p.assists
            << setw(5) << p.yellowCards << setw(5) << p.redCards << status << tag << "\n";
    }

    printDivider();
    pressEnter();
}

// -----------------------------------------
//  RUN LEAGUE
// -----------------------------------------
void runLeague() {
    if (leagueFinishedThisSeason) {
        printHeader("LEAGUE ALREADY COMPLETED");
        cout << YELLOW;
        typeLine("  This season's league has already finished.");
        typeLine("  Start a new season to play another league campaign.");
        cout << RESET;
        pressEnter();
        return;
    }

    if (leagueSchedule.empty()) {
        generateLeagueSchedule();
    }

    printHeader("SEASON " + to_string(currentSeason) + " - " + leagueName + " BEGINS");
    backToMainMenu = false;

    int perDay = max(1, (int)teams.size() / 2);
    bool skipToEnd = false;
    int matchday = 1;

    for (int i = 0; i < (int)leagueSchedule.size(); ) {
        cout << "\n" << BOLD << BLUE << "======  MATCHDAY " << matchday << "  ======\n" << RESET;
        int end = min(i + perDay, (int)leagueSchedule.size());

        for (; i < end; i++) {
            Match& m = leagueSchedule[i];
            m = simulateOneMatch(m.homeIdx, m.awayIdx, false);
        }

        cout << "\n";
        printTable();
        printTopScorers();
        printInjuryReport();

        if (i < (int)leagueSchedule.size() && !skipToEnd) {
            cout << "\n  1. Next matchday\n  2. Skip to end\n  0. Back\n\n  Choice: ";
            int ch;
            cin >> ch;
            cin.ignore(1000, '\n');

            if (ch == 2) {
                skipToEnd = true;
                fastDisplay = true;
            }
            else if (ch == 0) {
                backToMainMenu = true;
                return;
            }
        }

        matchday++;
    }

    leagueFinishedThisSeason = true;
}

// -----------------------------------------
//  RUN CUP
// -----------------------------------------
void runCup() {
    if (cupFinishedThisSeason) {
        printHeader("CUP ALREADY COMPLETED");
        cout << YELLOW;
        typeLine("  This season's cup has already finished.");
        typeLine("  Start a new season to play another cup.");
        cout << RESET;
        pressEnter();
        return;
    }

    printHeader("SEASON " + to_string(currentSeason) + " - CUP COMPETITION");
    backToMainMenu = false;
    bool skipToEnd = false;
    int round = 1;

    vector<int> pool;
    for (int i = 0; i < (int)teams.size(); i++) pool.push_back(i);
    for (int i = (int)pool.size() - 1; i > 0; i--) swap(pool[i], pool[rng(0, i)]);

    while (pool.size() > 1) {
        cout << "\n" << BOLD << BLUE << "======  CUP ROUND " << round << "  ======\n" << RESET;
        vector<int> survivors;

        for (int i = 0; i + 1 < (int)pool.size(); i += 2) {
            Match r = simulateOneMatch(pool[i], pool[i + 1], true);
            survivors.push_back((r.homeGoals > r.awayGoals) ? pool[i] : pool[i + 1]);

            if (!skipToEnd && i + 2 < (int)pool.size()) {
                cout << "\n  1. Continue\n  2. Skip\n  0. Back\n\n  Choice: ";
                int ch;
                cin >> ch;
                cin.ignore(1000, '\n');

                if (ch == 2) {
                    skipToEnd = true;
                    fastDisplay = true;
                }
                else if (ch == 0) {
                    backToMainMenu = true;
                    return;
                }
            }
        }

        if (pool.size() % 2 == 1) {
            cout << CYAN;
            typeLine("  " + teams[pool.back()].name + " gets a bye.");
            cout << RESET;
            survivors.push_back(pool.back());
        }

        pool = survivors;
        round++;
    }

    if (!pool.empty()) {
        cout << "\n" << BOLD << GREEN;
        typeLine("  *** CUP WINNER: " + teams[pool[0]].name + " ***");
        cout << RESET;
        printDivider();

        if (!history.empty() && history.back().season == currentSeason)
            history.back().cupWinner = teams[pool[0]].name;
    }

    cupFinishedThisSeason = true;
}

// -----------------------------------------
//  TRANSFER MARKET
// -----------------------------------------
void transferMarket() {
    while (true) {
        printHeader("TRANSFER MARKET");
        cout << "\n  1. Browse players for sale\n  2. List a player for sale\n"
            << "  3. Buy a player\n  4. View team budgets\n  0. Back\n\n  Choice: ";

        int c;
        cin >> c;
        cin.ignore(1000, '\n');

        if (c == 0) return;

        if (c == 1) {
            printHeader("PLAYERS FOR SALE");
            bool any = false;
            int idx = 1;

            for (int t = 0; t < (int)teams.size(); t++) {
                for (auto& pl : teams[t].players) {
                    if (pl.listedForSale) {
                        string tag = (pl.totalInjuries >= 3 || pl.totalRedCards >= 3) ? RED + " [AUTO]" + RESET : "";
                        cout << BOLD << "  " << idx++ << ". " << left << setw(22) << pl.name
                            << left << setw(6) << pl.position << "Team: " << left << setw(20) << teams[t].name
                            << "Val:" << pl.marketValue << "M  Ask:" << YELLOW << pl.askingPrice << "M" << RESET << tag << "\n";
                        any = true;
                    }
                }
            }

            if (!any) cout << "  No players listed.\n";
            pressEnter();
        }
        else if (c == 2) {
            cout << "  Team (1-" << teams.size() << ", 0=cancel): ";
            int ti;
            cin >> ti;
            cin.ignore(1000, '\n');

            if (ti == 0) continue;
            ti--;

            if (ti < 0 || ti >= (int)teams.size()) {
                cout << "Invalid.\n";
                pressEnter();
                continue;
            }

            for (int p = 0; p < (int)teams[ti].players.size(); p++) {
                auto& pl = teams[ti].players[p];
                cout << "  " << (p + 1) << ". " << left << setw(22) << pl.name << pl.position
                    << "  Val:" << pl.marketValue << "M" << (pl.listedForSale ? YELLOW + " [LISTED]" + RESET : "") << "\n";
            }

            cout << "  Player to list (0=cancel): ";
            int pi;
            cin >> pi;
            cin.ignore(1000, '\n');

            if (pi == 0) continue;
            pi--;

            if (pi < 0 || pi >= (int)teams[ti].players.size()) {
                cout << "Invalid.\n";
                pressEnter();
                continue;
            }

            cout << "  Asking price (M): ";
            int price;
            cin >> price;
            cin.ignore(1000, '\n');

            teams[ti].players[pi].listedForSale = true;
            teams[ti].players[pi].askingPrice = price;
            cout << GREEN;
            typeLine("  Listed!");
            cout << RESET;
            pressEnter();
        }
        else if (c == 3) {
            cout << "  Buying team (1-" << teams.size() << ", 0=cancel): ";
            int bi;
            cin >> bi;
            cin.ignore(1000, '\n');

            if (bi == 0) continue;
            bi--;

            if (bi < 0 || bi >= (int)teams.size()) {
                cout << "Invalid.\n";
                pressEnter();
                continue;
            }

            vector<pair<int, int>> listed;
            int idx = 1;

            for (int t = 0; t < (int)teams.size(); t++) {
                if (t == bi) continue;
                for (int p = 0; p < (int)teams[t].players.size(); p++) {
                    if (teams[t].players[p].listedForSale) {
                        cout << "  " << idx++ << ". " << left << setw(22) << teams[t].players[p].name
                            << left << setw(6) << teams[t].players[p].position
                            << "From:" << left << setw(20) << teams[t].name
                            << YELLOW << teams[t].players[p].askingPrice << "M" << RESET << "\n";
                        listed.push_back({ t, p });
                    }
                }
            }

            if (listed.empty()) {
                cout << "No players for sale.\n";
                pressEnter();
                continue;
            }

            cout << "\n  " << teams[bi].name << " Budget: " << GREEN << teams[bi].budget << "M" << RESET << "\n";
            cout << "  Which player? (0=cancel): ";

            int sel;
            cin >> sel;
            cin.ignore(1000, '\n');

            if (sel == 0) continue;
            if (sel < 1 || sel >(int)listed.size()) {
                cout << "Invalid.\n";
                pressEnter();
                continue;
            }

            int si = listed[sel - 1].first;
            int pi = listed[sel - 1].second;
            Player pl = teams[si].players[pi];
            int asking = pl.askingPrice;

            cout << "  Asking: " << asking << "M. Offer (0=cancel): ";
            int offer;
            cin >> offer;
            cin.ignore(1000, '\n');

            if (offer == 0) continue;

            if (offer < asking) {
                if (offer >= asking * 80 / 100) {
                    cout << YELLOW;
                    typeLine("  Counter accepted!");
                    cout << RESET;
                }
                else {
                    cout << RED;
                    typeLine("  Rejected. Min: " + to_string(asking * 80 / 100) + "M.");
                    cout << RESET;
                    pressEnter();
                    continue;
                }
            }

            if (teams[bi].budget < offer) {
                cout << RED;
                typeLine("  Not enough budget!");
                cout << RESET;
                pressEnter();
                continue;
            }

            teams[bi].budget -= offer;
            teams[si].budget += offer;
            pl.listedForSale = false;
            pl.askingPrice = 0;
            teams[bi].players.push_back(pl);
            teams[si].players.erase(teams[si].players.begin() + pi);

            cout << GREEN;
            typeLine("  TRANSFER: " + pl.name + " joins " + teams[bi].name + "!");
            cout << RESET;
            pressEnter();
        }
        else if (c == 4) {
            printHeader("TEAM BUDGETS");
            for (auto& t : teams) {
                cout << "  " << BOLD << left << setw(25) << t.name << RESET << GREEN << t.budget << "M\n" << RESET;
            }
            pressEnter();
        }
    }
}

// -----------------------------------------
//  SUMMARY & HISTORY
// -----------------------------------------
string getLeagueWinner() {
    int best = 0;

    for (int i = 1; i < (int)teams.size(); i++) {
        int gdI = teams[i].goalsScored - teams[i].goalsConceded;
        int gdB = teams[best].goalsScored - teams[best].goalsConceded;

        if (teams[i].points > teams[best].points) best = i;
        else if (teams[i].points == teams[best].points && gdI > gdB) best = i;
    }

    return teams[best].name;
}

string getTopScorer(int& goals) {
    string name = "None";
    goals = 0;

    for (auto& t : teams)
        for (auto& p : t.players)
            if (p.goals > goals) {
                goals = p.goals;
                name = p.name;
            }

    return name;
}

void printSeasonSummary() {
    printHeader("SEASON " + to_string(currentSeason) + " FINAL SUMMARY");
    string winner = getLeagueWinner();
    int tg;
    string scorer = getTopScorer(tg);

    cout << BOLD << GREEN;
    typeLine("  LEAGUE WINNER : " + winner);
    cout << RESET;

    cout << BOLD << CYAN;
    typeLine("  " + getChampionMessage(winner));
    cout << RESET;

    cout << BOLD << YELLOW;
    typeLine("  TOP SCORER    : " + scorer + " (" + to_string(tg) + " goals)");
    cout << RESET;

    printDivider();

    if (!seasonAlreadySaved()) {
        history.push_back({ currentSeason, winner, "TBD", scorer, tg });
    }
}

void printHistory() {
    if (history.empty()) {
        cout << "  No seasons completed yet.\n";
        return;
    }

    printHeader("SEASON HISTORY");
    cout << BOLD << left << setw(8) << "Season" << setw(25) << "League Winner"
        << setw(25) << "Cup Winner" << setw(25) << "Top Scorer" << "Goals\n" << RESET;
    printDivider('-');

    for (auto& r : history) {
        cout << left << setw(8) << r.season << setw(25) << r.leagueWinner
            << setw(25) << r.cupWinner << setw(25) << r.topScorer << r.topScorerGoals << "\n";
    }

    printDivider();
}

// -----------------------------------------
//  LEAGUE SELECTION
// -----------------------------------------
void selectLeague() {
    printHeader("SELECT YOUR LEAGUE");
    cout << "  1. Premier League \n"
        << "  2. La Liga \n"
        << "  3. Serie A \n"
        << "  4. Ligue 1 \n"
        << "  5. Bundesliga \n"
        << "  6. Egyptian Premier League\n"
        << "  7. Custom League\n\n"
        << "  Choice: ";

    int ch;
    cin >> ch;
    cin.ignore(1000, '\n');

    if (ch >= 1 && ch <= 6) loadPresetLeague(ch);
    else inputCustomLeague();

    leagueFinishedThisSeason = false;
    cupFinishedThisSeason = false;
    leagueSchedule.clear();

    cout << GREEN;
    typeLine("\n  " + leagueName + " loaded with " + to_string((int)teams.size()) + " teams!");
    cout << RESET;
}

// -----------------------------------------
//  MAIN MENU
// -----------------------------------------
void mainMenu() {
    while (true) {
        printHeader("MAIN MENU  -  " + leagueName + "  |  Season " + to_string(currentSeason));
        cout << "  1.  Run League Season\n"
            << "  2.  Run Cup Competition\n"
            << "  3.  Transfer Market\n"
            << "  4.  View League Table\n"
            << "  5.  View Top Scorers\n"
            << "  6.  View Injury Report\n"
            << "  7.  View Squad\n"
            << "  8.  View Season History\n"
            << "  9.  Start New Season\n"
            << "  10. Toggle Fast Display (" << (fastDisplay ? "ON" : "OFF") << ")\n"
            << "  11. Exit\n\n"
            << "  Choice: ";

        int c;
        cin >> c;
        cin.ignore(1000, '\n');

        if (c == 1) {
            bool old = fastDisplay;
            runLeague();
            if (leagueFinishedThisSeason && !backToMainMenu) {
                printSeasonSummary();
            }
            fastDisplay = old;
            backToMainMenu = false;
        }
        else if (c == 2) {
            bool old = fastDisplay;
            runCup();
            fastDisplay = old;
            backToMainMenu = false;
        }
        else if (c == 3) {
            transferMarket();
        }
        else if (c == 4) {
            printTable();
            pressEnter();
        }
        else if (c == 5) {
            printTopScorers(10);
            pressEnter();
        }
        else if (c == 6) {
            printInjuryReport();
            pressEnter();
        }
        else if (c == 7) {
            viewSquad();
        }
        else if (c == 8) {
            printHistory();
            pressEnter();
        }
        else if (c == 9) {
            if (leagueFinishedThisSeason && !seasonAlreadySaved()) {
                printSeasonSummary();
            }

            currentSeason++;
            resetSeasonStats();

            cout << GREEN;
            typeLine("\n  Season " + to_string(currentSeason) + " starts!");
            cout << RESET;

            cout << "\n  Continue with " << leagueName << " or choose a different league?\n"
                << "  1. Continue with " << leagueName << "\n"
                << "  2. Choose a different league\n\n"
                << "  Choice: ";

            int lch;
            cin >> lch;
            cin.ignore(1000, '\n');

            if (lch == 2) {
                selectLeague();
            }

            pressEnter();
        }
        else if (c == 10) {
            fastDisplay = !fastDisplay;
            cout << GREEN;
            typeLine("  Fast display: " + string(fastDisplay ? "ON" : "OFF"));
            cout << RESET;
            pressEnter();
        }
        else if (c == 11) {
            cout << CYAN;
            typeLine("\n  Goodbye!");
            cout << RESET;
            break;
        }
        else {
            cout << RED << "  Invalid choice.\n" << RESET;
            pressEnter();
        }
    }
}

// -----------------------------------------
//  MAIN
// -----------------------------------------
int main() {
    srand((unsigned)time(0));

    cout << BOLD << CYAN
        << "\n+----------------------------------------------------------+\n"
        << "|      FOOTBALL LEAGUE SIMULATOR                        |\n"
        << "|  Leagues | Cup | Transfers | Subs | Multi-Season        |\n"
        << "+----------------------------------------------------------+\n"
        << RESET;

    selectLeague();
    pressEnter();
    mainMenu();
    return 0;
}