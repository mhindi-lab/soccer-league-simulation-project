#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <mutex>
#include <thread>
#include <chrono>
#include <cctype>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

#include "httplib.h"

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

struct MatchSimulationResult {
    Match match;
    string homeName = "";
    string awayName = "";
    string stageLabel = "";
    string resultLine = "";
    string winnerLine = "";
    vector<CommentaryEvent> events;
};

struct ActionReport {
    string title = "";
    string summary = "";
    vector<MatchSimulationResult> matches;
    vector<string> notes;
    int batchesSimulated = 0;
};

struct SaveSnapshot {
    string savedAt = "";
    vector<Team> teams;
    vector<Match> leagueSchedule;
    vector<SeasonRecord> history;
    vector<int> cupPool;
    int currentSeason = 1;
    string leagueName = "Custom League";
    bool fastDisplay = false;
    bool backToMainMenu = false;
    bool leagueFinishedThisSeason = false;
    bool cupFinishedThisSeason = false;
    bool cupStartedThisSeason = false;
    int cupRoundNumber = 1;
    string cupWinnerThisSeason = "TBD";
};

struct SaveFileSummary {
    bool exists = false;
    bool valid = false;
    string leagueName = "";
    int currentSeason = 0;
    string savedAt = "";
};

// -----------------------------------------
//  HTML GUI GENERATOR
// -----------------------------------------
class HTMLGenerator {
private:
    static string escapeHTML(const string& s) {
        string r;
        for (char c : s) {
            if (c == '<') r += "&lt;";
            else if (c == '>') r += "&gt;";
            else if (c == '&') r += "&amp;";
            else if (c == '"') r += "&quot;";
            else r += c;
        }
        return r;
    }

    static string getEventClass(const string& text) {
        if (text.find("GOAL") != string::npos) return "goal";
        if (text.find("Yellow") != string::npos) return "yellow";
        if (text.find("RED") != string::npos) return "red";
        if (text.find("INJURY") != string::npos) return "injury";
        if (text.find("SUB") != string::npos) return "sub";
        return "other";
    }

    static void openHTML(const string& filename) {
        #ifdef _WIN32
        system(("start \"\" \"" + filename + "\"").c_str());
        #else
        system(("open \"" + filename + "\"").c_str());
        #endif
    }

public:
    static void showMainMenu(const string& leagueName, int currentSeason, const vector<Team>& teams,
                             const vector<SeasonRecord>& history, bool leagueFinished, bool cupFinished) {
        showStandings(leagueName, currentSeason, teams, false);
        showTopScorers(leagueName, currentSeason, teams, false);
        showInjuryReport(leagueName, currentSeason, teams, false);
        showSquadHub(teams, false);
        showHistory(history, false);

        stringstream html;
        html << R"MENU(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>League Simulator Dashboard</title>
    <style>
        :root {
            --bg-1: #07121b;
            --bg-2: #0d2231;
            --panel: rgba(8, 20, 31, 0.78);
            --panel-border: rgba(116, 210, 255, 0.18);
            --text: #eef8ff;
            --muted: #94aab9;
            --accent: #74d2ff;
            --accent-2: #ffb45c;
            --success: #7fd68f;
            --shadow: 0 28px 80px rgba(0, 0, 0, 0.45);
        }
        * { box-sizing: border-box; }
        body {
            margin: 0;
            min-height: 100vh;
            padding: 32px 16px;
            font-family: 'Trebuchet MS', Verdana, sans-serif;
            color: var(--text);
            background:
                radial-gradient(circle at top left, rgba(116, 210, 255, 0.18), transparent 34%),
                radial-gradient(circle at bottom right, rgba(255, 180, 92, 0.14), transparent 30%),
                linear-gradient(145deg, var(--bg-1) 0%, var(--bg-2) 55%, #081723 100%);
        }
        .dashboard {
            max-width: 1080px;
            margin: 0 auto;
        }
        .hero,
        .panel {
            background: var(--panel);
            border: 1px solid var(--panel-border);
            border-radius: 28px;
            box-shadow: var(--shadow);
            backdrop-filter: blur(16px);
        }
        .hero {
            padding: 32px;
            overflow: hidden;
            position: relative;
        }
        .hero::after {
            content: '';
            position: absolute;
            inset: auto -80px -90px auto;
            width: 240px;
            height: 240px;
            border-radius: 50%;
            background: radial-gradient(circle, rgba(116, 210, 255, 0.28), transparent 70%);
        }
        .eyebrow {
            display: inline-block;
            padding: 6px 12px;
            border-radius: 999px;
            background: rgba(116, 210, 255, 0.12);
            color: var(--accent);
            font-size: 0.78rem;
            letter-spacing: 0.16em;
            text-transform: uppercase;
        }
        h1 {
            margin: 18px 0 12px;
            font-size: clamp(2rem, 4vw, 3.3rem);
            line-height: 1.05;
        }
        .hero-copy {
            max-width: 620px;
            margin: 0;
            color: var(--muted);
            line-height: 1.6;
            font-size: 1rem;
        }
        .stat-strip {
            margin-top: 26px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 14px;
        }
        .stat-card {
            padding: 16px 18px;
            border-radius: 20px;
            background: rgba(255, 255, 255, 0.04);
            border: 1px solid rgba(255, 255, 255, 0.06);
        }
        .stat-label {
            display: block;
            margin-bottom: 8px;
            color: var(--muted);
            font-size: 0.8rem;
            text-transform: uppercase;
            letter-spacing: 0.1em;
        }
        .stat-card strong {
            font-size: 1.1rem;
            color: var(--text);
        }
        .layout {
            margin-top: 18px;
            display: grid;
            grid-template-columns: minmax(0, 1.2fr) minmax(0, 0.95fr);
            gap: 18px;
        }
        .panel {
            padding: 24px;
        }
        .panel-head {
            margin-bottom: 18px;
        }
        .panel-head h2 {
            margin: 0 0 8px;
            font-size: 1.45rem;
        }
        .panel-head p {
            margin: 0;
            color: var(--muted);
            line-height: 1.5;
        }
        .card-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
            gap: 14px;
        }
        .menu-card,
        .action-card {
            border-radius: 22px;
            border: 1px solid rgba(116, 210, 255, 0.12);
            background: linear-gradient(180deg, rgba(255, 255, 255, 0.05), rgba(255, 255, 255, 0.02));
            transition: transform 0.22s ease, border-color 0.22s ease, box-shadow 0.22s ease;
        }
        .menu-card {
            display: block;
            padding: 20px;
            color: inherit;
            text-decoration: none;
        }
        .menu-card:hover,
        .action-card:hover {
            transform: translateY(-4px);
            border-color: rgba(116, 210, 255, 0.35);
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.2);
        }
        .card-tag,
        .action-meta,
        .option-badge {
            display: inline-block;
            padding: 5px 10px;
            border-radius: 999px;
            background: rgba(116, 210, 255, 0.12);
            color: var(--accent);
            font-size: 0.74rem;
            letter-spacing: 0.08em;
            text-transform: uppercase;
        }
        .card-title,
        .action-title,
        .hint-title {
            margin-top: 14px;
            font-size: 1.15rem;
            font-weight: bold;
        }
        .card-copy,
        .action-copy,
        .hint-copy,
        .footer-note {
            margin-top: 10px;
            color: var(--muted);
            line-height: 1.55;
        }
        .action-list {
            display: grid;
            gap: 12px;
        }
        .action-card {
            width: 100%;
            padding: 18px;
            color: inherit;
            text-align: left;
            cursor: pointer;
        }
        .console-panel {
            margin-top: 16px;
            padding: 20px;
            border-radius: 22px;
            background: rgba(4, 14, 23, 0.92);
            border: 1px solid rgba(116, 210, 255, 0.18);
            min-height: 190px;
        }
        .step-list {
            margin-top: 16px;
            display: grid;
            gap: 10px;
        }
        .step {
            padding: 12px 14px;
            border-radius: 14px;
            background: rgba(255, 255, 255, 0.04);
            color: #dbe8f0;
        }
        .footer-note {
            margin-bottom: 0;
        }
        @media (max-width: 900px) {
            .layout {
                grid-template-columns: 1fr;
            }
        }
        @media (max-width: 640px) {
            body {
                padding: 18px 12px;
            }
            .hero,
            .panel {
                padding: 20px;
                border-radius: 22px;
            }
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <section class="hero">
            <div class="eyebrow">League Simulator</div>
            <h1>)MENU" << escapeHTML(leagueName) << R"MENU( Dashboard</h1>
            <p class="hero-copy">Use the browser for live reports and the console for gameplay actions. This keeps the interface clean while the simulator logic stays in C++.</p>
            <div class="stat-strip">
                <div class="stat-card">
                    <span class="stat-label">Season</span>
                    <strong>)MENU" << currentSeason << R"MENU(</strong>
                </div>
                <div class="stat-card">
                    <span class="stat-label">Teams</span>
                    <strong>)MENU" << teams.size() << R"MENU(</strong>
                </div>
                <div class="stat-card">
                    <span class="stat-label">League</span>
                    <strong>)MENU" << (leagueFinished ? "Completed" : "In Progress") << R"MENU(</strong>
                </div>
                <div class="stat-card">
                    <span class="stat-label">Cup</span>
                    <strong>)MENU" << (cupFinished ? "Completed" : "In Progress") << R"MENU(</strong>
                </div>
            </div>
        </section>

        <div class="layout">
            <section class="panel">
                <div class="panel-head">
                    <h2>Browser Views</h2>
                    <p>These pages open right away and show the latest data generated by the simulator.</p>
                </div>
                <div class="card-grid">
                    <a class="menu-card" href="standings.html">
                        <span class="card-tag">Browser</span>
                        <div class="card-title">League Table</div>
                        <div class="card-copy">See points, goals, results and the current order of every team.</div>
                    </a>
                    <a class="menu-card" href="scorers.html">
                        <span class="card-tag">Browser</span>
                        <div class="card-title">Top Scorers</div>
                        <div class="card-copy">Open the golden boot race with goals and assists leaders.</div>
                    </a>
                    <a class="menu-card" href="injuries.html">
                        <span class="card-tag">Browser</span>
                        <div class="card-title">Injury Report</div>
                        <div class="card-copy">Check who is unavailable and how many games are left.</div>
                    </a>
                    <a class="menu-card" href="squads.html">
                        <span class="card-tag">Browser</span>
                        <div class="card-title">Squad Viewer</div>
                        <div class="card-copy">Browse every team and open a polished squad page for each one.</div>
                    </a>
                    <a class="menu-card" href="history.html">
                        <span class="card-tag">Browser</span>
                        <div class="card-title">Season History</div>
                        <div class="card-copy">Review winners, cup champions and top scorers across seasons.</div>
                    </a>
                </div>
                <p class="footer-note">After you run matches in the console, reopen this dashboard from the main menu to refresh the browser pages.</p>
            </section>

            <section class="panel">
                <div class="panel-head">
                    <h2>Console Actions</h2>
                    <p>These still run inside the simulator window, and the buttons show the exact menu key to use.</p>
                </div>
                <div class="action-list">
                    <button class="action-card" type="button" onclick="showConsoleAction(1, 'Run League Season', 'Simulate the full league season from the console window.')">
                        <span class="action-meta">Menu 1</span>
                        <div class="action-title">Run League Season</div>
                        <div class="action-copy">Progress the league, play matchdays and refresh all reports afterward.</div>
                    </button>
                    <button class="action-card" type="button" onclick="showConsoleAction(2, 'Run Cup Competition', 'Play the knockout cup rounds in the console window.')">
                        <span class="action-meta">Menu 2</span>
                        <div class="action-title">Run Cup Competition</div>
                        <div class="action-copy">Advance the cup rounds and update the season story.</div>
                    </button>
                    <button class="action-card" type="button" onclick="showConsoleAction(3, 'Transfer Market', 'Open the transfer market flow in the console window.')">
                        <span class="action-meta">Menu 3</span>
                        <div class="action-title">Transfer Market</div>
                        <div class="action-copy">List, buy and manage players with the simulator still in control.</div>
                    </button>
                    <button class="action-card" type="button" onclick="showConsoleAction(9, 'Start New Season', 'Move into the next season from the console menu.')">
                        <span class="action-meta">Menu 9</span>
                        <div class="action-title">Start New Season</div>
                        <div class="action-copy">Reset the campaign, carry history forward and keep the browser pages fresh.</div>
                    </button>
                    <button class="action-card" type="button" onclick="showConsoleAction(10, 'Toggle Fast Display', 'Switch fast display on or off inside the console window.')">
                        <span class="action-meta">Menu 10</span>
                        <div class="action-title">Toggle Fast Display</div>
                        <div class="action-copy">Use this when you want quicker simulation output in the console.</div>
                    </button>
                </div>

                <div class="console-panel" id="console-panel">
                    <span class="option-badge">Console First</span>
                    <div class="hint-title">This dashboard is now your browser control center.</div>
                    <div class="hint-copy">Open reports here, then switch back to the simulator console whenever you want to run gameplay actions.</div>
                    <div class="step-list">
                        <div class="step">1. Use the cards on the left for league reports.</div>
                        <div class="step">2. Click an action on the right to see its exact menu key.</div>
                        <div class="step">3. Reopen the dashboard from the main menu any time you want refreshed pages.</div>
                    </div>
                </div>
            </section>
        </div>
    </div>

    <script>
        function showConsoleAction(option, title, detail) {
            const panel = document.getElementById('console-panel');
            panel.innerHTML =
                '<span class="option-badge">Menu ' + option + '</span>' +
                '<div class="hint-title">' + title + '</div>' +
                '<div class="hint-copy">' + detail + '</div>' +
                '<div class="step-list">' +
                    '<div class="step">1. Switch back to the simulator console window.</div>' +
                    '<div class="step">2. Enter <strong>' + option + '</strong> at the main menu prompt.</div>' +
                    '<div class="step">3. Reopen the HTML dashboard from the main menu whenever you want fresh browser pages.</div>' +
                '</div>';
        }
    </script>
</body>
</html>)MENU";

        ofstream f("menu.html");
        if (f.is_open()) { f << html.str(); f.close(); }
        openHTML("menu.html");
    }

    static void showStandings(const string& leagueName, int currentSeason, const vector<Team>& teams,
                              bool openInBrowser = true) {
        vector<pair<int, Team>> sorted;
        for (int i = 0; i < (int)teams.size(); i++) sorted.push_back({i, teams[i]});
        sort(sorted.begin(), sorted.end(), [&](const pair<int, Team>& a, const pair<int, Team>& b) {
            if (a.second.points != b.second.points) return a.second.points > b.second.points;
            int gda = a.second.goalsScored - a.second.goalsConceded;
            int gdb = b.second.goalsScored - b.second.goalsConceded;
            if (gda != gdb) return gda > gdb;
            return a.second.goalsScored > b.second.goalsScored;
        });

        string html = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>" + escapeHTML(leagueName) + " - Standings</title>\n"
            "<style>\n"
            "* { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #0d1b2a 0%, #1b263b 100%); min-height: 100vh; padding: 20px; }\n"
            ".container { max-width: 900px; margin: 0 auto; }\n"
            ".header { text-align: center; margin-bottom: 30px; }\n"
            ".header h1 { color: #fff; font-size: 2em; font-weight: 300; }\n"
            ".header .season { color: #4fc3f7; }\n"
            ".table-container { background: rgba(255,255,255,0.05); border-radius: 15px; overflow: hidden; }\n"
            "table { width: 100%; border-collapse: collapse; }\n"
            "thead { background: linear-gradient(90deg, #1e3a5f 0%, #0d2840 100%); }\n"
            "th { color: #4fc3f7; padding: 15px 10px; text-align: left; font-weight: 600; font-size: 0.85em; text-transform: uppercase; }\n"
            "th:not(:first-child) { text-align: center; }\n"
            "td { padding: 12px 10px; color: #ccc; border-bottom: 1px solid rgba(255,255,255,0.05); }\n"
            "td:not(:first-child):not(:nth-child(2)) { text-align: center; }\n"
            "tr:hover { background: rgba(79,195,247,0.1); }\n"
            ".pos { font-weight: bold; color: #4fc3f7; }\n"
            ".pts { color: #fff; font-weight: bold; }\n"
            ".w { color: #4caf50; }\n"
            ".d { color: #ff9800; }\n"
            ".l { color: #f44336; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div class=\"container\">\n"
            "<div class=\"header\">\n"
            "<h1>" + escapeHTML(leagueName) + "</h1>\n"
            "<div class=\"season\">Season " + to_string(currentSeason) + " - League Table</div>\n"
            "</div>\n"
            "<div class=\"table-container\">\n"
            "<table>\n"
            "<thead>\n"
            "<tr><th>#</th><th>Team</th><th>P</th><th>W</th><th>D</th><th>L</th><th>GF</th><th>GA</th><th>GD</th><th>Pts</th></tr>\n"
            "</thead>\n"
            "<tbody>\n";
        
        for (size_t i = 0; i < sorted.size(); i++) {
            const Team& t = sorted[i].second;
            int gd = t.goalsScored - t.goalsConceded;
            string gdStr = (gd >= 0 ? "+" : "") + to_string(gd);
            html += "<tr>";
            html += "<td class=\"pos\">" + to_string(i + 1) + "</td>";
            html += "<td>" + escapeHTML(t.name) + "</td>";
            html += "<td>" + to_string(t.wins + t.draws + t.losses) + "</td>";
            html += "<td class=\"w\">" + to_string(t.wins) + "</td>";
            html += "<td class=\"d\">" + to_string(t.draws) + "</td>";
            html += "<td class=\"l\">" + to_string(t.losses) + "</td>";
            html += "<td>" + to_string(t.goalsScored) + "</td>";
            html += "<td>" + to_string(t.goalsConceded) + "</td>";
            html += "<td>" + gdStr + "</td>";
            html += "<td class=\"pts\">" + to_string(t.points) + "</td>";
            html += "</tr>\n";
        }
        
        html += 
            "</tbody>\n"
            "</table>\n"
            "</div>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
        
        ofstream f("standings.html");
        if (f.is_open()) { f << html; f.close(); }
        if (openInBrowser) openHTML("standings.html");
    }

    static void showMatchCommentary(const string& homeTeam, int homeGoals,
                                     const string& awayTeam, int awayGoals,
                                     const vector<CommentaryEvent>& events) {
        string html = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>" + escapeHTML(homeTeam) + " vs " + escapeHTML(awayTeam) + "</title>\n"
            "<style>\n"
            "* { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #0f0f23 0%, #1a1a3e 50%, #0f0f23 100%); min-height: 100vh; padding: 20px; }\n"
            ".container { max-width: 800px; margin: 0 auto; }\n"
            ".scoreboard { background: linear-gradient(180deg, #1e1e4a 0%, #15153a 100%); border-radius: 20px; padding: 30px; margin-bottom: 30px; text-align: center; border: 2px solid rgba(79,195,247,0.3); }\n"
            ".match-title { color: #666; font-size: 0.9em; text-transform: uppercase; letter-spacing: 2px; margin-bottom: 20px; }\n"
            ".teams { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }\n"
            ".team { flex: 1; text-align: center; }\n"
            ".team-name { color: #fff; font-size: 1.4em; font-weight: 600; }\n"
            ".score { display: flex; flex-direction: column; align-items: center; padding: 0 30px; }\n"
            ".score-display { font-size: 4em; font-weight: bold; color: #fff; text-shadow: 0 0 30px rgba(79,195,247,0.5); }\n"
            ".score-display.home { color: #4fc3f7; }\n"
            ".score-display.away { color: #ff9800; }\n"
            ".vs { color: #444; font-size: 1.5em; margin: 10px 0; }\n"
            ".timeline-header { color: #4fc3f7; font-size: 1.3em; text-align: center; margin-bottom: 25px; letter-spacing: 2px; }\n"
            ".timeline { position: relative; padding-left: 30px; }\n"
            ".timeline::before { content: ''; position: absolute; left: 10px; top: 0; bottom: 0; width: 2px; background: linear-gradient(180deg, #4fc3f7 0%, #ff9800 100%); }\n"
            ".event { position: relative; margin-bottom: 20px; padding: 15px 20px; background: rgba(255,255,255,0.03); border-radius: 10px; border-left: 3px solid; animation: slideIn 0.5s ease forwards; opacity: 0; transform: translateX(-20px); }\n"
            "@keyframes slideIn { to { opacity: 1; transform: translateX(0); } }\n"
            ".event.goal { border-color: #4caf50; background: rgba(76,175,80,0.1); }\n"
            ".event.goal .minute { background: #4caf50; }\n"
            ".event.yellow { border-color: #ffeb3b; background: rgba(255,235,59,0.1); }\n"
            ".event.yellow .minute { background: #ffeb3b; color: #000; }\n"
            ".event.red { border-color: #f44336; background: rgba(244,67,54,0.1); }\n"
            ".event.red .minute { background: #f44336; }\n"
            ".event.injury { border-color: #e91e63; background: rgba(233,30,99,0.1); }\n"
            ".event.injury .minute { background: #e91e63; }\n"
            ".event.sub { border-color: #2196f3; background: rgba(33,150,243,0.1); }\n"
            ".event.sub .minute { background: #2196f3; }\n"
            ".event.other { border-color: #9c27b0; background: rgba(156,39,176,0.1); }\n"
            ".event.other .minute { background: #9c27b0; }\n"
            ".minute { position: absolute; left: -45px; background: #4fc3f7; color: #000; padding: 5px 10px; border-radius: 5px; font-weight: bold; font-size: 0.85em; }\n"
            ".event-text { color: #ccc; font-size: 1em; line-height: 1.5; }\n"
            ".result-box { margin-top: 30px; padding: 20px; background: linear-gradient(90deg, rgba(79,195,247,0.2) 0%, rgba(255,152,0,0.2) 100%); border-radius: 10px; text-align: center; }\n"
            ".result-text { color: #fff; font-size: 1.5em; font-weight: 600; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div class=\"container\">\n"
            "<div class=\"scoreboard\">\n"
            "<div class=\"match-title\">MATCH COMMENTARY</div>\n"
            "<div class=\"teams\">\n"
            "<div class=\"team\"><div class=\"team-name\">" + escapeHTML(homeTeam) + "</div></div>\n"
            "<div class=\"score\">\n"
            "<div class=\"score-display home\">" + to_string(homeGoals) + "</div>\n"
            "<div class=\"vs\">-</div>\n"
            "<div class=\"score-display away\">" + to_string(awayGoals) + "</div>\n"
            "</div>\n"
            "<div class=\"team\"><div class=\"team-name\">" + escapeHTML(awayTeam) + "</div></div>\n"
            "</div>\n"
            "</div>\n"
            "<div class=\"timeline-header\">MATCH TIMELINE</div>\n"
            "<div class=\"timeline\">\n";
        
        vector<CommentaryEvent> sortedEvents = events;
        sort(sortedEvents.begin(), sortedEvents.end(), 
             [](const CommentaryEvent& a, const CommentaryEvent& b) { return a.minute < b.minute; });
        
        for (size_t i = 0; i < sortedEvents.size(); i++) {
            const CommentaryEvent& e = sortedEvents[i];
            string eventClass = getEventClass(e.text);
            html += "<div class=\"event " + eventClass + "\" style=\"animation-delay: " + to_string(i * 0.1) + "s\">";
            html += "<div class=\"minute\">" + to_string(e.minute) + "'</div>";
            html += "<div class=\"event-text\">" + escapeHTML(e.text) + "</div>";
            html += "</div>\n";
        }
        
        string resultText;
        if (homeGoals > awayGoals) resultText = escapeHTML(homeTeam) + " Wins!";
        else if (awayGoals > homeGoals) resultText = escapeHTML(awayTeam) + " Wins!";
        else resultText = "Draw!";
        
        html += 
            "</div>\n"
            "<div class=\"result-box\">\n"
            "<div class=\"result-text\">" + resultText + "</div>\n"
            "</div>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
        
        ofstream f("match.html");
        if (f.is_open()) { f << html; f.close(); }
        openHTML("match.html");
    }

    static void showTopScorers(const string& leagueName, int currentSeason, const vector<Team>& teams,
                               bool openInBrowser = true) {
        struct Scorer { string name, team; int goals, assists; };
        vector<Scorer> scorers;
        
        for (const auto& t : teams) {
            for (const auto& p : t.players) {
                if (p.goals > 0) {
                    Scorer s;
                    s.name = p.name;
                    s.team = t.name;
                    s.goals = p.goals;
                    s.assists = p.assists;
                    scorers.push_back(s);
                }
            }
        }
        
        sort(scorers.begin(), scorers.end(), [](const Scorer& a, const Scorer& b) { return a.goals > b.goals; });
        
        string html = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>Top Scorers</title>\n"
            "<style>\n"
            "* { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; padding: 20px; }\n"
            ".container { max-width: 700px; margin: 0 auto; }\n"
            ".header { text-align: center; margin-bottom: 30px; }\n"
            ".header h1 { color: #fff; font-size: 2em; font-weight: 300; }\n"
            ".header .season { color: #4fc3f7; }\n"
            ".scorer-list { display: flex; flex-direction: column; gap: 10px; }\n"
            ".scorer-card { background: rgba(255,255,255,0.05); border-radius: 10px; padding: 20px; display: flex; align-items: center; justify-content: space-between; border: 1px solid rgba(255,255,255,0.1); transition: all 0.3s; }\n"
            ".scorer-card:hover { background: rgba(79,195,247,0.1); transform: translateX(10px); }\n"
            ".scorer-card.top3 { background: linear-gradient(90deg, rgba(255,215,0,0.2) 0%, rgba(255,215,0,0.05) 100%); border-color: rgba(255,215,0,0.5); }\n"
            ".rank { font-size: 1.5em; font-weight: bold; color: #4fc3f7; width: 40px; }\n"
            ".scorer-card.top3 .rank { color: #ffd700; }\n"
            ".scorer-info { flex: 1; }\n"
            ".scorer-name { color: #fff; font-size: 1.2em; font-weight: 600; }\n"
            ".scorer-team { color: #888; font-size: 0.9em; }\n"
            ".scorer-stats { text-align: right; }\n"
            ".goals { font-size: 2em; font-weight: bold; color: #4fc3f7; }\n"
            ".scorer-card.top3 .goals { color: #ffd700; }\n"
            ".goals-label { color: #666; font-size: 0.8em; }\n"
            ".assists { color: #ff9800; font-size: 0.9em; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div class=\"container\">\n"
            "<div class=\"header\">\n"
            "<h1>Top Scorers</h1>\n"
            "<div class=\"season\">" + escapeHTML(leagueName) + " - Season " + to_string(currentSeason) + "</div>\n"
            "</div>\n"
            "<div class=\"scorer-list\">\n";
        
        int count = min(10, (int)scorers.size());
        for (int i = 0; i < count; i++) {
            string topClass = (i < 3) ? " top3" : "";
            html += "<div class=\"scorer-card" + topClass + "\">";
            html += "<div class=\"rank\">" + to_string(i + 1) + "</div>";
            html += "<div class=\"scorer-info\">";
            html += "<div class=\"scorer-name\">" + escapeHTML(scorers[i].name) + "</div>";
            html += "<div class=\"scorer-team\">" + escapeHTML(scorers[i].team) + "</div>";
            html += "</div>";
            html += "<div class=\"scorer-stats\">";
            html += "<div class=\"goals\">" + to_string(scorers[i].goals) + "</div>";
            html += "<div class=\"goals-label\">GOALS</div>";
            html += "<div class=\"assists\">" + to_string(scorers[i].assists) + " assists</div>";
            html += "</div>";
            html += "</div>\n";
        }
        
        html += 
            "</div>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
        
        ofstream f("scorers.html");
        if (f.is_open()) { f << html; f.close(); }
        if (openInBrowser) openHTML("scorers.html");
    }

    static void showInjuryReport(const string& leagueName, int currentSeason, const vector<Team>& teams,
                                 bool openInBrowser = true) {
        string html = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>Injury Report</title>\n"
            "<style>\n"
            "* { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; padding: 20px; }\n"
            ".container { max-width: 700px; margin: 0 auto; }\n"
            ".header { text-align: center; margin-bottom: 30px; }\n"
            ".header h1 { color: #e91e63; font-size: 2em; font-weight: 300; }\n"
            ".header .season { color: #4fc3f7; }\n"
            ".injury-list { display: flex; flex-direction: column; gap: 15px; }\n"
            ".injury-card { background: rgba(233,30,99,0.1); border-radius: 10px; padding: 20px; border-left: 4px solid #e91e63; display: flex; justify-content: space-between; align-items: center; }\n"
            ".player-info { flex: 1; }\n"
            ".player-name { color: #fff; font-size: 1.2em; font-weight: 600; }\n"
            ".team-name { color: #e91e63; font-size: 0.9em; }\n"
            ".recovery { text-align: right; }\n"
            ".games-left { font-size: 1.5em; font-weight: bold; color: #e91e63; }\n"
            ".games-label { color: #666; font-size: 0.8em; }\n"
            ".no-injuries { text-align: center; padding: 40px; color: #4caf50; font-size: 1.2em; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div class=\"container\">\n"
            "<div class=\"header\">\n"
            "<h1>Injury Report</h1>\n"
            "<div class=\"season\">" + escapeHTML(leagueName) + " - Season " + to_string(currentSeason) + "</div>\n"
            "</div>\n"
            "<div class=\"injury-list\">\n";
        
        bool any = false;
        for (const auto& t : teams) {
            for (const auto& p : t.players) {
                if (p.injured) {
                    any = true;
                    html += "<div class=\"injury-card\">";
                    html += "<div class=\"player-info\">";
                    html += "<div class=\"player-name\">" + escapeHTML(p.name) + "</div>";
                    html += "<div class=\"team-name\">" + escapeHTML(t.name) + "</div>";
                    html += "</div>";
                    html += "<div class=\"recovery\">";
                    html += "<div class=\"games-left\">" + to_string(p.injuryGamesLeft) + "</div>";
                    html += "<div class=\"games-label\">games left</div>";
                    html += "</div>";
                    html += "</div>\n";
                }
            }
        }
        
        if (!any) html += "<div class=\"no-injuries\">All players fit!</div>\n";
        
        html += 
            "</div>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
        
        ofstream f("injuries.html");
        if (f.is_open()) { f << html; f.close(); }
        if (openInBrowser) openHTML("injuries.html");
    }

    static void showSquadView(int teamIdx, const vector<Team>& teams,
                              const string& outputFile = "squad.html", bool openInBrowser = true) {
        if (teamIdx < 0 || teamIdx >= (int)teams.size()) return;
        const Team& t = teams[teamIdx];
        
        string html = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>" + escapeHTML(t.name) + " - Squad</title>\n"
            "<style>\n"
            "* { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; padding: 20px; }\n"
            ".container { max-width: 900px; margin: 0 auto; }\n"
            ".header { background: linear-gradient(90deg, #1e3a5f 0%, #0d2840 100%); border-radius: 15px; padding: 30px; margin-bottom: 25px; text-align: center; }\n"
            ".team-name { color: #fff; font-size: 2.5em; font-weight: 300; margin-bottom: 10px; }\n"
            ".team-info { color: #4fc3f7; font-size: 1em; }\n"
            ".section-title { color: #4fc3f7; font-size: 1.2em; margin: 25px 0 15px 0; padding-bottom: 10px; border-bottom: 1px solid rgba(79,195,247,0.3); }\n"
            ".player-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(250px, 1fr)); gap: 15px; }\n"
            ".player-card { background: rgba(255,255,255,0.05); border-radius: 10px; padding: 15px; border: 1px solid rgba(255,255,255,0.1); transition: all 0.3s; }\n"
            ".player-card:hover { background: rgba(79,195,247,0.1); transform: translateY(-3px); }\n"
            ".player-card.starter { border-left: 3px solid #4fc3f7; }\n"
            ".player-card.substitute { border-left: 3px solid #ff9800; opacity: 0.8; }\n"
            ".player-name { color: #fff; font-size: 1.1em; font-weight: 600; margin-bottom: 5px; }\n"
            ".player-position { display: inline-block; background: #4fc3f7; color: #000; padding: 3px 10px; border-radius: 5px; font-size: 0.8em; font-weight: 600; margin-bottom: 10px; }\n"
            ".player-stats { display: flex; gap: 15px; color: #888; font-size: 0.85em; }\n"
            ".stat { color: #4fc3f7; font-weight: 600; }\n"
            ".status-fit { color: #4caf50; }\n"
            ".status-injured { color: #e91e63; }\n"
            ".status-suspended { color: #f44336; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div class=\"container\">\n"
            "<div class=\"header\">\n"
            "<div class=\"team-name\">" + escapeHTML(t.name) + "</div>\n"
            "<div class=\"team-info\">" + escapeHTML(t.stadium) + " - " + escapeHTML(t.city) + "</div>\n"
            "<div class=\"team-info\">Strength: " + to_string(t.strength) + "/10 | Budget: $" + to_string(t.budget) + "M</div>\n"
            "</div>\n"
            "<div class=\"section-title\">Starting XI</div>\n"
            "<div class=\"player-grid\">\n";
        
        for (const auto& p : t.players) {
            if (p.isSubstitute) continue;
            string status = p.injured ? "status-injured" : (p.suspended ? "status-suspended" : "status-fit");
            string statusText = p.injured ? "INJURED" : (p.suspended ? "SUSPENDED" : "FIT");
            
            html += "<div class=\"player-card starter\">";
            html += "<div class=\"player-name\">" + escapeHTML(p.name) + "</div>";
            html += "<div class=\"player-position\">" + escapeHTML(p.position) + "</div>";
            html += "<div class=\"player-stats\">";
            html += "<span>G: <span class=\"stat\">" + to_string(p.goals) + "</span></span>";
            html += "<span>A: <span class=\"stat\">" + to_string(p.assists) + "</span></span>";
            html += "<span>YC: <span class=\"stat\">" + to_string(p.yellowCards) + "</span></span>";
            html += "<span>RC: <span class=\"stat\">" + to_string(p.redCards) + "</span></span>";
            html += "</div>";
            html += "<div class=\"player-stats\" style=\"margin-top:5px;\">";
            html += "<span class=\"" + status + "\">" + statusText + "</span>";
            html += "<span>Value: $" + to_string(p.marketValue) + "M</span>";
            html += "</div>";
            html += "</div>\n";
        }
        
        html += 
            "</div>\n"
            "<div class=\"section-title\">Substitutes</div>\n"
            "<div class=\"player-grid\">\n";
        
        for (const auto& p : t.players) {
            if (!p.isSubstitute) continue;
            string status = p.injured ? "status-injured" : (p.suspended ? "status-suspended" : "status-fit");
            string statusText = p.injured ? "INJURED" : (p.suspended ? "SUSPENDED" : "FIT");
            
            html += "<div class=\"player-card substitute\">";
            html += "<div class=\"player-name\">" + escapeHTML(p.name) + "</div>";
            html += "<div class=\"player-position\">" + escapeHTML(p.position) + "</div>";
            html += "<div class=\"player-stats\">";
            html += "<span>G: <span class=\"stat\">" + to_string(p.goals) + "</span></span>";
            html += "<span>A: <span class=\"stat\">" + to_string(p.assists) + "</span></span>";
            html += "<span>Value: $" + to_string(p.marketValue) + "M</span>";
            html += "</div>";
            html += "<div class=\"player-stats\" style=\"margin-top:5px;\">";
            html += "<span class=\"" + status + "\">" + statusText + "</span>";
            html += "</div>";
            html += "</div>\n";
        }
        
        html += 
            "</div>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
        
        ofstream f(outputFile);
        if (f.is_open()) { f << html; f.close(); }
        if (openInBrowser) openHTML(outputFile);
    }

    static void showSquadHub(const vector<Team>& teams, bool openInBrowser = true) {
        for (int i = 0; i < (int)teams.size(); i++) {
            showSquadView(i, teams, "squad_" + to_string(i + 1) + ".html", false);
        }

        stringstream html;
        html << R"HUB(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Squad Viewer</title>
    <style>
        * { box-sizing: border-box; }
        body {
            margin: 0;
            min-height: 100vh;
            padding: 24px;
            font-family: 'Trebuchet MS', Verdana, sans-serif;
            color: #eef7ff;
            background:
                radial-gradient(circle at top left, rgba(116, 210, 255, 0.15), transparent 32%),
                linear-gradient(145deg, #07121b 0%, #0d2231 58%, #081723 100%);
        }
        .container {
            max-width: 980px;
            margin: 0 auto;
        }
        .hero {
            padding: 28px;
            border-radius: 26px;
            background: rgba(8, 20, 31, 0.82);
            border: 1px solid rgba(116, 210, 255, 0.16);
            box-shadow: 0 24px 70px rgba(0, 0, 0, 0.35);
        }
        .hero h1 {
            margin: 0 0 10px;
            font-size: clamp(2rem, 4vw, 3rem);
        }
        .hero p {
            margin: 0;
            color: #93a8b7;
            line-height: 1.6;
        }
        .team-grid {
            margin-top: 18px;
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 14px;
        }
        .team-card {
            display: block;
            padding: 20px;
            border-radius: 22px;
            color: inherit;
            text-decoration: none;
            background: rgba(8, 20, 31, 0.76);
            border: 1px solid rgba(116, 210, 255, 0.12);
            transition: transform 0.22s ease, border-color 0.22s ease;
        }
        .team-card:hover {
            transform: translateY(-4px);
            border-color: rgba(116, 210, 255, 0.34);
        }
        .team-name {
            font-size: 1.18rem;
            font-weight: bold;
            margin-bottom: 8px;
        }
        .team-copy {
            color: #93a8b7;
            line-height: 1.5;
            min-height: 48px;
        }
        .team-meta {
            margin-top: 16px;
            display: flex;
            justify-content: space-between;
            gap: 10px;
            color: #74d2ff;
            font-size: 0.88rem;
        }
        .back-link {
            display: inline-block;
            margin-top: 20px;
            color: #74d2ff;
            text-decoration: none;
        }
    </style>
</head>
<body>
    <div class="container">
        <section class="hero">
            <h1>Squad Viewer</h1>
            <p>Choose a club to open its squad page. Each page is generated from the latest simulator data.</p>
            <div class="team-grid">)HUB";

        for (int i = 0; i < (int)teams.size(); i++) {
            html << R"HUB(
                <a class="team-card" href=")HUB" << "squad_" << (i + 1) << R"HUB(.html">
                    <div class="team-name">)HUB" << escapeHTML(teams[i].name) << R"HUB(</div>
                    <div class="team-copy">)HUB" << escapeHTML(teams[i].stadium) << " - "
                 << escapeHTML(teams[i].city) << R"HUB(</div>
                    <div class="team-meta">
                        <span>Strength )HUB" << teams[i].strength << R"HUB(/10</span>
                        <span>Budget $)HUB" << teams[i].budget << R"HUB(M</span>
                    </div>
                </a>)HUB";
        }

        html << R"HUB(
            </div>
            <a class="back-link" href="menu.html">Back to Dashboard</a>
        </section>
    </div>
</body>
</html>)HUB";

        ofstream f("squads.html");
        if (f.is_open()) { f << html.str(); f.close(); }
        if (openInBrowser) openHTML("squads.html");
    }

    static void showHistory(const vector<SeasonRecord>& history, bool openInBrowser = true) {
        string html = 
            "<!DOCTYPE html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "<meta charset=\"UTF-8\">\n"
            "<title>Season History</title>\n"
            "<style>\n"
            "* { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "body { font-family: 'Segoe UI', sans-serif; background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%); min-height: 100vh; padding: 20px; }\n"
            ".container { max-width: 800px; margin: 0 auto; }\n"
            ".header { text-align: center; margin-bottom: 30px; }\n"
            ".header h1 { color: #fff; font-size: 2em; font-weight: 300; }\n"
            ".history-list { display: flex; flex-direction: column; gap: 15px; }\n"
            ".season-card { background: rgba(255,255,255,0.05); border-radius: 15px; padding: 25px; border: 1px solid rgba(255,255,255,0.1); }\n"
            ".season-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px; padding-bottom: 15px; border-bottom: 1px solid rgba(255,255,255,0.1); }\n"
            ".season-num { color: #4fc3f7; font-size: 1.5em; font-weight: bold; }\n"
            ".winner { color: #ffd700; font-size: 1.3em; font-weight: 600; }\n"
            ".details { display: flex; justify-content: space-around; }\n"
            ".detail { text-align: center; }\n"
            ".detail-label { color: #666; font-size: 0.8em; text-transform: uppercase; margin-bottom: 5px; }\n"
            ".detail-value { color: #fff; font-size: 1.1em; }\n"
            ".cup-value { color: #4fc3f7; }\n"
            ".scorer-value { color: #ff9800; }\n"
            "</style>\n"
            "</head>\n"
            "<body>\n"
            "<div class=\"container\">\n"
            "<div class=\"header\">\n"
            "<h1>Season History</h1>\n"
            "</div>\n"
            "<div class=\"history-list\">\n";
        
        if (history.empty()) {
            html += "<div style=\"text-align:center;padding:40px;color:#666;\">No seasons completed yet.</div>\n";
        } else {
            for (auto it = history.rbegin(); it != history.rend(); ++it) {
                html += "<div class=\"season-card\">";
                html += "<div class=\"season-header\">";
                html += "<div class=\"season-num\">Season " + to_string(it->season) + "</div>";
                html += "<div class=\"winner\">" + escapeHTML(it->leagueWinner) + "</div>";
                html += "</div>";
                html += "<div class=\"details\">";
                html += "<div class=\"detail\">";
                html += "<div class=\"detail-label\">Cup Winner</div>";
                html += "<div class=\"detail-value cup-value\">" + escapeHTML(it->cupWinner) + "</div>";
                html += "</div>";
                html += "<div class=\"detail\">";
                html += "<div class=\"detail-label\">Top Scorer</div>";
                html += "<div class=\"detail-value scorer-value\">" + escapeHTML(it->topScorer) + " (" + to_string(it->topScorerGoals) + ")</div>";
                html += "</div>";
                html += "</div>";
                html += "</div>\n";
            }
        }
        
        html += 
            "</div>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
        
        ofstream f("history.html");
        if (f.is_open()) { f << html; f.close(); }
        if (openInBrowser) openHTML("history.html");
    }
};

// -----------------------------------------
//  GLOBALS
// -----------------------------------------
vector<Team> teams;
vector<Match> leagueSchedule;
vector<SeasonRecord> history;
vector<int> cupPool;

int currentSeason = 1;
string leagueName = "Custom League";
bool fastDisplay = false;
bool backToMainMenu = false;
bool leagueFinishedThisSeason = false;
bool cupFinishedThisSeason = false;
bool cupStartedThisSeason = false;
int cupRoundNumber = 1;
string cupWinnerThisSeason = "TBD";

ActionReport lastLeagueAction;
ActionReport lastCupAction;
string webNotice = "Choose a league to begin.";
string webNoticeKind = "info";
string currentServerUrl = "";
const string SAVE_FILE_NAME = "league-simulator-save.txt";
bool startupResumePromptDismissed = false;
mutex stateMutex;

string getLeagueWinner();
string getTopScorer(int& goals);

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

int findSeasonRecordIndex(int season) {
    for (int i = 0; i < (int)history.size(); i++) {
        if (history[i].season == season) return i;
    }
    return -1;
}

void syncSeasonRecord(bool createIfNeeded = true) {
    if (!createIfNeeded && !seasonAlreadySaved()) return;

    int topGoals = 0;
    string topScorer = getTopScorer(topGoals);
    if (topGoals == 0) topScorer = "None";

    SeasonRecord record;
    record.season = currentSeason;
    record.leagueWinner = leagueFinishedThisSeason ? getLeagueWinner() : "TBD";
    record.cupWinner = cupFinishedThisSeason ? cupWinnerThisSeason : "TBD";
    record.topScorer = topScorer;
    record.topScorerGoals = topGoals;

    int idx = findSeasonRecordIndex(currentSeason);
    if (idx >= 0) history[idx] = record;
    else if (createIfNeeded) history.push_back(record);
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
    cupStartedThisSeason = false;
    cupRoundNumber = 1;
    cupWinnerThisSeason = "TBD";
    cupPool.clear();
    backToMainMenu = false;
    lastLeagueAction = {};
    lastCupAction = {};
}

void resetCurrentSessionState() {
    teams.clear();
    history.clear();
    leagueSchedule.clear();
    cupPool.clear();
    currentSeason = 1;
    leagueName = "Custom League";
    backToMainMenu = false;
    leagueFinishedThisSeason = false;
    cupFinishedThisSeason = false;
    cupStartedThisSeason = false;
    cupRoundNumber = 1;
    cupWinnerThisSeason = "TBD";
    lastLeagueAction = {};
    lastCupAction = {};
    startupResumePromptDismissed = true;
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
void updateInjuries(Team& t, bool verboseConsole = true) {
    for (auto& p : t.players) {
        if (p.injured && p.injuryGamesLeft > 0) {
            p.injuryGamesLeft--;
            if (p.injuryGamesLeft == 0) {
                p.injured = false;
                if (verboseConsole) {
                    cout << GREEN;
                    typeLine("  + " + p.name + " (" + t.name + ") has recovered!");
                    cout << RESET;
                }
            }
        }
    }
}

// -----------------------------------------
//  SIMULATE ONE MATCH
// -----------------------------------------
MatchSimulationResult simulateMatchDetailed(int homeIdx, int awayIdx, bool isCup, bool verboseConsole) {
    Team& home = teams[homeIdx];
    Team& away = teams[awayIdx];

    updateInjuries(home, verboseConsole);
    updateInjuries(away, verboseConsole);

    if (verboseConsole) {
        printDivider('-');
        cout << BOLD;
        typeLine("  " + home.name + " (Home)  vs  " + away.name + " (Away)");
        cout << RESET;
        printDivider('-');
    }

    vector<CommentaryEvent> events;
    addEvent(events, 0, "Kick-off - " + home.name + " vs " + away.name, CYAN);

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
    int regularHomeGoals = max(0, rng(0, 2) + (homeEff >= 8 ? 1 : 0) + (homeEff >= 10 ? 1 : 0));
    int regularAwayGoals = max(0, rng(0, 2) + (awayEff >= 8 ? 1 : 0) + (awayEff >= 10 ? 1 : 0));
    int extraHomeGoals = 0;
    int extraAwayGoals = 0;
    bool wentExtraTime = false;

    if (isCup && regularHomeGoals == regularAwayGoals) {
        wentExtraTime = true;
        addEvent(events, 90, "End of normal time - level scores, extra time ahead.", YELLOW);
        addEvent(events, 91, "Extra time begins!", YELLOW);
        if (rng(1, 2) == 1) extraHomeGoals++;
        else extraAwayGoals++;
    }

    int homeGoals = regularHomeGoals + extraHomeGoals;
    int awayGoals = regularAwayGoals + extraAwayGoals;

    auto assignGoals = [&](int count, Team& team, vector<int>& str, vector<int>& any, const string& tname, int minMinute, int maxMinute) {
        for (int g = 0; g < count; g++) {
            int minute = rng(minMinute, maxMinute);
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

    assignGoals(regularHomeGoals, home, homeStr, homeAvail, home.name, 1, 90);
    assignGoals(regularAwayGoals, away, awayStr, awayAvail, away.name, 1, 90);
    assignGoals(extraHomeGoals, home, homeStr, homeAvail, home.name, 91, 120);
    assignGoals(extraAwayGoals, away, awayStr, awayAvail, away.name, 91, 120);

    auto processEvents = [&](Team& team, vector<int>& avail) {
        for (int idx : avail) {
            Player& p = team.players[idx];
            int minute = rng(1, wentExtraTime ? 120 : 90);
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

    string resultLine = home.name + " " + to_string(homeGoals) + " - " + to_string(awayGoals) + " " + away.name;
    string winnerLine;

    if (verboseConsole) {
        cout << BOLD;
        typeLine("\n  RESULT: " + resultLine);
        cout << RESET;
    }

    if (!isCup) {
        home.goalsScored += homeGoals;
        home.goalsConceded += awayGoals;
        away.goalsScored += awayGoals;
        away.goalsConceded += homeGoals;

        if (homeGoals > awayGoals) {
            home.wins++;
            home.points += 3;
            away.losses++;
            winnerLine = home.name + " wins!";
            if (verboseConsole) {
                cout << GREEN;
                typeLine("  >> " + winnerLine);
                cout << RESET;
            }
        }
        else if (awayGoals > homeGoals) {
            away.wins++;
            away.points += 3;
            home.losses++;
            winnerLine = away.name + " wins!";
            if (verboseConsole) {
                cout << GREEN;
                typeLine("  >> " + winnerLine);
                cout << RESET;
            }
        }
        else {
            home.draws++;
            home.points++;
            away.draws++;
            away.points++;
            winnerLine = "Draw!";
            if (verboseConsole) {
                cout << YELLOW;
                typeLine("  >> Draw!");
                cout << RESET;
            }
        }
    }
    else {
        int winner = (homeGoals > awayGoals) ? homeIdx : awayIdx;
        winnerLine = teams[winner].name + " advances!";
        if (verboseConsole) {
            cout << GREEN;
            typeLine("  >> " + winnerLine);
            cout << RESET;
        }
    }

    addEvent(events, 45, "Half-time whistle.", CYAN);
    addEvent(events, wentExtraTime ? 120 : 90, "Full time - " + resultLine, CYAN);
    sort(events.begin(), events.end(), [](const CommentaryEvent& a, const CommentaryEvent& b) {
        if (a.minute != b.minute) return a.minute < b.minute;
        return a.text < b.text;
    });

    if (verboseConsole) {
        printChronologicalEvents(events);
    }

    for (auto& p : home.players) p.suspended = false;
    for (auto& p : away.players) p.suspended = false;

    MatchSimulationResult result;
    result.match = { homeIdx, awayIdx, homeGoals, awayGoals, true, isCup };
    result.homeName = home.name;
    result.awayName = away.name;
    result.resultLine = resultLine;
    result.winnerLine = winnerLine;
    result.events = events;
    return result;
}

Match simulateOneMatch(int homeIdx, int awayIdx, bool isCup) {
    return simulateMatchDetailed(homeIdx, awayIdx, isCup, true).match;
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

        cupWinnerThisSeason = teams[pool[0]].name;
        syncSeasonRecord();
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

    syncSeasonRecord();
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
    cupStartedThisSeason = false;
    cupRoundNumber = 1;
    cupWinnerThisSeason = "TBD";
    cupPool.clear();

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
            << "  11. Open HTML Dashboard (Browser)\n"
            << "  12. Exit\n\n"
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
            printHeader("OPEN HTML DASHBOARD");
            cout << "\n  1. Main Dashboard\n"
                << "  2. League Table\n"
                << "  3. Top Scorers\n"
                << "  4. Injury Report\n"
                << "  5. Squad Viewer\n"
                << "  6. Season History\n"
                << "  0. Back\n\n"
                << "  Choice: ";
            
            int hch;
            cin >> hch;
            cin.ignore(1000, '\n');
            
            if (hch == 1) {
                HTMLGenerator::showMainMenu(leagueName, currentSeason, teams, history,
                                            leagueFinishedThisSeason, cupFinishedThisSeason);
                cout << CYAN;
                typeLine("\n  Opened HTML dashboard in browser!");
                typeLine("  Use browser cards for reports and the console for gameplay actions.");
                cout << RESET;
            }
            else if (hch == 2) {
                HTMLGenerator::showStandings(leagueName, currentSeason, teams);
                cout << CYAN;
                typeLine("\n  Opened standings in browser!");
                cout << RESET;
            }
            else if (hch == 3) {
                HTMLGenerator::showTopScorers(leagueName, currentSeason, teams);
                cout << CYAN;
                typeLine("\n  Opened top scorers in browser!");
                cout << RESET;
            }
            else if (hch == 4) {
                HTMLGenerator::showInjuryReport(leagueName, currentSeason, teams);
                cout << CYAN;
                typeLine("\n  Opened injury report in browser!");
                cout << RESET;
            }
            else if (hch == 5) {
                HTMLGenerator::showSquadHub(teams);
                cout << CYAN;
                typeLine("\n  Opened squad viewer in browser!");
                cout << RESET;
            }
            else if (hch == 6) {
                HTMLGenerator::showHistory(history);
                cout << CYAN;
                typeLine("\n  Opened history in browser!");
                cout << RESET;
            }
        }
        else if (c == 12) {
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
//  WEB APP HELPERS
// -----------------------------------------
string escapeHtml(const string& s) {
    string r;
    for (char c : s) {
        if (c == '<') r += "&lt;";
        else if (c == '>') r += "&gt;";
        else if (c == '&') r += "&amp;";
        else if (c == '"') r += "&quot;";
        else if (c == '\'') r += "&#39;";
        else r += c;
    }
    return r;
}

string trimCopy(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

int toInt(const string& s, int fallback = 0) {
    try {
        size_t used = 0;
        int value = stoi(s, &used);
        if (used != s.size()) return fallback;
        return value;
    }
    catch (...) {
        return fallback;
    }
}

string paramValue(const httplib::Request& req, const string& key, const string& fallback = "") {
    if (req.has_param(key)) return req.get_param_value(key);
    return fallback;
}

void setNotice(const string& message, const string& kind = "info") {
    webNotice = message;
    webNoticeKind = kind;
}

string sanitizeReturnPath(const string& returnTo, const string& fallback = "/settings") {
    string cleaned = trimCopy(returnTo);
    if (cleaned.empty() || cleaned[0] != '/' || (cleaned.size() > 1 && cleaned[1] == '/')) {
        return fallback;
    }
    return cleaned;
}

string currentTimestampText() {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    stringstream ss;
    ss << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void applySaveSnapshot(const SaveSnapshot& snapshot) {
    teams = snapshot.teams;
    leagueSchedule = snapshot.leagueSchedule;
    history = snapshot.history;
    cupPool = snapshot.cupPool;
    currentSeason = max(1, snapshot.currentSeason);
    leagueName = snapshot.leagueName.empty() ? "Custom League" : snapshot.leagueName;
    fastDisplay = snapshot.fastDisplay;
    backToMainMenu = snapshot.backToMainMenu;
    leagueFinishedThisSeason = snapshot.leagueFinishedThisSeason;
    cupFinishedThisSeason = snapshot.cupFinishedThisSeason;
    cupStartedThisSeason = snapshot.cupStartedThisSeason;
    cupRoundNumber = max(1, snapshot.cupRoundNumber);
    cupWinnerThisSeason = snapshot.cupWinnerThisSeason.empty() ? "TBD" : snapshot.cupWinnerThisSeason;
    lastLeagueAction = {};
    lastCupAction = {};
}

SaveFileSummary readSaveFileSummary() {
    SaveFileSummary summary;
    ifstream existing(SAVE_FILE_NAME);
    if (!existing) {
        ofstream createSlot(SAVE_FILE_NAME);
        if (createSlot) {
            createSlot << "LEAGUE_SIMULATOR_SAVE_V1\n";
            createSlot << "STATE EMPTY\n";
            createSlot << "END\n";
        }
    }

    ifstream in(SAVE_FILE_NAME);
    if (!in) return summary;

    summary.exists = true;

    string line;
    if (!getline(in, line) || trimCopy(line) != "LEAGUE_SIMULATOR_SAVE_V1") return summary;

    if (!getline(in, line)) return summary;
    {
        istringstream stateStream(line);
        string tag;
        string state;
        if ((stateStream >> tag >> state) && tag == "STATE") {
            if (state == "EMPTY") return summary;
            if (state != "OCCUPIED") return summary;
            if (!getline(in, line)) return summary;
        }
    }

    {
        istringstream iss(line);
        string tag;
        if (!(iss >> tag) || tag != "SAVED_AT" || !(iss >> quoted(summary.savedAt))) return summary;
    }

    {
        if (!getline(in, line)) return summary;
        istringstream iss(line);
        string tag;
        int fastFlag = 0;
        int backFlag = 0;
        int leagueFinishedFlag = 0;
        int cupFinishedFlag = 0;
        int cupStartedFlag = 0;
        int cupRound = 0;
        string cupWinner;
        if (!(iss >> tag) || tag != "META") return summary;
        if (!(iss >> quoted(summary.leagueName) >> summary.currentSeason >> fastFlag >> backFlag
            >> leagueFinishedFlag >> cupFinishedFlag >> cupStartedFlag >> cupRound >> quoted(cupWinner))) {
            return summary;
        }
    }

    summary.valid = true;
    return summary;
}

bool resetSaveFile(string& message) {
    ofstream out(SAVE_FILE_NAME);
    if (!out) {
        message = "The save slot could not be reset.";
        return false;
    }

    out << "LEAGUE_SIMULATOR_SAVE_V1\n";
    out << "STATE EMPTY\n";
    out << "END\n";

    if (!out.good()) {
        message = "The save slot could not be reset.";
        return false;
    }

    message = "The save slot has been cleared.";
    return true;
}

bool saveProgressToFile(string& message) {
    if (teams.empty()) {
        message = "Start a league first, then save your progress from Settings.";
        return false;
    }

    if (leagueFinishedThisSeason || cupFinishedThisSeason) syncSeasonRecord();

    SaveSnapshot snapshot;
    snapshot.savedAt = currentTimestampText();
    snapshot.teams = teams;
    snapshot.leagueSchedule = leagueSchedule;
    snapshot.history = history;
    snapshot.cupPool = cupPool;
    snapshot.currentSeason = currentSeason;
    snapshot.leagueName = leagueName;
    snapshot.fastDisplay = fastDisplay;
    snapshot.backToMainMenu = backToMainMenu;
    snapshot.leagueFinishedThisSeason = leagueFinishedThisSeason;
    snapshot.cupFinishedThisSeason = cupFinishedThisSeason;
    snapshot.cupStartedThisSeason = cupStartedThisSeason;
    snapshot.cupRoundNumber = cupRoundNumber;
    snapshot.cupWinnerThisSeason = cupWinnerThisSeason;

    ofstream out(SAVE_FILE_NAME);
    if (!out) {
        message = "Could not create the save file.";
        return false;
    }

    out << "LEAGUE_SIMULATOR_SAVE_V1\n";
    out << "STATE OCCUPIED\n";
    out << "SAVED_AT " << quoted(snapshot.savedAt) << "\n";
    out << "META "
        << quoted(snapshot.leagueName) << " "
        << snapshot.currentSeason << " "
        << (snapshot.fastDisplay ? 1 : 0) << " "
        << (snapshot.backToMainMenu ? 1 : 0) << " "
        << (snapshot.leagueFinishedThisSeason ? 1 : 0) << " "
        << (snapshot.cupFinishedThisSeason ? 1 : 0) << " "
        << (snapshot.cupStartedThisSeason ? 1 : 0) << " "
        << snapshot.cupRoundNumber << " "
        << quoted(snapshot.cupWinnerThisSeason) << "\n";

    out << "TEAMS " << snapshot.teams.size() << "\n";
    for (const auto& team : snapshot.teams) {
        out << "TEAM "
            << quoted(team.name) << " "
            << quoted(team.stadium) << " "
            << quoted(team.city) << " "
            << team.strength << " "
            << team.points << " "
            << team.wins << " "
            << team.draws << " "
            << team.losses << " "
            << team.goalsScored << " "
            << team.goalsConceded << " "
            << team.budget << " "
            << team.players.size() << "\n";

        for (const auto& player : team.players) {
            out << "PLAYER "
                << quoted(player.name) << " "
                << quoted(player.position) << " "
                << player.goals << " "
                << player.assists << " "
                << player.yellowCards << " "
                << player.redCards << " "
                << (player.suspended ? 1 : 0) << " "
                << (player.injured ? 1 : 0) << " "
                << player.injuryGamesLeft << " "
                << player.totalInjuries << " "
                << player.totalRedCards << " "
                << player.marketValue << " "
                << (player.listedForSale ? 1 : 0) << " "
                << player.askingPrice << " "
                << (player.isSubstitute ? 1 : 0) << "\n";
        }
    }

    out << "LEAGUE_SCHEDULE " << snapshot.leagueSchedule.size() << "\n";
    for (const auto& match : snapshot.leagueSchedule) {
        out << "MATCH "
            << match.homeIdx << " "
            << match.awayIdx << " "
            << match.homeGoals << " "
            << match.awayGoals << " "
            << (match.played ? 1 : 0) << " "
            << (match.isCup ? 1 : 0) << "\n";
    }

    out << "HISTORY " << snapshot.history.size() << "\n";
    for (const auto& record : snapshot.history) {
        out << "RECORD "
            << record.season << " "
            << quoted(record.leagueWinner) << " "
            << quoted(record.cupWinner) << " "
            << quoted(record.topScorer) << " "
            << record.topScorerGoals << "\n";
    }

    out << "CUP_POOL " << snapshot.cupPool.size();
    for (int teamIdx : snapshot.cupPool) out << " " << teamIdx;
    out << "\nEND\n";

    if (!out.good()) {
        message = "The save file could not be written completely.";
        return false;
    }

    message = "Progress saved for " + leagueName + ", Season " + to_string(currentSeason) + ".";
    return true;
}

bool loadProgressFromFile(string& message) {
    readSaveFileSummary();
    ifstream in(SAVE_FILE_NAME);
    if (!in) {
        message = "No saved progress was found.";
        return false;
    }

    SaveSnapshot snapshot;
    string line;
    string tag;

    if (!getline(in, line) || trimCopy(line) != "LEAGUE_SIMULATOR_SAVE_V1") {
        message = "The saved progress file is not valid.";
        return false;
    }

    if (!getline(in, line)) {
        message = "The save slot is empty.";
        return false;
    }
    {
        istringstream stateStream(line);
        string stateTag;
        string state;
        if ((stateStream >> stateTag >> state) && stateTag == "STATE") {
            if (state == "EMPTY") {
                message = "The save slot is empty.";
                return false;
            }
            if (state != "OCCUPIED") {
                message = "The saved progress file is not valid.";
                return false;
            }
            if (!getline(in, line)) {
                message = "The saved progress file is incomplete.";
                return false;
            }
        }
    }

    {
        istringstream iss(line);
        if (!(iss >> tag) || tag != "SAVED_AT" || !(iss >> quoted(snapshot.savedAt))) {
            message = "The saved progress timestamp could not be read.";
            return false;
        }
    }

    {
        if (!getline(in, line)) {
            message = "The saved progress file is missing league details.";
            return false;
        }
        istringstream iss(line);
        int fastFlag = 0;
        int backFlag = 0;
        int leagueFinishedFlag = 0;
        int cupFinishedFlag = 0;
        int cupStartedFlag = 0;
        if (!(iss >> tag) || tag != "META"
            || !(iss >> quoted(snapshot.leagueName) >> snapshot.currentSeason >> fastFlag >> backFlag
                >> leagueFinishedFlag >> cupFinishedFlag >> cupStartedFlag >> snapshot.cupRoundNumber
                >> quoted(snapshot.cupWinnerThisSeason))) {
            message = "The saved progress details could not be read.";
            return false;
        }
        snapshot.fastDisplay = fastFlag != 0;
        snapshot.backToMainMenu = backFlag != 0;
        snapshot.leagueFinishedThisSeason = leagueFinishedFlag != 0;
        snapshot.cupFinishedThisSeason = cupFinishedFlag != 0;
        snapshot.cupStartedThisSeason = cupStartedFlag != 0;
    }

    int teamCount = 0;
    {
        if (!getline(in, line)) {
            message = "The saved progress file is missing team data.";
            return false;
        }
        istringstream iss(line);
        if (!(iss >> tag >> teamCount) || tag != "TEAMS" || teamCount < 0 || teamCount > 32) {
            message = "The saved team list could not be read.";
            return false;
        }
    }

    snapshot.teams.clear();
    for (int i = 0; i < teamCount; i++) {
        if (!getline(in, line)) {
            message = "A saved team entry is missing.";
            return false;
        }

        Team team;
        int playerCount = 0;
        istringstream teamStream(line);
        if (!(teamStream >> tag) || tag != "TEAM"
            || !(teamStream >> quoted(team.name) >> quoted(team.stadium) >> quoted(team.city)
                >> team.strength >> team.points >> team.wins >> team.draws >> team.losses
                >> team.goalsScored >> team.goalsConceded >> team.budget >> playerCount)
            || playerCount < 0 || playerCount > 40) {
            message = "A saved team entry could not be read.";
            return false;
        }

        team.players.clear();
        for (int p = 0; p < playerCount; p++) {
            if (!getline(in, line)) {
                message = "A saved player entry is missing.";
                return false;
            }

            Player player;
            int suspendedFlag = 0;
            int injuredFlag = 0;
            int listedFlag = 0;
            int substituteFlag = 0;
            istringstream playerStream(line);
            if (!(playerStream >> tag) || tag != "PLAYER"
                || !(playerStream >> quoted(player.name) >> quoted(player.position)
                    >> player.goals >> player.assists >> player.yellowCards >> player.redCards
                    >> suspendedFlag >> injuredFlag >> player.injuryGamesLeft >> player.totalInjuries
                    >> player.totalRedCards >> player.marketValue >> listedFlag >> player.askingPrice
                    >> substituteFlag)) {
                message = "A saved player entry could not be read.";
                return false;
            }

            player.suspended = suspendedFlag != 0;
            player.injured = injuredFlag != 0;
            player.listedForSale = listedFlag != 0;
            player.isSubstitute = substituteFlag != 0;
            team.players.push_back(player);
        }

        snapshot.teams.push_back(team);
    }

    int scheduleCount = 0;
    {
        if (!getline(in, line)) {
            message = "The saved league schedule is missing.";
            return false;
        }
        istringstream iss(line);
        if (!(iss >> tag >> scheduleCount) || tag != "LEAGUE_SCHEDULE" || scheduleCount < 0 || scheduleCount > 1000) {
            message = "The saved league schedule could not be read.";
            return false;
        }
    }

    snapshot.leagueSchedule.clear();
    for (int i = 0; i < scheduleCount; i++) {
        if (!getline(in, line)) {
            message = "A saved league match entry is missing.";
            return false;
        }

        Match match;
        int playedFlag = 0;
        int cupFlag = 0;
        istringstream matchStream(line);
        if (!(matchStream >> tag) || tag != "MATCH"
            || !(matchStream >> match.homeIdx >> match.awayIdx >> match.homeGoals >> match.awayGoals >> playedFlag >> cupFlag)) {
            message = "A saved league match entry could not be read.";
            return false;
        }

        match.played = playedFlag != 0;
        match.isCup = cupFlag != 0;
        snapshot.leagueSchedule.push_back(match);
    }

    int historyCount = 0;
    {
        if (!getline(in, line)) {
            message = "The saved history section is missing.";
            return false;
        }
        istringstream iss(line);
        if (!(iss >> tag >> historyCount) || tag != "HISTORY" || historyCount < 0 || historyCount > 200) {
            message = "The saved history section could not be read.";
            return false;
        }
    }

    snapshot.history.clear();
    for (int i = 0; i < historyCount; i++) {
        if (!getline(in, line)) {
            message = "A saved history record is missing.";
            return false;
        }

        SeasonRecord record;
        istringstream recordStream(line);
        if (!(recordStream >> tag) || tag != "RECORD"
            || !(recordStream >> record.season >> quoted(record.leagueWinner) >> quoted(record.cupWinner)
                >> quoted(record.topScorer) >> record.topScorerGoals)) {
            message = "A saved history record could not be read.";
            return false;
        }
        snapshot.history.push_back(record);
    }

    {
        if (!getline(in, line)) {
            message = "The saved cup section is missing.";
            return false;
        }
        istringstream poolStream(line);
        int poolCount = 0;
        if (!(poolStream >> tag >> poolCount) || tag != "CUP_POOL" || poolCount < 0 || poolCount > 64) {
            message = "The saved cup section could not be read.";
            return false;
        }
        snapshot.cupPool.clear();
        for (int i = 0; i < poolCount; i++) {
            int teamIdx = 0;
            if (!(poolStream >> teamIdx)) {
                message = "The saved cup team list could not be read.";
                return false;
            }
            snapshot.cupPool.push_back(teamIdx);
        }
    }

    if (!getline(in, line) || trimCopy(line) != "END") {
        message = "The saved progress file did not finish correctly.";
        return false;
    }

    applySaveSnapshot(snapshot);
    startupResumePromptDismissed = true;
    message = "Loaded saved progress for " + leagueName + ", Season " + to_string(currentSeason) + ".";
    return true;
}

int defaultMarketValueForIndex(int index) {
    static const int values[] = { 5,8,8,8,8,8,12,12,12,20,20,4,4,4,4,4 };
    if (index >= 0 && index < 16) return values[index];
    return 4;
}

string defaultPositionForIndex(int index) {
    static const vector<string> positions = {
        "GK","DEF","DEF","DEF","DEF","MID","MID","MID","STR","STR","STR",
        "GK","DEF","MID","MID","STR"
    };
    if (index >= 0 && index < (int)positions.size()) return positions[index];
    return "MID";
}

vector<int> sortedTeamOrder() {
    vector<int> order;
    for (int i = 0; i < (int)teams.size(); i++) order.push_back(i);

    sort(order.begin(), order.end(), [](int a, int b) {
        if (teams[a].points != teams[b].points) return teams[a].points > teams[b].points;
        int gda = teams[a].goalsScored - teams[a].goalsConceded;
        int gdb = teams[b].goalsScored - teams[b].goalsConceded;
        if (gda != gdb) return gda > gdb;
        return teams[a].goalsScored > teams[b].goalsScored;
    });

    return order;
}

int leagueMatchesPerDay() {
    return max(1, (int)teams.size() / 2);
}

int nextUnplayedLeagueMatchIndex() {
    for (int i = 0; i < (int)leagueSchedule.size(); i++) {
        if (!leagueSchedule[i].played) return i;
    }
    return -1;
}

int currentLeagueMatchdayNumber() {
    if (leagueSchedule.empty()) return 1;
    int nextIdx = nextUnplayedLeagueMatchIndex();
    if (nextIdx < 0) return max(1, (int)leagueSchedule.size() / leagueMatchesPerDay());
    return nextIdx / leagueMatchesPerDay() + 1;
}

void prepareLoadedLeagueState() {
    resetSeasonStats();
    if (teams.empty()) return;

    for (auto& team : teams) {
        for (auto& player : team.players) {
            player.listedForSale = false;
            player.askingPrice = 0;
            player.totalInjuries = 0;
            player.totalRedCards = 0;
        }
    }
}

void initializeCupCompetition() {
    cupPool.clear();
    for (int i = 0; i < (int)teams.size(); i++) cupPool.push_back(i);
    for (int i = (int)cupPool.size() - 1; i > 0; i--) {
        swap(cupPool[i], cupPool[rng(0, i)]);
    }
    cupStartedThisSeason = true;
    cupRoundNumber = 1;
    cupWinnerThisSeason = "TBD";
}

ActionReport simulateLeagueWeb(bool finishSeason) {
    ActionReport report;
    report.title = finishSeason ? "League Season" : "Run League Season";

    if (teams.empty()) {
        report.summary = "Choose a league first.";
        setNotice(report.summary, "error");
        lastLeagueAction = report;
        return report;
    }

    if (leagueFinishedThisSeason) {
        report.summary = "The league season is already complete.";
        setNotice(report.summary, "info");
        lastLeagueAction = report;
        return report;
    }

    if (leagueSchedule.empty()) generateLeagueSchedule();

    int totalMatches = 0;
    int matchdays = 0;
    int firstMatchday = currentLeagueMatchdayNumber();
    const int maxReports = finishSeason ? 30 : 20;
    if (finishSeason && firstMatchday > 1) {
        report.notes.push_back("Continuing from Matchday " + to_string(firstMatchday));
    }

    while (true) {
        int start = nextUnplayedLeagueMatchIndex();
        if (start < 0) break;

        int end = min(start + leagueMatchesPerDay(), (int)leagueSchedule.size());
        int matchday = start / leagueMatchesPerDay() + 1;
        report.notes.push_back("Matchday " + to_string(matchday));

        for (int i = start; i < end; i++) {
            if (leagueSchedule[i].played) continue;
            MatchSimulationResult result = simulateMatchDetailed(leagueSchedule[i].homeIdx, leagueSchedule[i].awayIdx, false, false);
            result.stageLabel = "Matchday " + to_string(matchday) + " - Match " + to_string(i - start + 1);
            leagueSchedule[i] = result.match;
            totalMatches++;

            if ((int)report.matches.size() == maxReports) {
                report.matches.erase(report.matches.begin());
            }
            report.matches.push_back(result);
        }

        matchdays++;
        if (!finishSeason) break;
    }

    if (nextUnplayedLeagueMatchIndex() < 0) {
        leagueFinishedThisSeason = true;
        syncSeasonRecord();
        report.summary = "Season complete. " + getLeagueWinner() + " are league champions.";
        if (totalMatches > maxReports) {
            report.notes.push_back("Showing the most recent " + to_string(maxReports) + " match reports.");
        }
    }
    else {
        if (finishSeason) {
            report.summary = "Simulated " + to_string(matchdays) + " matchdays and " + to_string(totalMatches) + " matches.";
            if (totalMatches > maxReports) {
                report.notes.push_back("Showing the most recent " + to_string(maxReports) + " match reports.");
            }
        }
        else {
            report.summary = "Matchday " + to_string(firstMatchday) + " completed.";
        }
    }

    report.batchesSimulated = matchdays;
    setNotice(report.summary, "success");
    lastLeagueAction = report;
    return report;
}

ActionReport simulateCupWeb(bool finishCup) {
    ActionReport report;
    report.title = finishCup ? "Cup Competition" : "Run Cup Competition";

    if (teams.empty()) {
        report.summary = "Choose a league first.";
        setNotice(report.summary, "error");
        lastCupAction = report;
        return report;
    }

    if (cupFinishedThisSeason) {
        report.summary = "The cup is already complete.";
        setNotice(report.summary, "info");
        lastCupAction = report;
        return report;
    }

    if (!cupStartedThisSeason || cupPool.empty()) initializeCupCompetition();

    int rounds = 0;
    int firstRound = cupRoundNumber;
    if (finishCup && firstRound > 1) {
        report.notes.push_back("Continuing from Round " + to_string(firstRound));
    }
    while (cupPool.size() > 1) {
        int currentRound = cupRoundNumber;
        vector<int> survivors;
        report.notes.push_back("Round " + to_string(currentRound));

        for (int i = 0; i + 1 < (int)cupPool.size(); i += 2) {
            MatchSimulationResult result = simulateMatchDetailed(cupPool[i], cupPool[i + 1], true, false);
            result.stageLabel = "Round " + to_string(currentRound) + " - Match " + to_string(i / 2 + 1);
            report.matches.push_back(result);
            survivors.push_back((result.match.homeGoals > result.match.awayGoals) ? cupPool[i] : cupPool[i + 1]);
        }

        if (cupPool.size() % 2 == 1) {
            survivors.push_back(cupPool.back());
            report.notes.push_back(teams[cupPool.back()].name + " received a bye.");
        }

        cupPool = survivors;
        rounds++;

        if (cupPool.size() <= 1) break;
        cupRoundNumber++;
        if (!finishCup) break;
    }

    if (cupPool.size() == 1) {
        cupFinishedThisSeason = true;
        cupWinnerThisSeason = teams[cupPool[0]].name;
        syncSeasonRecord();
        report.summary = cupWinnerThisSeason + " won the cup.";
    }
    else {
        report.summary = finishCup ? ("Simulated " + to_string(rounds) + " cup rounds.") :
                                     ("Round " + to_string(cupRoundNumber) + " completed.");
    }

    report.batchesSimulated = rounds;
    setNotice(report.summary, "success");
    lastCupAction = report;
    return report;
}

vector<pair<int, int>> listedPlayerRefs() {
    vector<pair<int, int>> refs;
    for (int t = 0; t < (int)teams.size(); t++) {
        for (int p = 0; p < (int)teams[t].players.size(); p++) {
            if (teams[t].players[p].listedForSale) refs.push_back({ t, p });
        }
    }
    return refs;
}

pair<int, int> parsePlayerRef(const string& ref) {
    size_t sep = ref.find(':');
    if (sep == string::npos) return { -1, -1 };
    return { toInt(ref.substr(0, sep), -1), toInt(ref.substr(sep + 1), -1) };
}

bool listPlayerForSaleWeb(int teamIdx, int playerIdx, int askingPrice, string& message) {
    if (teamIdx < 0 || teamIdx >= (int)teams.size()) {
        message = "Choose a valid team.";
        return false;
    }
    if (playerIdx < 0 || playerIdx >= (int)teams[teamIdx].players.size()) {
        message = "Choose a valid player.";
        return false;
    }
    if (askingPrice <= 0) {
        message = "Enter a valid asking price.";
        return false;
    }

    Player& player = teams[teamIdx].players[playerIdx];
    player.listedForSale = true;
    player.askingPrice = askingPrice;
    message = player.name + " is now listed for " + to_string(askingPrice) + "M.";
    return true;
}

bool buyPlayerWeb(int buyerIdx, int sellerIdx, int playerIdx, int offer, string& message) {
    if (buyerIdx < 0 || buyerIdx >= (int)teams.size()) {
        message = "Choose a valid buying team.";
        return false;
    }
    if (sellerIdx < 0 || sellerIdx >= (int)teams.size() || playerIdx < 0 || playerIdx >= (int)teams[sellerIdx].players.size()) {
        message = "Choose a valid listed player.";
        return false;
    }
    if (buyerIdx == sellerIdx) {
        message = "A team cannot buy its own player.";
        return false;
    }

    Player player = teams[sellerIdx].players[playerIdx];
    if (!player.listedForSale) {
        message = "That player is no longer listed for sale.";
        return false;
    }

    if (offer <= 0) {
        message = "Enter a valid offer.";
        return false;
    }

    int asking = player.askingPrice;
    if (offer < asking && offer < asking * 80 / 100) {
        message = "Offer rejected. Minimum accepted offer is " + to_string(asking * 80 / 100) + "M.";
        return false;
    }

    if (teams[buyerIdx].budget < offer) {
        message = teams[buyerIdx].name + " do not have enough budget.";
        return false;
    }

    teams[buyerIdx].budget -= offer;
    teams[sellerIdx].budget += offer;
    player.listedForSale = false;
    player.askingPrice = 0;
    teams[buyerIdx].players.push_back(player);
    teams[sellerIdx].players.erase(teams[sellerIdx].players.begin() + playerIdx);

    message = player.name + " joined " + teams[buyerIdx].name + " for " + to_string(offer) + "M.";
    return true;
}

void loadCustomLeagueFromRequest(const httplib::Request& req) {
    teams.clear();
    leagueName = trimCopy(paramValue(req, "league_name", "Custom League"));
    if (leagueName.empty()) leagueName = "Custom League";

    int teamCount = max(2, toInt(paramValue(req, "team_count", "2"), 2));
    for (int i = 0; i < teamCount; i++) {
        Team team;
        team.name = trimCopy(paramValue(req, "team_" + to_string(i) + "_name", "Team " + to_string(i + 1)));
        team.stadium = trimCopy(paramValue(req, "team_" + to_string(i) + "_stadium", team.name + " Stadium"));
        team.city = trimCopy(paramValue(req, "team_" + to_string(i) + "_city", "City " + to_string(i + 1)));
        team.strength = max(1, min(10, toInt(paramValue(req, "team_" + to_string(i) + "_strength", "5"), 5)));
        team.budget = max(10, toInt(paramValue(req, "team_" + to_string(i) + "_budget", "50"), 50));

        for (int p = 0; p < 16; p++) {
            Player player;
            player.name = trimCopy(paramValue(req,
                "team_" + to_string(i) + "_player_" + to_string(p) + "_name",
                team.name + " Player " + to_string(p + 1)));
            player.position = trimCopy(paramValue(req,
                "team_" + to_string(i) + "_player_" + to_string(p) + "_position",
                defaultPositionForIndex(p)));
            if (player.position.empty()) player.position = defaultPositionForIndex(p);
            player.marketValue = defaultMarketValueForIndex(p);
            player.isSubstitute = (p >= 11);
            team.players.push_back(player);
        }

        teams.push_back(team);
    }

    prepareLoadedLeagueState();
}

string navClass(const string& active, const string& current) {
    return active == current ? "nav-link active" : "nav-link";
}

string renderNotice() {
    if (webNotice.empty()) return "";
    string message = webNotice;
    string kind = webNoticeKind;
    webNotice.clear();
    webNoticeKind = "info";
    stringstream ss;
    ss << "<div class=\"notice " << escapeHtml(kind) << "\">" << escapeHtml(message) << "</div>";
    return ss.str();
}

string walkthroughSkipLabel(const ActionReport& report) {
    if (report.title == "League Season") return "Show Full Season Results";
    if (report.title == "Cup Competition") return "Show Full Cup Results";
    if (report.title == "Run League Season") return "Show Full Matchday Results";
    if (report.title == "Run Cup Competition") return "Show Full Round Results";
    return "Show Full Results Now";
}

string walkthroughShownStatus(const ActionReport& report) {
    if (report.title == "League Season") return "Full season results shown.";
    if (report.title == "Cup Competition") return "Full cup results shown.";
    if (report.title == "Run League Season") return "Full matchday results shown.";
    if (report.title == "Run Cup Competition") return "Full round results shown.";
    return "Full results shown.";
}

string jsStringLiteral(const string& value) {
    string out = "\"";
    for (char ch : value) {
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += ch; break;
        }
    }
    out += "\"";
    return out;
}

string matchEventClass(const CommentaryEvent& event) {
    if (event.text.find("GOAL") != string::npos) return "goal";
    if (event.text.find("Yellow") != string::npos) return "yellow";
    if (event.text.find("RED") != string::npos) return "red";
    if (event.text.find("INJURY") != string::npos) return "injury";
    if (event.text.find("SUB") != string::npos) return "sub";
    if (event.text.find("Kick-off") != string::npos || event.text.find("Half-time") != string::npos ||
        event.text.find("Full time") != string::npos || event.text.find("Extra time") != string::npos ||
        event.text.find("End of normal time") != string::npos) return "phase";
    return "other";
}

string renderPageShell(const string& title, const string& active, const string& body, const string& extraScript = "") {
    stringstream html;
    html << R"SHELL(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)SHELL" << escapeHtml(title) << R"SHELL(</title>
    <style>
        :root {
            --bg-1: #07121b;
            --bg-2: #102739;
            --panel: rgba(8, 20, 31, 0.82);
            --panel-border: rgba(116, 210, 255, 0.14);
            --text: #edf7ff;
            --muted: #93a8b7;
            --accent: #74d2ff;
            --accent-2: #ffb45c;
            --success: #7fd68f;
            --danger: #ff7e7e;
            --warning: #ffd37a;
        }
        * { box-sizing: border-box; }
        body {
            margin: 0;
            min-height: 100vh;
            font-family: 'Trebuchet MS', Verdana, sans-serif;
            color: var(--text);
            background:
                radial-gradient(circle at top left, rgba(116, 210, 255, 0.18), transparent 30%),
                radial-gradient(circle at bottom right, rgba(255, 180, 92, 0.14), transparent 25%),
                linear-gradient(145deg, var(--bg-1) 0%, var(--bg-2) 55%, #07131d 100%);
        }
        a { color: inherit; }
        .shell {
            max-width: 1180px;
            margin: 0 auto;
            padding: 24px 16px 40px;
        }
        .hero, .panel, .mini-card, .card, .table-wrap, .form-card, .match-card {
            background: var(--panel);
            border: 1px solid var(--panel-border);
            border-radius: 24px;
            box-shadow: 0 24px 70px rgba(0, 0, 0, 0.28);
            backdrop-filter: blur(14px);
        }
        .hero {
            padding: 28px;
            margin-bottom: 18px;
        }
        .hero-top {
            display: flex;
            justify-content: space-between;
            align-items: flex-start;
            gap: 12px;
        }
        .hero-actions {
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .eyebrow, .pill {
            display: inline-block;
            padding: 6px 12px;
            border-radius: 999px;
            background: rgba(116, 210, 255, 0.12);
            color: var(--accent);
            text-transform: uppercase;
            letter-spacing: 0.08em;
            font-size: 0.74rem;
        }
        h1, h2, h3 { margin: 0; }
        .hero h1 {
            margin-top: 14px;
            font-size: clamp(2rem, 4vw, 3.2rem);
            line-height: 1.05;
        }
        .hero p, .panel-copy, .card-copy, .meta, .note, .empty, label, .notice, .form-help {
            color: var(--muted);
            line-height: 1.55;
        }
        .hero p { max-width: 720px; margin: 12px 0 0; }
        .nav {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin: 0 0 18px;
        }
        .nav-link {
            text-decoration: none;
            padding: 11px 14px;
            border-radius: 999px;
            color: var(--muted);
            border: 1px solid rgba(255,255,255,0.08);
            background: rgba(255,255,255,0.04);
            transition: 0.2s ease;
        }
        .nav-link:hover, .nav-link.active {
            color: var(--text);
            border-color: rgba(116, 210, 255, 0.36);
            background: rgba(116, 210, 255, 0.12);
        }
        .grid {
            display: grid;
            gap: 16px;
        }
        .two-col {
            grid-template-columns: minmax(0, 1.15fr) minmax(0, 0.95fr);
        }
        .three-col {
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
        }
        .panel, .form-card, .table-wrap {
            padding: 22px;
        }
        .panel-head {
            margin-bottom: 16px;
        }
        .panel-head p {
            margin: 8px 0 0;
        }
        .stat-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 12px;
            margin-top: 20px;
        }
        .mini-card {
            padding: 16px 18px;
        }
        .mini-label {
            display: block;
            margin-bottom: 8px;
            color: var(--muted);
            text-transform: uppercase;
            letter-spacing: 0.08em;
            font-size: 0.74rem;
        }
        .card-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
            gap: 14px;
        }
        .card {
            display: block;
            padding: 20px;
            text-decoration: none;
            transition: transform 0.2s ease, border-color 0.2s ease;
        }
        .card:hover, .button:hover, .form-card:hover {
            transform: translateY(-3px);
            border-color: rgba(116, 210, 255, 0.34);
        }
        .card-title {
            margin-top: 14px;
            font-size: 1.1rem;
            font-weight: bold;
        }
        .card-copy {
            margin-top: 8px;
        }
        .preset-card {
            position: relative;
            text-align: left;
            overflow: hidden;
        }
        .preset-card::before {
            content: "";
            position: absolute;
            inset: 0 0 auto 0;
            height: 4px;
            background: linear-gradient(90deg, var(--accent), var(--accent-2));
        }
        .preset-head {
            display: flex;
            align-items: flex-start;
            justify-content: space-between;
            gap: 12px;
        }
        .preset-region {
            color: var(--accent-2);
            text-transform: uppercase;
            letter-spacing: 0.16em;
            font-size: 0.72rem;
        }
        .preset-code {
            font-size: 2.1rem;
            font-weight: 900;
            line-height: 0.95;
            letter-spacing: 0.08em;
            color: rgba(237, 247, 255, 0.86);
        }
        .preset-name {
            margin-top: 18px;
            font-size: 1.55rem;
            line-height: 1.02;
            font-weight: 900;
            color: var(--text);
            text-wrap: balance;
        }
        .preset-card .card-copy {
            margin-top: 12px;
            max-width: 22ch;
        }
        .button-row, .inline-actions {
            display: flex;
            flex-wrap: wrap;
            gap: 12px;
        }
        .custom-builder-toolbar {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 14px;
            margin-bottom: 16px;
        }
        .custom-builder-toolbar input {
            margin-bottom: 0;
        }
        .custom-builder-status {
            margin-top: 8px;
            color: var(--muted);
        }
        .custom-builder-shell {
            margin-top: 18px;
            padding: 18px;
            border-radius: 20px;
            border: 1px solid rgba(255,255,255,0.08);
            background: rgba(255,255,255,0.03);
        }
        .custom-builder-shell.is-hidden {
            display: none;
        }
        .custom-builder-step {
            color: var(--text);
            font-weight: bold;
        }
        .custom-builder-nav {
            display: flex;
            flex-wrap: wrap;
            gap: 12px;
            align-items: center;
            margin-top: 16px;
        }
        .team-builder-stack {
            display: grid;
            gap: 14px;
            margin-top: 18px;
        }
        .team-builder-card {
            display: none;
            margin-top: 0;
            padding-top: 0;
            border-top: 0;
            background: rgba(255,255,255,0.03);
            border: 1px solid rgba(255,255,255,0.08);
            border-radius: 18px;
            overflow: hidden;
        }
        .team-builder-card.is-active {
            display: block;
        }
        .team-builder-card summary {
            list-style: none;
            padding: 16px 18px;
            display: flex;
            align-items: center;
            justify-content: space-between;
            color: var(--text);
        }
        .team-builder-card summary::-webkit-details-marker {
            display: none;
        }
        .team-builder-card[open] summary {
            border-bottom: 1px solid rgba(255,255,255,0.08);
        }
        .team-builder-title {
            margin-top: 10px;
            font-size: 1.1rem;
            font-weight: bold;
            color: var(--text);
        }
        .team-summary-meta {
            margin-top: 6px;
            color: var(--muted);
            font-size: 0.92rem;
        }
        .team-builder-content {
            padding: 18px;
        }
        .team-meta-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(180px, 1fr));
            gap: 12px;
        }
        .team-meta-grid input,
        .team-meta-grid select {
            margin-bottom: 0;
        }
        .player-builder-list {
            margin-top: 16px;
            display: grid;
            gap: 10px;
        }
        .player-builder-row {
            display: grid;
            grid-template-columns: 110px minmax(0, 1fr) 120px;
            gap: 10px;
            align-items: center;
            padding: 10px 12px;
            border-radius: 14px;
            background: rgba(255,255,255,0.03);
            border: 1px solid rgba(255,255,255,0.06);
        }
        .player-builder-row input,
        .player-builder-row select {
            margin: 0;
        }
        .player-slot {
            display: flex;
            flex-direction: column;
            gap: 4px;
        }
        .player-slot strong {
            color: var(--text);
            font-size: 0.95rem;
        }
        .player-slot span {
            color: var(--muted);
            font-size: 0.82rem;
        }
        .button, button {
            border: 0;
            cursor: pointer;
            border-radius: 16px;
            padding: 12px 18px;
            font: inherit;
            color: #04141f;
            background: linear-gradient(90deg, var(--accent), #9fe6ff);
            box-shadow: 0 18px 35px rgba(116, 210, 255, 0.18);
        }
        .button.alt {
            background: linear-gradient(90deg, var(--accent-2), #ffd18a);
        }
        .button.ghost {
            color: var(--text);
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.08);
            box-shadow: none;
        }
        .button.is-active {
            color: #04141f;
            background: linear-gradient(90deg, #7fd68f, #b7f2c0);
            border: 1px solid rgba(127, 214, 143, 0.45);
            box-shadow: 0 18px 35px rgba(127, 214, 143, 0.18);
        }
        .button.danger {
            background: linear-gradient(90deg, #ff9a9a, #ffc1a3);
        }
        .button.edge-exit {
            padding: 8px 12px;
            font-size: 0.86rem;
            border-radius: 999px;
            color: #ffe8e8;
            background: rgba(255, 126, 126, 0.12);
            border: 1px solid rgba(255, 126, 126, 0.28);
            box-shadow: none;
        }
        .edge-icon-link {
            width: 40px;
            height: 40px;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            border-radius: 999px;
            color: var(--text);
            background: rgba(255, 255, 255, 0.06);
            border: 1px solid rgba(255, 255, 255, 0.1);
            box-shadow: none;
            text-decoration: none;
            transition: 0.2s ease;
        }
        .edge-icon-link:hover {
            transform: translateY(-2px);
            border-color: rgba(116, 210, 255, 0.34);
            background: rgba(116, 210, 255, 0.12);
        }
        .edge-icon-link svg {
            width: 18px;
            height: 18px;
            display: block;
        }
        .button:disabled, button:disabled {
            cursor: not-allowed;
            opacity: 0.45;
            transform: none;
            box-shadow: none;
        }
        form { margin: 0; }
        input, select, textarea {
            width: 100%;
            margin-top: 8px;
            margin-bottom: 14px;
            border-radius: 14px;
            border: 1px solid rgba(255,255,255,0.08);
            background: rgba(255,255,255,0.04);
            color: var(--text);
            padding: 12px 14px;
            font: inherit;
        }
        select option {
            color: #0a1620;
        }
        textarea {
            min-height: 120px;
            resize: vertical;
        }
        .table-wrap {
            overflow-x: auto;
        }
        table {
            width: 100%;
            border-collapse: collapse;
        }
        th, td {
            padding: 12px 10px;
            text-align: left;
            border-bottom: 1px solid rgba(255,255,255,0.06);
        }
        th {
            color: var(--accent);
            text-transform: uppercase;
            font-size: 0.78rem;
            letter-spacing: 0.08em;
        }
        td.center, th.center {
            text-align: center;
        }
        .notice {
            margin-bottom: 16px;
            padding: 14px 16px;
            border-radius: 16px;
            border: 1px solid rgba(255,255,255,0.08);
            background: rgba(255,255,255,0.05);
            max-height: 120px;
            overflow: hidden;
            transition: opacity 0.35s ease, transform 0.35s ease, max-height 0.35s ease, margin 0.35s ease, padding 0.35s ease, border-width 0.35s ease;
        }
        .notice.is-hiding {
            opacity: 0;
            transform: translateY(-8px);
            max-height: 0;
            margin-bottom: 0;
            padding-top: 0;
            padding-bottom: 0;
            border-width: 0;
        }
        .notice.success {
            color: #d7ffe0;
            border-color: rgba(127, 214, 143, 0.32);
            background: rgba(127, 214, 143, 0.12);
        }
        .notice.error {
            color: #ffe0e0;
            border-color: rgba(255, 126, 126, 0.32);
            background: rgba(255, 126, 126, 0.12);
        }
        .notice.info {
            color: #d7f5ff;
            border-color: rgba(116, 210, 255, 0.26);
            background: rgba(116, 210, 255, 0.10);
        }
        .notice.warn {
            color: #fff0cf;
            border-color: rgba(255, 211, 122, 0.32);
            background: rgba(255, 211, 122, 0.12);
        }
        .match-list {
            display: grid;
            gap: 14px;
        }
        .match-card {
            padding: 18px;
        }
        .match-score {
            font-size: 1.15rem;
            font-weight: bold;
            margin-bottom: 8px;
        }
        .match-fixture {
            margin-bottom: 10px;
            color: var(--muted);
            font-size: 0.95rem;
        }
        .match-result {
            color: var(--accent);
            margin-bottom: 10px;
        }
        .match-card.walkthrough-match.is-current {
            border-color: rgba(116, 210, 255, 0.36);
            box-shadow: 0 0 0 1px rgba(116, 210, 255, 0.14), 0 24px 70px rgba(0, 0, 0, 0.28);
        }
        .match-card.walkthrough-match.is-finished {
            border-color: rgba(127, 214, 143, 0.24);
        }
        .walkthrough-hidden-result {
            opacity: 0;
            max-height: 0;
            overflow: hidden;
            transform: translateY(-6px);
            transition: opacity 0.35s ease, transform 0.35s ease, max-height 0.35s ease;
        }
        .walkthrough-hidden-result.is-visible {
            opacity: 1;
            max-height: 80px;
            transform: translateY(0);
        }
        details {
            margin-top: 12px;
            border-top: 1px solid rgba(255,255,255,0.08);
            padding-top: 12px;
        }
        summary {
            cursor: pointer;
            color: var(--warning);
        }
        .event-list {
            margin: 14px 0 0;
            padding-left: 0;
            list-style: none;
            display: grid;
            gap: 10px;
        }
        .event-line {
            display: flex;
            align-items: flex-start;
            gap: 10px;
            padding: 10px 12px;
            border-radius: 14px;
            border: 1px solid rgba(255,255,255,0.06);
            background: rgba(255,255,255,0.03);
            transition: opacity 0.35s ease, transform 0.35s ease, filter 0.35s ease;
        }
        .event-line.is-hidden {
            opacity: 0;
            transform: translateX(-12px) scale(0.98);
            filter: blur(3px);
        }
        .event-line.is-visible {
            opacity: 1;
            transform: translateX(0) scale(1);
            filter: blur(0);
        }
        .event-minute {
            min-width: 48px;
            padding: 5px 8px;
            border-radius: 10px;
            text-align: center;
            font-weight: bold;
            color: #04141f;
            background: rgba(255,255,255,0.72);
        }
        .event-text {
            color: var(--text);
            line-height: 1.5;
        }
        .event-line.goal {
            border-color: rgba(127, 214, 143, 0.28);
            background: rgba(127, 214, 143, 0.12);
        }
        .event-line.goal .event-minute {
            background: #7fd68f;
        }
        .event-line.yellow {
            border-color: rgba(255, 211, 122, 0.30);
            background: rgba(255, 211, 122, 0.12);
        }
        .event-line.yellow .event-minute {
            background: #ffd37a;
        }
        .event-line.red {
            border-color: rgba(255, 126, 126, 0.30);
            background: rgba(255, 126, 126, 0.14);
        }
        .event-line.red .event-minute {
            background: #ff7e7e;
        }
        .event-line.injury {
            border-color: rgba(244, 125, 199, 0.30);
            background: rgba(244, 125, 199, 0.12);
        }
        .event-line.injury .event-minute {
            background: #f47dc7;
        }
        .event-line.sub {
            border-color: rgba(116, 210, 255, 0.32);
            background: rgba(116, 210, 255, 0.12);
        }
        .event-line.sub .event-minute {
            background: #74d2ff;
        }
        .event-line.phase {
            border-color: rgba(116, 210, 255, 0.32);
            background: rgba(116, 210, 255, 0.10);
        }
        .event-line.phase .event-minute {
            background: #74d2ff;
        }
        .event-line.other {
            border-color: rgba(255,255,255,0.08);
            background: rgba(255,255,255,0.04);
        }
        .event-line.other .event-minute {
            background: #c9d7e1;
        }
        .walkthrough-minute {
            margin: 0 0 10px;
            display: inline-flex;
            gap: 8px;
            align-items: center;
            padding: 8px 12px;
            border-radius: 999px;
            background: rgba(255,255,255,0.05);
            border: 1px solid rgba(255,255,255,0.08);
            color: var(--muted);
            font-size: 0.92rem;
        }
        .walkthrough-minute strong {
            color: var(--text);
            font-size: 1rem;
        }
        .walkthrough-bar {
            margin-top: 14px;
            display: flex;
            flex-wrap: wrap;
            gap: 12px;
            align-items: center;
        }
        .walkthrough-status {
            color: var(--muted);
            font-size: 0.95rem;
        }
        .split {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
            gap: 14px;
        }
        .subtle {
            color: var(--muted);
        }
        @media (max-width: 900px) {
            .two-col { grid-template-columns: 1fr; }
        }
        @media (max-width: 700px) {
            .player-builder-row { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="shell">)SHELL";

    if (!teams.empty()) {
        html << R"SHELL(
        <nav class="nav">
            <a class=")SHELL" << navClass(active, "dashboard") << R"SHELL(" href="/">Dashboard</a>
            <a class=")SHELL" << navClass(active, "league") << R"SHELL(" href="/league">Run League</a>
            <a class=")SHELL" << navClass(active, "cup") << R"SHELL(" href="/cup">Run Cup</a>
            <a class=")SHELL" << navClass(active, "transfers") << R"SHELL(" href="/transfers">Transfers</a>
            <a class=")SHELL" << navClass(active, "standings") << R"SHELL(" href="/standings">Standings</a>
            <a class=")SHELL" << navClass(active, "scorers") << R"SHELL(" href="/scorers">Scorers</a>
            <a class=")SHELL" << navClass(active, "injuries") << R"SHELL(" href="/injuries">Injuries</a>
            <a class=")SHELL" << navClass(active, "squads") << R"SHELL(" href="/squads">Squads</a>
            <a class=")SHELL" << navClass(active, "history") << R"SHELL(" href="/history">History</a>
            <a class=")SHELL" << navClass(active, "new-season") << R"SHELL(" href="/new-season">New Season</a>
            <a class=")SHELL" << navClass(active, "settings") << R"SHELL(" href="/settings">Settings</a>
        </nav>)SHELL";
    }

    html << renderNotice();
    html << body;
    html << R"SHELL(
    </div>)SHELL";

    if (!extraScript.empty()) {
        html << "<script>" << extraScript << "</script>";
    }

    html << R"SHELL(
<script>
(() => {
    const notice = document.querySelector('.notice');
    if (!notice) return;
    window.setTimeout(() => {
        notice.classList.add('is-hiding');
        window.setTimeout(() => {
            if (notice.parentNode) notice.parentNode.removeChild(notice);
        }, 380);
    }, 3600);
})();
</script>)SHELL";

    html << R"SHELL(
</body>
</html>)SHELL";
    return html.str();
}

string renderActionReport(const ActionReport& report) {
    if (report.summary.empty() && report.matches.empty() && report.notes.empty()) return "";

    bool walkthroughMode = !fastDisplay && !report.matches.empty();
    string skipLabel = walkthroughSkipLabel(report);
    string shownStatus = walkthroughShownStatus(report);
    stringstream html;
    html << R"REP(<section class="panel">
        <div class="panel-head">
            <span class="pill">Latest Result</span>
            <h2>)REP" << escapeHtml(report.title.empty() ? "Recent Simulation" : report.title) << R"REP(</h2>
            <p>)REP" << escapeHtml(report.summary) << R"REP(</p>
        </div>)REP";

    if (walkthroughMode) {
        html << R"REP(
        <div class="walkthrough-bar">
            <span class="pill">Walkthrough Mode</span>
            <span class="walkthrough-status" id="walkthrough-status">The live feed will play minute by minute.</span>
            <button class="button ghost" type="button" id="follow-match-toggle">Follow Match</button>
            <button class="button ghost" type="button" id="skip-walkthrough">)REP" << escapeHtml(skipLabel) << R"REP(</button>
        </div>)REP";
    }

    if (!report.notes.empty()) {
        html << "<div class=\"inline-actions\">";
        for (const auto& note : report.notes) {
            html << "<span class=\"pill\">" << escapeHtml(note) << "</span>";
        }
        html << "</div>";
    }

    if (!report.matches.empty()) {
        html << "<div class=\"match-list\" style=\"margin-top:16px;\">";
        for (size_t i = 0; i < report.matches.size(); i++) {
            const auto& match = report.matches[i];
            html << "<article class=\"match-card";
            if (walkthroughMode) html << " walkthrough-match";
            html << "\" data-fixture=\"" << escapeHtml(match.homeName + " vs " + match.awayName) << "\">";
            html << "<div class=\"pill\">" << escapeHtml(match.stageLabel.empty() ? ("Match " + to_string(i + 1)) : match.stageLabel) << "</div>";
            html << "<div class=\"match-fixture\">" << escapeHtml(match.homeName + " vs " + match.awayName) << "</div>";
            if (walkthroughMode) {
                html << "<div class=\"walkthrough-minute\">Live minute <strong class=\"walkthrough-minute-value\">0'</strong></div>";
            }
            html << "<div class=\"match-score";
            if (walkthroughMode) html << " walkthrough-hidden-result";
            html << "\">" << escapeHtml(match.resultLine) << "</div>";
            html << "<div class=\"match-result";
            if (walkthroughMode) html << " walkthrough-hidden-result";
            html << "\">" << escapeHtml(match.winnerLine) << "</div>";
            if (!match.events.empty()) {
                html << "<details";
                if (walkthroughMode) html << " open class=\"walkthrough-details\"";
                html << "><summary>" << (walkthroughMode ? "Live Match Feed" : "Match commentary") << "</summary><ul class=\"event-list\">";
                for (const auto& event : match.events) {
                    html << "<li class=\"event-line " << matchEventClass(event);
                    if (walkthroughMode) html << " is-hidden";
                    else html << " is-visible";
                    html << "\" data-minute=\"" << event.minute << "\">";
                    html << "<span class=\"event-minute\">" << event.minute << "'</span>";
                    html << "<span class=\"event-text\">" << escapeHtml(event.text) << "</span>";
                    html << "</li>";
                }
                html << "</ul></details>";
            }
            html << "</article>";
        }
        html << "</div>";
    }

    if (walkthroughMode) {
        html << R"REP(
<script>
(function() {
    const cards = Array.from(document.querySelectorAll('.walkthrough-match'));
    const status = document.getElementById('walkthrough-status');
    const followButton = document.getElementById('follow-match-toggle');
    const skipButton = document.getElementById('skip-walkthrough');
    const shownStatus = )REP" << jsStringLiteral(shownStatus) << R"REP(;
    let skipped = false;
    let followMode = false;
    let autoScrollLockUntil = 0;
    const kickoffPause = cards.length > 8 ? 950 : 1200;
    const minuteDelay = cards.length > 15 ? 55 : cards.length > 8 ? 72 : cards.length > 4 ? 95 : 120;
    const matchPause = cards.length > 8 ? 2200 : 3000;

    function wait(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    function revealLine(line) {
        line.classList.remove('is-hidden');
        line.classList.add('is-visible');
    }

    function updateFollowButton() {
        if (!followButton) return;
        followButton.textContent = followMode ? 'Following Match' : 'Follow Match';
        followButton.classList.toggle('is-active', followMode);
    }

    function currentFollowTarget() {
        const currentCard = document.querySelector('.walkthrough-match.is-current');
        const latestFinishedCard = cards.slice().reverse().find(card => card.classList.contains('is-finished'));
        const targetCard = currentCard || latestFinishedCard || cards[0];
        if (!targetCard) return null;
        const visibleLines = Array.from(targetCard.querySelectorAll('.event-line.is-visible'));
        return visibleLines.length ? visibleLines[visibleLines.length - 1] : targetCard;
    }

    function followTarget(target) {
        if (!followMode || !target) return;
        autoScrollLockUntil = Date.now() + 450;
        target.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
    }

    function stopFollowing() {
        if (!followMode) return;
        followMode = false;
        updateFollowButton();
    }

    function showResult(card, minuteLabel) {
        card.querySelectorAll('.walkthrough-hidden-result').forEach(block => {
            block.classList.add('is-visible');
        });
        const minuteValue = card.querySelector('.walkthrough-minute-value');
        if (minuteValue && minuteLabel) {
            minuteValue.textContent = minuteLabel;
        }
    }

    function eventPause(bucket) {
        if (!bucket.length) return 0;
        let pause = cards.length > 8 ? 260 : 360;
        bucket.forEach(line => {
            if (line.classList.contains('goal')) pause += cards.length > 8 ? 520 : 700;
            else if (line.classList.contains('red') || line.classList.contains('injury')) pause += cards.length > 8 ? 420 : 560;
            else if (line.classList.contains('yellow') || line.classList.contains('sub')) pause += cards.length > 8 ? 220 : 320;
            else pause += cards.length > 8 ? 180 : 260;
        });
        return pause;
    }

    function showAllNow() {
        skipped = true;
        stopFollowing();
        cards.forEach(card => {
            card.classList.remove('is-current');
            card.classList.add('is-finished');
            card.querySelectorAll('.event-line').forEach(revealLine);
            showResult(card, 'FT');
            const details = card.querySelector('details');
            if (details) details.open = true;
        });
        if (status) status.textContent = shownStatus;
    }

    if (skipButton) {
        skipButton.addEventListener('click', showAllNow);
    }

    if (followButton) {
        updateFollowButton();
        followButton.addEventListener('click', () => {
            followMode = true;
            updateFollowButton();
            followTarget(currentFollowTarget());
        });
    }

    window.addEventListener('scroll', () => {
        if (!followMode) return;
        if (Date.now() <= autoScrollLockUntil) return;
        stopFollowing();
    }, { passive: true });

    window.addEventListener('wheel', () => {
        if (!followMode) return;
        if (Date.now() <= autoScrollLockUntil) return;
        stopFollowing();
    }, { passive: true });

    window.addEventListener('touchmove', () => {
        if (!followMode) return;
        if (Date.now() <= autoScrollLockUntil) return;
        stopFollowing();
    }, { passive: true });

    window.addEventListener('keydown', (event) => {
        if (!followMode) return;
        if (Date.now() <= autoScrollLockUntil) return;
        if (['ArrowUp', 'ArrowDown', 'PageUp', 'PageDown', 'Home', 'End', ' '].includes(event.key)) {
            stopFollowing();
        }
    });

    (async function runWalkthrough() {
        for (const card of cards) {
            if (skipped) return;

            const fixture = card.dataset.fixture || 'Match';
            const details = card.querySelector('details');
            const minuteValue = card.querySelector('.walkthrough-minute-value');
            const lines = Array.from(card.querySelectorAll('.event-line'));
            const minuteMap = new Map();
            let endMinute = 90;

            lines.forEach(line => {
                const minute = parseInt(line.dataset.minute || '0', 10);
                if (!minuteMap.has(minute)) minuteMap.set(minute, []);
                minuteMap.get(minute).push(line);
                endMinute = Math.max(endMinute, minute);
            });

            card.classList.add('is-current');
            if (details) details.open = true;
            followTarget(card);

            for (let minute = 0; minute <= endMinute; minute++) {
                if (skipped) return;

                if (minuteValue) {
                    minuteValue.textContent = minute + "'";
                }
                if (status) {
                    status.textContent = 'Now playing: ' + fixture + ' - ' + minute + "'";
                }

                const bucket = minuteMap.get(minute) || [];
                bucket.forEach(revealLine);
                if (bucket.length) {
                    followTarget(bucket[bucket.length - 1]);
                }
                await wait((minute === 0 ? kickoffPause : minuteDelay) + eventPause(bucket));
            }

            showResult(card, 'FT');
            followTarget(card.querySelector('.match-result') || card);
            card.classList.remove('is-current');
            card.classList.add('is-finished');
            if (status) {
                status.textContent = (card === cards[cards.length - 1])
                    ? (fixture + ' complete.')
                    : (fixture + ' complete. Next match starts shortly.');
            }
            await wait(card === cards[cards.length - 1] ? 500 : matchPause);
        }

        if (status) status.textContent = 'Walkthrough complete.';
    })();
})();
</script>)REP";
    }

    html << "</section>";
    return html.str();
}

string customLeagueWizardScript() {
    return R"JS(
let currentBuilderStep = 0;

function defaultPosition(index) {
    const positions = ['GK','DEF','DEF','DEF','DEF','MID','MID','MID','STR','STR','STR','GK','DEF','MID','MID','STR'];
    return positions[index] || 'MID';
}

function defaultRoleLabel(index) {
    return index < 11 ? 'Starter ' + (index + 1) : 'Sub ' + (index - 10);
}

function defaultPlayerName(teamName, index) {
    return teamName + ' ' + (index < 11 ? 'starter ' : 'sub ') + (index + 1);
}

function escapeAttr(value) {
    return String(value ?? '')
        .replace(/&/g, '&amp;')
        .replace(/"/g, '&quot;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/'/g, '&#39;');
}

function clampTeamCount(value) {
    const parsed = parseInt(value, 10);
    if (Number.isNaN(parsed)) return 4;
    return Math.max(2, Math.min(20, parsed));
}

function readValue(values, name, fallback) {
    return Object.prototype.hasOwnProperty.call(values, name) ? values[name] : fallback;
}

function readFormValues(form) {
    const values = {};
    form.querySelectorAll('input[name], select[name]').forEach(field => {
        values[field.name] = field.value;
    });
    return values;
}

function buildPositionOptions(selected) {
    const current = (selected || 'MID').toUpperCase();
    return ['GK', 'DEF', 'MID', 'STR'].map(position =>
        `<option value="${position}" ${position === current ? 'selected' : ''}>${position}</option>`
    ).join('');
}

function buildPlayerRows(teamIndex, teamName, values) {
    let rows = '';
    for (let playerIndex = 0; playerIndex < 16; playerIndex++) {
        const nameKey = `team_${teamIndex}_player_${playerIndex}_name`;
        const positionKey = `team_${teamIndex}_player_${playerIndex}_position`;
        const roleLabel = defaultRoleLabel(playerIndex);
        const roleType = playerIndex < 11 ? 'Starter' : 'Substitute';
        const playerName = readValue(values, nameKey, defaultPlayerName(teamName, playerIndex));
        const position = readValue(values, positionKey, defaultPosition(playerIndex));

        rows += `
            <div class="player-builder-row">
                <div class="player-slot">
                    <strong>${roleLabel}</strong>
                    <span>${roleType}</span>
                </div>
                <input type="text" name="${nameKey}" value="${escapeAttr(playerName)}" aria-label="${roleLabel} name" required>
                <select name="${positionKey}" aria-label="${roleLabel} position">${buildPositionOptions(position)}</select>
            </div>`;
    }
    return rows;
}

function buildTeamCard(teamIndex, values) {
    const teamNameKey = `team_${teamIndex}_name`;
    const stadiumKey = `team_${teamIndex}_stadium`;
    const cityKey = `team_${teamIndex}_city`;
    const strengthKey = `team_${teamIndex}_strength`;
    const budgetKey = `team_${teamIndex}_budget`;
    const fallbackName = `Team ${teamIndex + 1}`;
    const teamName = readValue(values, teamNameKey, fallbackName) || fallbackName;
    const city = readValue(values, cityKey, `City ${teamIndex + 1}`) || `City ${teamIndex + 1}`;
    const stadium = readValue(values, stadiumKey, `${teamName} Stadium`) || `${teamName} Stadium`;
    const strength = readValue(values, strengthKey, '5');
    const budget = readValue(values, budgetKey, '50');

    return `
        <details class="team-builder-card" data-team-card="${teamIndex}">
            <summary>
                <div>
                    <span class="pill">Club ${teamIndex + 1}</span>
                    <div class="team-builder-title" data-team-summary="${teamIndex}">${escapeAttr(teamName)}</div>
                    <div class="team-summary-meta" data-team-meta="${teamIndex}">${escapeAttr(city)} | Strength ${escapeAttr(strength)} | Budget ${escapeAttr(budget)}M</div>
                </div>
            </summary>
            <div class="team-builder-content">
                <div class="team-meta-grid">
                    <div>
                        <label>Team Name</label>
                        <input type="text" name="${teamNameKey}" value="${escapeAttr(teamName)}" data-team-header-input="${teamIndex}" required>
                    </div>
                    <div>
                        <label>Stadium</label>
                        <input type="text" name="${stadiumKey}" value="${escapeAttr(stadium)}" required>
                    </div>
                    <div>
                        <label>City</label>
                        <input type="text" name="${cityKey}" value="${escapeAttr(city)}" data-team-header-input="${teamIndex}" required>
                    </div>
                    <div>
                        <label>Strength</label>
                        <input type="number" min="1" max="10" name="${strengthKey}" value="${escapeAttr(strength)}" data-team-header-input="${teamIndex}" required>
                    </div>
                    <div>
                        <label>Budget (M)</label>
                        <input type="number" min="10" max="500" name="${budgetKey}" value="${escapeAttr(budget)}" data-team-header-input="${teamIndex}" required>
                    </div>
                </div>
                <div class="player-builder-list">
                    ${buildPlayerRows(teamIndex, teamName, values)}
                </div>
            </div>
        </details>`;
}

function updateBuilderStatus(teamCount) {
    const status = document.getElementById('custom-builder-status');
    if (!status) return;
    status.textContent = `${teamCount} clubs ready. Fill in one club at a time, then move to the next section.`;
}

function updateBuilderNavigation(teamCount) {
    const step = document.getElementById('custom-builder-step');
    const prev = document.getElementById('custom-builder-prev');
    const next = document.getElementById('custom-builder-next');
    if (step) {
        step.textContent = teamCount > 0 ? `Club ${currentBuilderStep + 1} of ${teamCount}` : 'No clubs yet';
    }
    if (prev) prev.disabled = currentBuilderStep <= 0;
    if (next) next.disabled = teamCount === 0 || currentBuilderStep >= teamCount - 1;
}

function refreshTeamHeader(teamIndex) {
    const fallbackName = `Team ${Number(teamIndex) + 1}`;
    const teamNameInput = document.querySelector(`[name="team_${teamIndex}_name"]`);
    const cityInput = document.querySelector(`[name="team_${teamIndex}_city"]`);
    const strengthInput = document.querySelector(`[name="team_${teamIndex}_strength"]`);
    const budgetInput = document.querySelector(`[name="team_${teamIndex}_budget"]`);
    const summary = document.querySelector(`[data-team-summary="${teamIndex}"]`);
    const meta = document.querySelector(`[data-team-meta="${teamIndex}"]`);

    const teamName = teamNameInput && teamNameInput.value.trim() ? teamNameInput.value.trim() : fallbackName;
    const city = cityInput && cityInput.value.trim() ? cityInput.value.trim() : `City ${Number(teamIndex) + 1}`;
    const strength = strengthInput && strengthInput.value ? strengthInput.value : '5';
    const budget = budgetInput && budgetInput.value ? budgetInput.value : '50';

    if (summary) summary.textContent = teamName;
    if (meta) meta.textContent = `${city} | Strength ${strength} | Budget ${budget}M`;
}

function setActiveTeamStep(index) {
    const cards = Array.from(document.querySelectorAll('.team-builder-card'));
    if (!cards.length) {
        currentBuilderStep = 0;
        updateBuilderNavigation(0);
        return;
    }

    currentBuilderStep = Math.max(0, Math.min(index, cards.length - 1));
    cards.forEach((card, cardIndex) => {
        const isActive = cardIndex === currentBuilderStep;
        card.classList.toggle('is-active', isActive);
        card.open = isActive;
    });
    updateBuilderNavigation(cards.length);
}

function setBuilderVisible(visible) {
    const shell = document.getElementById('custom-builder-shell');
    const openButton = document.getElementById('open-custom-builder');
    if (!shell) return;

    shell.classList.toggle('is-hidden', !visible);
    if (openButton) {
        openButton.textContent = visible ? 'Hide Custom League Builder' : 'Create Custom League';
    }

    if (visible) {
        rebuildCustomLeagueBuilder();
        shell.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
}

function attachBuilderListeners() {
    document.querySelectorAll('[data-team-header-input]').forEach(field => {
        const teamIndex = field.dataset.teamHeaderInput;
        field.addEventListener('input', () => refreshTeamHeader(teamIndex));
        field.addEventListener('change', () => refreshTeamHeader(teamIndex));
    });
}

function rebuildCustomLeagueBuilder() {
    const form = document.getElementById('custom-league-form');
    const builder = document.getElementById('custom-team-builder');
    const countInput = document.getElementById('team-count');
    if (!form || !builder || !countInput) return;

    const values = readFormValues(form);
    const teamCount = clampTeamCount(countInput.value);
    countInput.value = teamCount;

    let html = '';
    for (let teamIndex = 0; teamIndex < teamCount; teamIndex++) {
        html += buildTeamCard(teamIndex, values);
    }

    builder.innerHTML = html;
    updateBuilderStatus(teamCount);
    attachBuilderListeners();
    setActiveTeamStep(Math.min(currentBuilderStep, Math.max(teamCount - 1, 0)));
}

(function initCustomLeagueBuilder() {
    const form = document.getElementById('custom-league-form');
    const countInput = document.getElementById('team-count');
    const refreshButton = document.getElementById('refresh-custom-builder');
    const openButton = document.getElementById('open-custom-builder');
    const closeButton = document.getElementById('close-custom-builder');
    const prevButton = document.getElementById('custom-builder-prev');
    const nextButton = document.getElementById('custom-builder-next');
    if (!form || !countInput) return;

    if (openButton) {
        openButton.addEventListener('click', () => {
            const shell = document.getElementById('custom-builder-shell');
            const showing = shell && !shell.classList.contains('is-hidden');
            setBuilderVisible(!showing);
        });
    }

    if (closeButton) {
        closeButton.addEventListener('click', () => setBuilderVisible(false));
    }

    if (refreshButton) {
        refreshButton.addEventListener('click', rebuildCustomLeagueBuilder);
    }

    if (prevButton) {
        prevButton.addEventListener('click', () => setActiveTeamStep(currentBuilderStep - 1));
    }

    if (nextButton) {
        nextButton.addEventListener('click', () => setActiveTeamStep(currentBuilderStep + 1));
    }

    countInput.addEventListener('change', rebuildCustomLeagueBuilder);
    countInput.addEventListener('blur', rebuildCustomLeagueBuilder);

    rebuildCustomLeagueBuilder();
    setBuilderVisible(false);
})();
)JS";
}

string renderLeagueChoiceSections(bool newSeasonMode) {
    stringstream body;
    body << R"SEL(<section class="panel">
        <div class="panel-head">
            <h2>Preset Leagues</h2>
            <p>Select one of the built-in competitions, or use the full custom league builder below.</p>
        </div>
        <div class="card-grid">)SEL";

    struct PresetChoice {
        int id;
        string name;
        string region;
        string code;
    };

    vector<PresetChoice> choices = {
        {1, "Premier League", "England", "EPL"},
        {2, "La Liga", "Spain", "ESP"},
        {3, "Serie A", "Italy", "ITA"},
        {4, "Ligue 1", "France", "FRA"},
        {5, "Bundesliga", "Germany", "GER"},
        {6, "Egyptian Premier League", "Egypt", "EGY"}
    };

    for (const auto& choice : choices) {
        body << R"SEL(
            <form method="post" action=")SEL" << (newSeasonMode ? "/action/new-season-preset" : "/action/select-preset-league") << R"SEL(">
                <input type="hidden" name="choice" value=")SEL" << choice.id << R"SEL(">
                <button class="card preset-card" type="submit">
                    <div class="preset-head">
                        <span class="pill">Preset League</span>
                        <span class="preset-code">)SEL" << escapeHtml(choice.code) << R"SEL(</span>
                    </div>
                    <div class="preset-region">)SEL" << escapeHtml(choice.region) << R"SEL(</div>
                    <div class="preset-name">)SEL" << escapeHtml(choice.name) << R"SEL(</div>
                    <div class="card-copy">Jump straight into )SEL" << escapeHtml(choice.name) << R"SEL( and start your season.</div>
                </button>
            </form>)SEL";
    }

    body << R"SEL(
        </div>
    </section>
    <section class="panel">
        <div class="panel-head">
            <h2>Custom League Builder</h2>
            <p>Open the builder when you are ready, then fill in the setup one club at a time.</p>
        </div>)SEL";

    if (newSeasonMode) {
        body << R"SEL(
        <div class="button-row" style="margin-bottom:16px;">
            <form method="post" action="/action/new-season-continue">
                <button class="button ghost" type="submit">Keep Current League</button>
            </form>
        </div>)SEL";
    }

    body << R"SEL(
        <div class="button-row">
            <button class="button alt" type="button" id="open-custom-builder">Create Custom League</button>
            <span class="pill">Section By Section Setup</span>
        </div>
        <p class="form-help">The builder opens inside the page and guides you through the league basics and then one club at a time.</p>
        <div id="custom-builder-shell" class="custom-builder-shell is-hidden">
            <form id="custom-league-form" method="post" action=")SEL" << (newSeasonMode ? "/action/new-season-custom" : "/action/select-custom-league") << R"SEL(">
                <div class="custom-builder-toolbar">
                    <div>
                        <label for="league-name">League Name</label>
                        <input id="league-name" type="text" name="league_name" value="Custom League" required>
                    </div>
                    <div>
                        <label for="team-count">Number Of Teams</label>
                        <input id="team-count" type="number" name="team_count" min="2" max="20" value="4" required>
                    </div>
                </div>
                <div class="button-row">
                    <button class="button alt" type="button" id="refresh-custom-builder">Update Team Forms</button>
                    <button class="button ghost" type="button" id="close-custom-builder">Hide Builder</button>
                    <button class="button" type="submit">Start Custom League</button>
                </div>
                <p class="form-help">No browser popups here. Start with the league basics, then move through the clubs one section at a time.</p>
                <div class="custom-builder-status" id="custom-builder-status"></div>
                <div class="custom-builder-nav">
                    <button class="button ghost" type="button" id="custom-builder-prev">Previous Club</button>
                    <span class="custom-builder-step" id="custom-builder-step">Club 1 of 4</span>
                    <button class="button ghost" type="button" id="custom-builder-next">Next Club</button>
                </div>
                <div id="custom-team-builder" class="team-builder-stack"></div>
            </form>
        </div>
    </section>)SEL";

    return body.str();
}

string renderLeagueSelectPage(bool newSeasonMode) {
    string intro = newSeasonMode
        ? "Pick the next competition for the new season or create your own league."
        : "Choose a league and start your season.";
    string headline = newSeasonMode ? "Choose The Next League" : "Launch Your League";

    stringstream body;
    body << R"SEL(<section class="hero">
        <div class="hero-top">
            <span class="eyebrow">League Setup</span>
            <div class="hero-actions">
                <a class="edge-icon-link" href="/settings" aria-label="Open settings" title="Settings">
                    <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.9" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
                        <circle cx="12" cy="12" r="3.1"></circle>
                        <path d="M19.4 15a1 1 0 0 0 .2 1.1l.1.1a1.2 1.2 0 0 1 0 1.7l-1.6 1.6a1.2 1.2 0 0 1-1.7 0l-.1-.1a1 1 0 0 0-1.1-.2 1 1 0 0 0-.6.9v.3A1.2 1.2 0 0 1 13.4 22h-2.8a1.2 1.2 0 0 1-1.2-1.2v-.2a1 1 0 0 0-.7-1 1 1 0 0 0-1.1.2l-.1.1a1.2 1.2 0 0 1-1.7 0l-1.6-1.6a1.2 1.2 0 0 1 0-1.7l.1-.1a1 1 0 0 0 .2-1.1 1 1 0 0 0-.9-.6h-.3A1.2 1.2 0 0 1 2 13.4v-2.8a1.2 1.2 0 0 1 1.2-1.2h.2a1 1 0 0 0 1-.7 1 1 0 0 0-.2-1.1l-.1-.1a1.2 1.2 0 0 1 0-1.7l1.6-1.6a1.2 1.2 0 0 1 1.7 0l.1.1a1 1 0 0 0 1.1.2 1 1 0 0 0 .6-.9v-.3A1.2 1.2 0 0 1 10.6 2h2.8a1.2 1.2 0 0 1 1.2 1.2v.2a1 1 0 0 0 .7 1 1 1 0 0 0 1.1-.2l.1-.1a1.2 1.2 0 0 1 1.7 0l1.6 1.6a1.2 1.2 0 0 1 0 1.7l-.1.1a1 1 0 0 0-.2 1.1 1 1 0 0 0 .9.6h.3A1.2 1.2 0 0 1 22 10.6v2.8a1.2 1.2 0 0 1-1.2 1.2h-.2a1 1 0 0 0-1 .6z"></path>
                    </svg>
                </a>
                <form method="post" action="/action/shutdown">
                    <button class="button edge-exit" type="submit">Exit</button>
                </form>
            </div>
        </div>
        <h1>)SEL" << escapeHtml(headline) << R"SEL(</h1>
        <p>)SEL" << escapeHtml(intro) << R"SEL(</p>
    </section>)SEL";
    body << renderLeagueChoiceSections(newSeasonMode);

    return renderPageShell(newSeasonMode ? "Choose League" : "League Selection", newSeasonMode ? "new-season" : "", body.str(), customLeagueWizardScript());
}

string renderResumeChoicePage(const SaveFileSummary& summary) {
    stringstream body;
    body << R"LOAD(<section class="hero">
        <div class="hero-top">
            <span class="eyebrow">Saved Progress</span>
            <form method="post" action="/action/shutdown">
                <button class="button edge-exit" type="submit">Exit</button>
            </form>
        </div>
        <h1>Pick Up Where You Left Off</h1>
        <p>We found saved progress from an earlier session. Load it to continue right away, or start a fresh league without deleting the saved file.</p>
    </section>)LOAD";

    if (summary.valid) {
        body << R"LOAD(<section class="panel">
            <div class="panel-head">
                <h2>Saved Session Found</h2>
                <p>Your last saved session is ready to load.</p>
            </div>
            <div class="stat-grid">
                <div class="mini-card"><span class="mini-label">League</span><strong>)LOAD" << escapeHtml(summary.leagueName) << R"LOAD(</strong></div>
                <div class="mini-card"><span class="mini-label">Season</span><strong>)LOAD" << summary.currentSeason << R"LOAD(</strong></div>
                <div class="mini-card"><span class="mini-label">Saved</span><strong>)LOAD" << escapeHtml(summary.savedAt.empty() ? "Recently" : summary.savedAt) << R"LOAD(</strong></div>
            </div>
        </section>)LOAD";
    }

    body << R"LOAD(<div class="grid two-col">
        <section class="form-card">
            <div class="panel-head">
                <h2>Load Saved Progress</h2>
                <p>)LOAD" << escapeHtml(summary.valid
                    ? ("Resume " + summary.leagueName + " in Season " + to_string(summary.currentSeason) + ".")
                    : "Resume the most recent saved session.") << R"LOAD(</p>
            </div>
            <form method="post" action="/action/load-progress">
                <input type="hidden" name="return_to" value="/">
                <button class="button" type="submit">Load Saved Progress</button>
            </form>
        </section>
        <section class="form-card">
            <div class="panel-head">
                <h2>Start New Session</h2>
                <p>Open the league setup and start fresh. Your saved file stays available until you overwrite it later.</p>
            </div>
            <form method="post" action="/action/start-new-session">
                <button class="button alt" type="submit">Create New Session</button>
            </form>
        </section>
    </div>)LOAD";

    return renderPageShell("Saved Progress", "", body.str());
}

string renderDashboardPage() {
    if (teams.empty()) {
        SaveFileSummary summary = readSaveFileSummary();
        if (summary.valid && !startupResumePromptDismissed) return renderResumeChoicePage(summary);
        return renderLeagueSelectPage(false);
    }

    int topGoals = 0;
    string topScorer = getTopScorer(topGoals);
    if (topGoals == 0) topScorer = "No goals yet";

    stringstream body;
    body << R"DASH(<section class="hero">
        <span class="eyebrow">League Simulator</span>
        <h1>)DASH" << escapeHtml(leagueName) << R"DASH( Dashboard</h1>
        <p>Everything you need for the season is here, from matchdays and cup rounds to transfers and team reports.</p>
        <div class="stat-grid">
            <div class="mini-card"><span class="mini-label">Season</span><strong>)DASH" << currentSeason << R"DASH(</strong></div>
            <div class="mini-card"><span class="mini-label">Teams</span><strong>)DASH" << teams.size() << R"DASH(</strong></div>
            <div class="mini-card"><span class="mini-label">League</span><strong>)DASH" << escapeHtml(leagueFinishedThisSeason ? "Completed" : ("Matchday " + to_string(currentLeagueMatchdayNumber()) + " next")) << R"DASH(</strong></div>
            <div class="mini-card"><span class="mini-label">Cup</span><strong>)DASH" << escapeHtml(cupFinishedThisSeason ? ("Winner: " + cupWinnerThisSeason) : (cupStartedThisSeason ? ("Round " + to_string(cupRoundNumber) + " next") : "Not started")) << R"DASH(</strong></div>
            <div class="mini-card"><span class="mini-label">Top Scorer</span><strong>)DASH" << escapeHtml(topScorer) << R"DASH(</strong></div>
        </div>
    </section>
    <section class="panel">
        <div class="panel-head">
            <h2>Main Menu</h2>
            <p>Pick an area to continue your season.</p>
        </div>
        <div class="card-grid">
            <a class="card" href="/league"><span class="pill">League</span><div class="card-title">Run League Season</div><div class="card-copy">Play the next matchday or finish the rest of the season.</div></a>
            <a class="card" href="/cup"><span class="pill">Cup</span><div class="card-title">Run Cup Competition</div><div class="card-copy">Advance one round at a time or finish the whole cup.</div></a>
            <a class="card" href="/transfers"><span class="pill">Transfers</span><div class="card-title">Transfer Market</div><div class="card-copy">List players, make offers and manage budgets.</div></a>
            <a class="card" href="/standings"><span class="pill">Standings</span><div class="card-title">View League Table</div><div class="card-copy">Check the full standings with live points and goal difference.</div></a>
            <a class="card" href="/scorers"><span class="pill">Scorers</span><div class="card-title">View Top Scorers</div><div class="card-copy">Track the golden boot race.</div></a>
            <a class="card" href="/injuries"><span class="pill">Injuries</span><div class="card-title">View Injury Report</div><div class="card-copy">See which players are unavailable right now.</div></a>
            <a class="card" href="/squads"><span class="pill">Squads</span><div class="card-title">View Squad</div><div class="card-copy">Open any team squad with starters, subs and status.</div></a>
            <a class="card" href="/history"><span class="pill">History</span><div class="card-title">View Season History</div><div class="card-copy">Review completed seasons and winners.</div></a>
            <a class="card" href="/new-season"><span class="pill">New Season</span><div class="card-title">Start New Season</div><div class="card-copy">Reset the campaign or switch leagues from the browser.</div></a>
            <a class="card" href="/settings"><span class="pill">Settings</span><div class="card-title">Settings</div><div class="card-copy">Toggle fast display and close the app when you are done.</div></a>
        </div>
    </section>)DASH";

    return renderPageShell("Dashboard", "dashboard", body.str());
}

string renderResetGamePage() {
    if (teams.empty()) return renderLeagueSelectPage(false);

    stringstream body;
    body << R"RESET(<section class="hero">
        <span class="eyebrow">Reset Game</span>
        <h1>Start Over With Another League?</h1>
        <p>You can save your current progress first, reset without saving, or cancel and keep playing this season.</p>
    </section>
    <div class="grid two-col">
        <section class="form-card">
            <div class="panel-head">
                <h2>Save And Reset</h2>
                <p>Store the current season in the save slot, then return to league selection.</p>
            </div>
            <form method="post" action="/action/reset-game-save">
                <button class="button" type="submit">Save And Reset</button>
            </form>
        </section>
        <section class="form-card">
            <div class="panel-head">
                <h2>Reset Without Saving</h2>
                <p>Clear the current session immediately and choose a different league right away.</p>
            </div>
            <form method="post" action="/action/reset-game-now">
                <button class="button danger" type="submit">Reset Without Saving</button>
            </form>
        </section>
    </div>
    <section class="panel">
        <div class="panel-head">
            <h2>Keep This Session</h2>
            <p>Go back to the dashboard if you do not want to reset right now.</p>
        </div>
        <a class="button ghost" href="/">Cancel</a>
    </section>)RESET";

    return renderPageShell("Reset Game", "dashboard", body.str());
}

string renderStandingsWebPage() {
    stringstream body;
    body << R"TABLE(<section class="hero">
        <span class="eyebrow">League Table</span>
        <h1>)TABLE" << escapeHtml(leagueName) << R"TABLE(</h1>
        <p>Season )TABLE" << currentSeason << R"TABLE( standings.</p>
    </section>
    <section class="table-wrap"><table>
        <thead><tr>
            <th>#</th><th>Team</th><th class="center">P</th><th class="center">W</th><th class="center">D</th><th class="center">L</th>
            <th class="center">GF</th><th class="center">GA</th><th class="center">GD</th><th class="center">Pts</th>
        </tr></thead><tbody>)TABLE";

    vector<int> order = sortedTeamOrder();
    for (int i = 0; i < (int)order.size(); i++) {
        const Team& team = teams[order[i]];
        int played = team.wins + team.draws + team.losses;
        int gd = team.goalsScored - team.goalsConceded;
        body << "<tr>";
        body << "<td>" << (i + 1) << "</td>";
        body << "<td>" << escapeHtml(team.name) << "</td>";
        body << "<td class=\"center\">" << played << "</td>";
        body << "<td class=\"center\">" << team.wins << "</td>";
        body << "<td class=\"center\">" << team.draws << "</td>";
        body << "<td class=\"center\">" << team.losses << "</td>";
        body << "<td class=\"center\">" << team.goalsScored << "</td>";
        body << "<td class=\"center\">" << team.goalsConceded << "</td>";
        body << "<td class=\"center\">" << (gd >= 0 ? "+" : "") << gd << "</td>";
        body << "<td class=\"center\"><strong>" << team.points << "</strong></td>";
        body << "</tr>";
    }

    body << R"TABLE(</tbody></table></section>)TABLE";
    return renderPageShell("Standings", "standings", body.str());
}

string renderScorersWebPage() {
    struct ScorerRow { string name; string team; int goals; int assists; };
    vector<ScorerRow> scorers;
    for (const auto& team : teams) {
        for (const auto& player : team.players) {
            if (player.goals > 0) scorers.push_back({ player.name, team.name, player.goals, player.assists });
        }
    }
    sort(scorers.begin(), scorers.end(), [](const ScorerRow& a, const ScorerRow& b) {
        if (a.goals != b.goals) return a.goals > b.goals;
        return a.assists > b.assists;
    });

    stringstream body;
    body << R"SCORERS(<section class="hero">
        <span class="eyebrow">Top Scorers</span>
        <h1>Golden Boot Race</h1>
        <p>Current scoring leaders in )SCORERS" << escapeHtml(leagueName) << R"SCORERS(.</p>
    </section>
    <section class="table-wrap"><table>
        <thead><tr><th>#</th><th>Player</th><th>Team</th><th class="center">Goals</th><th class="center">Assists</th></tr></thead>
        <tbody>)SCORERS";

    if (scorers.empty()) {
        body << "<tr><td colspan=\"5\" class=\"empty\">No goals yet this season.</td></tr>";
    } else {
        for (int i = 0; i < (int)scorers.size() && i < 15; i++) {
            body << "<tr><td>" << (i + 1) << "</td><td>" << escapeHtml(scorers[i].name) << "</td><td>"
                 << escapeHtml(scorers[i].team) << "</td><td class=\"center\">" << scorers[i].goals
                 << "</td><td class=\"center\">" << scorers[i].assists << "</td></tr>";
        }
    }

    body << R"SCORERS(</tbody></table></section>)SCORERS";
    return renderPageShell("Top Scorers", "scorers", body.str());
}

string renderInjuriesWebPage() {
    stringstream body;
    body << R"INJ(<section class="hero">
        <span class="eyebrow">Injuries</span>
        <h1>Medical Report</h1>
        <p>Live availability status for every player in the league.</p>
    </section>
    <section class="table-wrap"><table>
        <thead><tr><th>Player</th><th>Team</th><th class="center">Games Left</th></tr></thead><tbody>)INJ";

    bool any = false;
    for (const auto& team : teams) {
        for (const auto& player : team.players) {
            if (!player.injured) continue;
            any = true;
            body << "<tr><td>" << escapeHtml(player.name) << "</td><td>" << escapeHtml(team.name)
                 << "</td><td class=\"center\">" << player.injuryGamesLeft << "</td></tr>";
        }
    }

    if (!any) {
        body << "<tr><td colspan=\"3\" class=\"empty\">All players are fit right now.</td></tr>";
    }

    body << R"INJ(</tbody></table></section>)INJ";
    return renderPageShell("Injuries", "injuries", body.str());
}

string renderHistoryWebPage() {
    stringstream body;
    body << R"HIST(<section class="hero">
        <span class="eyebrow">Season History</span>
        <h1>Past Winners</h1>
        <p>Every completed season tracked by the simulator appears here.</p>
    </section>
    <section class="table-wrap"><table>
        <thead><tr><th>Season</th><th>League Winner</th><th>Cup Winner</th><th>Top Scorer</th><th class="center">Goals</th></tr></thead>
        <tbody>)HIST";

    if (history.empty()) {
        body << "<tr><td colspan=\"5\" class=\"empty\">No seasons have been recorded yet.</td></tr>";
    } else {
        for (auto it = history.rbegin(); it != history.rend(); ++it) {
            body << "<tr><td>" << it->season << "</td><td>" << escapeHtml(it->leagueWinner)
                 << "</td><td>" << escapeHtml(it->cupWinner) << "</td><td>" << escapeHtml(it->topScorer)
                 << "</td><td class=\"center\">" << it->topScorerGoals << "</td></tr>";
        }
    }

    body << R"HIST(</tbody></table></section>)HIST";
    return renderPageShell("History", "history", body.str());
}

string renderSquadsWebPage() {
    stringstream body;
    body << R"SQUADS(<section class="hero">
        <span class="eyebrow">Squads</span>
        <h1>Choose A Team</h1>
        <p>Open any club to inspect starters, substitutes, injuries and transfer status.</p>
    </section>
    <section class="panel">
        <div class="card-grid">)SQUADS";

    for (int i = 0; i < (int)teams.size(); i++) {
        body << R"SQUADS(
            <a class="card" href="/squads/)SQUADS" << (i + 1) << R"SQUADS(">
                <span class="pill">Team )SQUADS" << (i + 1) << R"SQUADS(</span>
                <div class="card-title">)SQUADS" << escapeHtml(teams[i].name) << R"SQUADS(</div>
                <div class="card-copy">)SQUADS" << escapeHtml(teams[i].city) << " - " << escapeHtml(teams[i].stadium) << R"SQUADS(</div>
            </a>)SQUADS";
    }

    body << R"SQUADS(
        </div>
    </section>)SQUADS";
    return renderPageShell("Squads", "squads", body.str());
}

string renderSingleSquadWebPage(int teamIdx) {
    if (teamIdx < 0 || teamIdx >= (int)teams.size()) {
        return renderPageShell("Squad", "squads", "<section class=\"panel\"><h2>Team not found.</h2></section>");
    }

    const Team& team = teams[teamIdx];
    stringstream body;
    body << R"SINGLE(<section class="hero">
        <span class="eyebrow">Squad View</span>
        <h1>)SINGLE" << escapeHtml(team.name) << R"SINGLE(</h1>
        <p>)SINGLE" << escapeHtml(team.city) << " - " << escapeHtml(team.stadium) << " | Strength "
         << team.strength << "/10 | Budget $" << team.budget << R"SINGLE(M</p>
    </section>
    <section class="table-wrap"><table>
        <thead><tr><th>#</th><th>Player</th><th>Pos</th><th>Role</th><th class="center">G</th><th class="center">A</th><th>Status</th></tr></thead>
        <tbody>)SINGLE";

    for (int i = 0; i < (int)team.players.size(); i++) {
        const Player& player = team.players[i];
        string status = player.injured ? ("Injured (" + to_string(player.injuryGamesLeft) + ")")
                                       : player.suspended ? "Suspended"
                                                          : "Fit";
        if (player.listedForSale) status += " | Listed for " + to_string(player.askingPrice) + "M";

        body << "<tr><td>" << (i + 1) << "</td><td>" << escapeHtml(player.name) << "</td><td>"
             << escapeHtml(player.position) << "</td><td>" << (player.isSubstitute ? "Substitute" : "Starter")
             << "</td><td class=\"center\">" << player.goals << "</td><td class=\"center\">" << player.assists
             << "</td><td>" << escapeHtml(status) << "</td></tr>";
    }

    body << R"SINGLE(</tbody></table></section>)SINGLE";
    return renderPageShell(team.name + " Squad", "squads", body.str());
}

string renderLeaguePage() {
    string status = teams.empty() ? "Choose a league first."
        : leagueFinishedThisSeason ? ("Season complete. " + getLeagueWinner() + " are champions.")
        : ("Next up: Matchday " + to_string(currentLeagueMatchdayNumber()) + ".");

    stringstream body;
    body << R"LEAGUE(<section class="hero">
        <span class="eyebrow">League Season</span>
        <h1>Run League Season</h1>
        <p>)LEAGUE" << escapeHtml(status) << R"LEAGUE(</p>
    </section>
    <section class="panel">
        <div class="panel-head">
            <h2>Simulation Controls</h2>
            <p>Advance one matchday at a time or simulate the rest of the season in one click.</p>
        </div>
        <div class="button-row">
            <form method="post" action="/action/league-next"><button class="button" type="submit">Play Next Matchday</button></form>
            <form method="post" action="/action/league-finish"><button class="button alt" type="submit">Simulate Rest Of Season</button></form>
            <a class="button ghost" href="/standings">Open Standings</a>
        </div>
    </section>)LEAGUE";
    body << renderActionReport(lastLeagueAction);
    return renderPageShell("Run League Season", "league", body.str());
}

string renderCupPage() {
    string status = teams.empty() ? "Choose a league first."
        : cupFinishedThisSeason ? (cupWinnerThisSeason + " have won the cup.")
        : cupStartedThisSeason ? ("Next up: Cup Round " + to_string(cupRoundNumber) + ".")
                               : "The cup has not started yet.";

    stringstream body;
    body << R"CUP(<section class="hero">
        <span class="eyebrow">Cup Competition</span>
        <h1>Run Cup Competition</h1>
        <p>)CUP" << escapeHtml(status) << R"CUP(</p>
    </section>
    <section class="panel">
        <div class="panel-head">
            <h2>Simulation Controls</h2>
            <p>Advance one round at a time or finish the whole cup.</p>
        </div>
        <div class="button-row">
            <form method="post" action="/action/cup-next"><button class="button" type="submit">Play Next Round</button></form>
            <form method="post" action="/action/cup-finish"><button class="button alt" type="submit">Simulate Rest Of Cup</button></form>
        </div>
    </section>)CUP";
    body << renderActionReport(lastCupAction);
    return renderPageShell("Cup Competition", "cup", body.str());
}

string renderTransfersPage() {
    vector<pair<int, int>> listed = listedPlayerRefs();
    stringstream listPlayerOptions;
    stringstream listedPlayerOptions;

    for (int t = 0; t < (int)teams.size(); t++) {
        for (int p = 0; p < (int)teams[t].players.size(); p++) {
            const Player& player = teams[t].players[p];
            listPlayerOptions << "<option value=\"" << t << ":" << p << "\" data-team=\"" << t << "\">"
                              << escapeHtml(teams[t].name + " - " + player.name + " (" + player.position + ")")
                              << "</option>";
            if (player.listedForSale) {
                listedPlayerOptions << "<option value=\"" << t << ":" << p << "\" data-seller=\"" << t << "\">"
                                    << escapeHtml(player.name + " | " + teams[t].name + " | Ask " + to_string(player.askingPrice) + "M")
                                    << "</option>";
            }
        }
    }

    stringstream body;
    body << R"TRANS(<section class="hero">
        <span class="eyebrow">Transfers</span>
        <h1>Transfer Market</h1>
        <p>List players for sale, make offers and manage club budgets.</p>
    </section>
    <div class="grid two-col">
        <section class="form-card">
            <div class="panel-head">
                <h2>List A Player</h2>
                <p>Choose a club, then pick a player and asking price.</p>
            </div>
            <form method="post" action="/action/list-player">
                <label for="list-team">Selling team</label>
                <select id="list-team" name="team_idx" onchange="filterPlayerOptions('list-team','list-player')" required>)TRANS";

    for (int i = 0; i < (int)teams.size(); i++) {
        body << "<option value=\"" << i << "\">" << escapeHtml(teams[i].name) << "</option>";
    }

    body << R"TRANS(</select>
                <label for="list-player">Player</label>
                <select id="list-player" name="player_ref" required>)TRANS" << listPlayerOptions.str() << R"TRANS(</select>
                <label for="asking-price">Asking price (M)</label>
                <input id="asking-price" type="number" min="1" name="asking_price" value="20" required>
                <button class="button" type="submit">List Player</button>
            </form>
        </section>
        <section class="form-card">
            <div class="panel-head">
                <h2>Buy A Listed Player</h2>
                <p>Pick the buying team, choose a listed player and submit an offer.</p>
            </div>
            <form method="post" action="/action/buy-player">
                <label for="buy-team">Buying team</label>
                <select id="buy-team" name="buyer_idx" required>)TRANS";

    for (int i = 0; i < (int)teams.size(); i++) {
        body << "<option value=\"" << i << "\">" << escapeHtml(teams[i].name) << " | Budget " << teams[i].budget << "M</option>";
    }

    body << R"TRANS(</select>
                <label for="buy-player">Listed player</label>
                <select id="buy-player" name="player_ref" required>)TRANS" << listedPlayerOptions.str() << R"TRANS(</select>
                <label for="offer">Offer (M)</label>
                <input id="offer" type="number" min="1" name="offer" value="20" required>
                <button class="button alt" type="submit">Submit Offer</button>
            </form>
        </section>
    </div>
    <section class="table-wrap" style="margin-top:16px;">
        <div class="panel-head">
            <h2>Players For Sale</h2>
            <p>Current transfer list.</p>
        </div>
        <table>
            <thead><tr><th>Player</th><th>Team</th><th>Pos</th><th class="center">Value</th><th class="center">Asking</th></tr></thead>
            <tbody>)TRANS";

    if (listed.empty()) {
        body << "<tr><td colspan=\"5\" class=\"empty\">No players are currently listed for sale.</td></tr>";
    } else {
        for (const auto& ref : listed) {
            const Player& player = teams[ref.first].players[ref.second];
            body << "<tr><td>" << escapeHtml(player.name) << "</td><td>" << escapeHtml(teams[ref.first].name)
                 << "</td><td>" << escapeHtml(player.position) << "</td><td class=\"center\">" << player.marketValue
                 << "M</td><td class=\"center\">" << player.askingPrice << "M</td></tr>";
        }
    }

    body << R"TRANS(</tbody></table>
    </section>
    <section class="table-wrap" style="margin-top:16px;">
        <div class="panel-head">
            <h2>Team Budgets</h2>
            <p>Budgets update immediately after every transfer.</p>
        </div>
        <table>
            <thead><tr><th>Team</th><th class="center">Budget</th></tr></thead>
            <tbody>)TRANS";

    for (const auto& team : teams) {
        body << "<tr><td>" << escapeHtml(team.name) << "</td><td class=\"center\">" << team.budget << "M</td></tr>";
    }

    body << R"TRANS(</tbody></table>
    </section>)TRANS";

    string script = R"JS(
function filterPlayerOptions(teamSelectId, playerSelectId) {
    const teamSelect = document.getElementById(teamSelectId);
    const playerSelect = document.getElementById(playerSelectId);
    const teamValue = teamSelect.value;
    let firstVisible = null;

    Array.from(playerSelect.options).forEach(option => {
        const visible = option.dataset.team === teamValue || !option.dataset.team;
        option.hidden = !visible;
        if (visible && firstVisible === null) firstVisible = option.value;
    });

    if (firstVisible !== null) {
        playerSelect.value = firstVisible;
    }
}
filterPlayerOptions('list-team', 'list-player');
)JS";

    return renderPageShell("Transfer Market", "transfers", body.str(), script);
}

string renderNewSeasonPage() {
    if (teams.empty()) return renderLeagueSelectPage(false);

    stringstream body;
    body << R"NEW(<section class="hero">
        <span class="eyebrow">New Season</span>
        <h1>Start New Season</h1>
        <p>Move into season )NEW" << (currentSeason + 1) << R"NEW(, keep your current league or switch to a new one.</p>
    </section>
    <section class="panel">
        <div class="panel-head">
            <h2>Keep The Current League</h2>
            <p>Carry the current clubs and transfers into the next season.</p>
        </div>
        <form method="post" action="/action/new-season-continue">
            <button class="button" type="submit">Start Season )NEW" << (currentSeason + 1) << " In " << escapeHtml(leagueName) << R"NEW(</button>
        </form>
    </section>)NEW";

    body << renderLeagueChoiceSections(true);
    return renderPageShell("Start New Season", "new-season", body.str(), customLeagueWizardScript());
}

string renderSettingsPage() {
    SaveFileSummary summary = readSaveFileSummary();
    bool hasActiveSession = !teams.empty();
    string mainMenuHref = hasActiveSession ? "/" : "/league-select";

    stringstream body;
    body << R"SET(<section class="hero">
        <span class="eyebrow">Settings</span>
        <h1>Settings</h1>
        <p>Adjust a few app settings, save your progress, and close the game when you are done.</p>
        <div class="button-row" style="margin-top: 18px;">
            <a class="button ghost" href=")SET" << mainMenuHref << R"SET(">Back To Main Menu</a>
        </div>
    </section>
    <div class="grid two-col">
        <section class="form-card">
            <div class="panel-head">
                <h2>Fast Display</h2>
                <p>ON shows match reports instantly. OFF plays a slower minute-by-minute walkthrough with color-coded match events.</p>
            </div>
            <div class="button-row">
                <form method="post" action="/action/toggle-fast">
                    <input type="hidden" name="return_to" value="/settings">
                    <button class="button" type="submit">Fast Display: )SET" << (fastDisplay ? "ON" : "OFF") << R"SET(</button>
                </form>
            </div>
            <p class="form-help">Use OFF for the slower minute-by-minute walkthrough with color-coded match events.</p>
        </section>)SET";

    body << R"SET(<section class="form-card">
            <div class="panel-head">
                <h2>Save And Load</h2>
                <p>Keep your season progress and come back to it the next time you open the simulator.</p>
            </div>
            <div class="button-row">)SET";

    if (hasActiveSession) {
        body << R"SET(<form method="post" action="/action/save-progress">
                <input type="hidden" name="return_to" value="/settings">
                <button class="button" type="submit">Save Progress</button>
            </form>)SET";
    }
    else {
        body << R"SET(<span class="pill">No Active Session Yet</span>)SET";
    }

    if (summary.valid) {
        body << R"SET(<form method="post" action="/action/load-progress">
                <input type="hidden" name="return_to" value="/settings">
                <button class="button alt" type="submit">Load Saved Progress</button>
            </form>)SET";
    }
    else {
        body << R"SET(<span class="pill">Save Slot Empty</span>)SET";
    }

    body << R"SET(<form method="post" action="/action/reset-save">
                <input type="hidden" name="return_to" value="/settings">
                <button class="button danger" type="submit">Reset Save Slot</button>
            </form>)SET";

    body << R"SET(</div>)SET";

    if (summary.valid) {
        body << R"SET(<p class="form-help">Saved session: )SET" << escapeHtml(summary.leagueName)
            << " | Season " << summary.currentSeason
            << " | Last saved " << escapeHtml(summary.savedAt.empty() ? "Recently" : summary.savedAt)
            << R"SET(</p>)SET";
    }

    if (!hasActiveSession) {
        body << R"SET(<p class="note">Start a league before saving. You can still load an existing save from here.</p>)SET";
    }

    body << R"SET(</section>
    </div>)SET";

    body << R"SET(<section class="form-card">
            <div class="panel-head">
                <h2>Game Reset</h2>
                <p>Start over with another league. You will be asked whether you want to save first.</p>
            </div>)SET";

    if (hasActiveSession) {
        body << R"SET(<a class="button danger" href="/reset-game">Reset Game</a>)SET";
    }
    else {
        body << R"SET(<span class="pill">No Active Session Yet</span>
            <p class="form-help">Load or start a league first if you want to reset the current session.</p>)SET";
    }

    body << R"SET(</section>
    <section class="form-card">
            <div class="panel-head">
                <h2>Open Link</h2>
                <p>If the page does not open automatically, use this address: )SET" << escapeHtml(currentServerUrl) << R"SET(</p>
            </div>
            <form method="post" action="/action/shutdown">
                <button class="button danger" type="submit">Exit Application</button>
            </form>
        </section>
    )SET";
    return renderPageShell("Settings", "settings", body.str());
}

void openUrl(const string& url) {
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
    system(("xdg-open \"" + url + "\"").c_str());
#endif
}

void startWebApp() {
    httplib::Server server;
    server.new_task_queue = [] { return new httplib::ThreadPool(1); };

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderDashboardPage(), "text/html; charset=UTF-8");
    });

    server.Get("/league-select", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderLeagueSelectPage(false), "text/html; charset=UTF-8");
    });

    server.Get("/league", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderLeaguePage(), "text/html; charset=UTF-8");
    });

    server.Get("/cup", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderCupPage(), "text/html; charset=UTF-8");
    });

    server.Get("/transfers", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderTransfersPage(), "text/html; charset=UTF-8");
    });

    server.Get("/standings", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderStandingsWebPage(), "text/html; charset=UTF-8");
    });

    server.Get("/scorers", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderScorersWebPage(), "text/html; charset=UTF-8");
    });

    server.Get("/injuries", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderInjuriesWebPage(), "text/html; charset=UTF-8");
    });

    server.Get("/squads", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderSquadsWebPage(), "text/html; charset=UTF-8");
    });

    server.Get(R"(/squads/(\d+))", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        int teamIdx = toInt(string(req.matches[1]), 1) - 1;
        res.set_content(renderSingleSquadWebPage(teamIdx), "text/html; charset=UTF-8");
    });

    server.Get("/history", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderHistoryWebPage(), "text/html; charset=UTF-8");
    });

    server.Get("/new-season", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderNewSeasonPage(), "text/html; charset=UTF-8");
    });

    server.Get("/reset-game", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderResetGamePage(), "text/html; charset=UTF-8");
    });

    server.Get("/settings", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        res.set_content(renderSettingsPage(), "text/html; charset=UTF-8");
    });

    server.Post("/action/select-preset-league", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        int choice = toInt(paramValue(req, "choice", "1"), 1);
        loadPresetLeague(choice);
        prepareLoadedLeagueState();
        setNotice(leagueName + " loaded successfully.", "success");
        res.set_redirect("/", 303);
    });

    server.Post("/action/select-custom-league", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        currentSeason = 1;
        history.clear();
        loadCustomLeagueFromRequest(req);
        startupResumePromptDismissed = true;
        setNotice(leagueName + " loaded successfully.", "success");
        res.set_redirect("/", 303);
    });

    server.Post("/action/league-next", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        simulateLeagueWeb(false);
        res.set_redirect("/league", 303);
    });

    server.Post("/action/league-finish", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        simulateLeagueWeb(true);
        res.set_redirect("/league", 303);
    });

    server.Post("/action/cup-next", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        simulateCupWeb(false);
        res.set_redirect("/cup", 303);
    });

    server.Post("/action/cup-finish", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        simulateCupWeb(true);
        res.set_redirect("/cup", 303);
    });

    server.Post("/action/list-player", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        int teamIdx = toInt(paramValue(req, "team_idx", "-1"), -1);
        pair<int, int> ref = parsePlayerRef(paramValue(req, "player_ref"));
        string message;
        bool ok = false;
        if (teamIdx == ref.first) {
            ok = listPlayerForSaleWeb(teamIdx, ref.second, toInt(paramValue(req, "asking_price", "0"), 0), message);
        }
        if (!ok && message.empty()) message = "Choose a player from the selected team.";
        setNotice(message, ok ? "success" : "error");
        res.set_redirect("/transfers", 303);
    });

    server.Post("/action/buy-player", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        int buyerIdx = toInt(paramValue(req, "buyer_idx", "-1"), -1);
        pair<int, int> ref = parsePlayerRef(paramValue(req, "player_ref"));
        string message;
        bool ok = buyPlayerWeb(buyerIdx, ref.first, ref.second, toInt(paramValue(req, "offer", "0"), 0), message);
        setNotice(message, ok ? "success" : "error");
        res.set_redirect("/transfers", 303);
    });

    server.Post("/action/new-season-continue", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        if (leagueFinishedThisSeason || cupFinishedThisSeason) syncSeasonRecord();
        currentSeason++;
        prepareLoadedLeagueState();
        setNotice("Season " + to_string(currentSeason) + " started in " + leagueName + ".", "success");
        res.set_redirect("/", 303);
    });

    server.Post("/action/new-season-preset", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        if (leagueFinishedThisSeason || cupFinishedThisSeason) syncSeasonRecord();
        currentSeason++;
        int choice = toInt(paramValue(req, "choice", "1"), 1);
        loadPresetLeague(choice);
        prepareLoadedLeagueState();
        setNotice("Season " + to_string(currentSeason) + " started in " + leagueName + ".", "success");
        res.set_redirect("/", 303);
    });

    server.Post("/action/new-season-custom", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        if (leagueFinishedThisSeason || cupFinishedThisSeason) syncSeasonRecord();
        currentSeason++;
        loadCustomLeagueFromRequest(req);
        setNotice("Season " + to_string(currentSeason) + " started in " + leagueName + ".", "success");
        res.set_redirect("/", 303);
    });

    server.Post("/action/save-progress", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        string message;
        bool ok = saveProgressToFile(message);
        setNotice(message, ok ? "success" : "error");
        res.set_redirect(sanitizeReturnPath(paramValue(req, "return_to", "/settings"), "/settings"), 303);
    });

    server.Post("/action/load-progress", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        string message;
        bool ok = loadProgressFromFile(message);
        setNotice(message, ok ? "success" : "error");
        string returnTo = sanitizeReturnPath(paramValue(req, "return_to", "/"), "/");
        res.set_redirect(ok ? "/" : returnTo, 303);
    });

    server.Post("/action/reset-save", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        string message;
        bool ok = resetSaveFile(message);
        setNotice(message, ok ? "success" : "error");
        res.set_redirect(sanitizeReturnPath(paramValue(req, "return_to", "/settings"), "/settings"), 303);
    });

    server.Post("/action/start-new-session", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        startupResumePromptDismissed = true;
        setNotice("Choose a league and start a fresh session whenever you are ready.", "info");
        res.set_redirect("/league-select", 303);
    });

    server.Post("/action/reset-game-save", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        string message;
        if (!saveProgressToFile(message)) {
            setNotice(message, "error");
            res.set_redirect("/reset-game", 303);
            return;
        }
        resetCurrentSessionState();
        setNotice("Progress saved and current session cleared. Choose another league to begin.", "success");
        res.set_redirect("/league-select", 303);
    });

    server.Post("/action/reset-game-now", [](const httplib::Request&, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        resetCurrentSessionState();
        setNotice("Current session cleared. Choose another league to begin.", "success");
        res.set_redirect("/league-select", 303);
    });

    server.Post("/action/toggle-fast", [](const httplib::Request& req, httplib::Response& res) {
        lock_guard<mutex> lock(stateMutex);
        fastDisplay = !fastDisplay;
        string returnTo = sanitizeReturnPath(paramValue(req, "return_to", "/settings"), "/settings");
        setNotice("Fast display is now " + string(fastDisplay ? "ON - match reports appear instantly." : "OFF - minute-by-minute walkthrough mode is active."), "success");
        res.set_redirect(returnTo, 303);
    });

    server.Post("/action/shutdown", [&server](const httplib::Request&, httplib::Response& res) {
        {
            lock_guard<mutex> lock(stateMutex);
            res.set_content("<!DOCTYPE html><html><body style=\"font-family:Trebuchet MS;background:#07121b;color:#eef7ff;padding:40px;\"><h1>League Simulator closed.</h1><p>You can close this browser tab now.</p></body></html>", "text/html; charset=UTF-8");
        }
        thread([&server]() {
#ifdef _WIN32
            Sleep(250);
#else
            usleep(250000);
#endif
            server.stop();
        }).detach();
    });

    server.Get("/favicon.ico", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    int port = server.bind_to_port("127.0.0.1", 8080) ? 8080 : server.bind_to_any_port("127.0.0.1");
    if (port < 0) {
        cerr << "Failed to bind HTTP server.\n";
        return;
    }

    currentServerUrl = "http://127.0.0.1:" + to_string(port);
    ofstream("league-simulator-web-url.txt") << currentServerUrl;

    cout << BOLD << CYAN << "\nLaunching browser app at " << currentServerUrl << RESET << "\n";
    cout << "Close the console window or use Settings -> Exit Application when you are done.\n";

    openUrl(currentServerUrl);
    server.listen_after_bind();
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

    fastDisplay = false;
    readSaveFileSummary();
    startWebApp();
    return 0;
}
