# quadtree – Algorithmus und Pseudocode (Übung 2, 191.002 Betriebssysteme)

## 1. Worum geht es?

Das Programm bekommt viele 2D-Punkte über `stdin` (eine Zeile pro Punkt, z. B. `1,1`), einen Zielpunkt `goal` und eine Maximaldistanz `distance`. Es gibt alle Punkte aus, die höchstens `distance` vom Ziel entfernt sind, im Format `1.414: 1,1`.

Eine einfache Schleife würde dafür reichen. Das Ziel der Übung ist aber, **fork, exec und Pipes** zu üben. Darum muss die Suche als **Baum aus Prozessen** laufen: Jeder Prozess ist ein Knoten und startet sich selbst vier Mal neu (einmal pro Quadrant).

**Wichtig:** Es gibt keine Quadtree-Datenstruktur (keine Knoten, keine Zeiger). Der „Baum“ existiert nur als Prozessbaum, solange das Programm läuft.

**Wichtig 2 (Folie 31, „Tips for the Exercise“):** Die Eltern lesen `stdin` Zeile für Zeile und schicken jede Zeile **sofort** an das passende Kind. Es wird **nicht** erst die ganze Eingabe in ein Array gelesen und dann geforkt. So laufen Lesen und Verarbeiten parallel („Reading and processing are parallelized!“). Vor dem ersten `fork` werden nur so viele Zeilen gelesen, wie nötig sind, um zu wissen, ob es 0, 1 oder mehr verschiedene Zeilen gibt.

| normale Rekursion | in dieser Aufgabe |
|---|---|
| Funktion ruft sich selbst auf | Programm startet sich selbst (`fork` + `execlp(argv[0], ...)`) |
| Parameter (Array, Grenzen) | `stdin` (die Punkte) + Kommandozeilenargumente (Ziel, Distanz, Grenzen) |
| Rückgabewert | `stdout` (Ergebniszeilen) + Exit-Status |
| Aufrufstack | Prozessbaum |
| Abbruchbedingung `if (n <= 1) return` | 0 oder 1 verschiedene Zeile: ausgeben und beenden, kein `fork` |

---

## 2. Aufruf

```
./quadtree <goal_x>,<goal_y> <distance> <min_x> <max_x> <min_y> <max_y>
```

- Reihenfolge der Grenzen: `min_x max_x min_y max_y`.
- `argc != 7` → Usage auf **stdout**, `EXIT_FAILURE` (Testcase 08).
- Parsen mit `strtof` (alles als `float`, passend zu `sqrtf`/`powf`). Danach prüfen, dass der ganze String verbraucht wurde (`endptr`).
- Beim Ziel muss das Komma zwischen x und y geprüft werden.
- Parse-Fehler → Meldung auf **stderr** (mit `argv[0]`), `EXIT_FAILURE`.
- `goal` und `distance` bleiben immer gleich und werden 1:1 an jedes Kind weitergegeben. Nur die 4 Grenzen ändern sich.

---

## 3. Die Quadranten

Das Rechteck wird in der **eigenen Mitte** geteilt (nicht beim Ziel):

```
mid_x = (min_x + max_x) / 2
mid_y = (min_y + max_y) / 2
```

```
 max_y +-----------+-----------+
       |    TL     |    TR     |
 mid_y +-----------+-----------+
       |    BL     |    BR     |
 min_y +-----------+-----------+
     min_x       mid_x       max_x
```

| Kind | Index | min_x | max_x | min_y | max_y |
|------|-------|-------|-------|-------|-------|
| BL   | 0 | min_x | mid_x | min_y | mid_y |
| BR   | 1 | mid_x | max_x | min_y | mid_y |
| TL   | 2 | min_x | mid_x | mid_y | max_y |
| TR   | 3 | mid_x | max_x | mid_y | max_y |

**Randregel (kein freier Spielraum):** `x <= mid_x` → links, `y <= mid_y` → unten. Punkte genau auf der Mittellinie gehören also nach links bzw. unten. Nur so stimmt die Ausgabereihenfolge von Testcase 03.

Die Grenzen werden den Kindern als Text übergeben. Mit `%.9g` ausgeben, dann liefert `strtof` exakt denselben `float` zurück und die Grenzen driften in tieferen Ebenen nicht.

---

## 4. Pseudocode

Schreibweise: `←` ist Zuweisung, `=` ist Vergleich, `//` ist Kommentar.

### 4.1 Hilfsfunktionen

```
Funktion ParsePunkt(text) → (ok, x, y)
    entferne ein abschließendes '\n' von text
    suche das Komma in text
    wenn kein Komma vorhanden dann
        gib (falsch, 0, 0) zurück
    x ← strtof(Teil vor dem Komma)
    y ← strtof(Teil nach dem Komma)
    wenn x oder y nicht vollständig gelesen wurde (endptr ≠ Ende) dann
        gib (falsch, 0, 0) zurück
    gib (wahr, x, y) zurück
```

Dieselbe Funktion wird für das Ziel **und** für jede Eingabezeile benutzt. `"1,"` schlägt fehl (Testcase 09).

```
Funktion Distanz(x, y, goal_x, goal_y) → float
    gib sqrtf( powf(x − goal_x, 2) + powf(y − goal_y, 2) ) zurück
```

Die Distanz gilt immer zum **Ziel**, nie zur Mitte (Pivot).

```
Funktion Quadrant(x, y, mid_x, mid_y) → {0, 1, 2, 3}
    links ← (x ≤ mid_x)
    unten ← (y ≤ mid_y)
    wenn unten und links dann gib 0 zurück     // BL
    wenn unten dann           gib 1 zurück     // BR
    wenn links dann           gib 2 zurück     // TL
    gib 3 zurück                               // TR
```

```
Funktion GleicherPunkt(x1, y1, x2, y2) → Wahrheitswert
    gib (x1 = x2 und y1 = y2) zurück
```

Es wird mit den **Zahlenwerten** verglichen, nicht mit dem Text (`1.0,1` = `1,1`). Jede Zeile, die denselben Punkt wie die **erste** Zeile hat, wird verworfen (nicht weitergeschickt). Die erste Zeile selbst bleibt genau einmal erhalten. Duplikate anderer Punkte laufen weiter nach unten und werden dort verworfen, wenn sie in einem Kind die erste Zeile sind.

```
Prozedur PruefeUndGibAus(zeile, x, y, goal_x, goal_y, distance)
    d ← Distanz(x, y, goal_x, goal_y)
    wenn d ≤ distance dann
        schreibe  "%.3f: <zeile ohne '\n'>\n"  auf stdout    // Originaltext der Zeile!
```

### 4.2 Hauptprogramm (ein Prozess = eine Ebene)

```
Prozedur main(argc, argv)

    // ---------- Schritt 1: Argumente ----------
    SIGPIPE ignorieren                         // bleibt auch über fork und exec ignoriert
    wenn argc ≠ 7 dann
        schreibe Usage (mit argv[0]) auf stdout
        beende mit EXIT_FAILURE

    (ok, goal_x, goal_y) ← ParsePunkt(argv[1])
    distance, min_x, max_x, min_y, max_y ← strtof(argv[2..6])
    wenn irgendetwas nicht parst dann
        schreibe Fehlermeldung (mit argv[0]) auf stderr
        beende mit EXIT_FAILURE

    // ---------- Schritt 2: nur die ersten Zeilen lesen (Fallunterscheidung) ----------
    // Wir müssen nur wissen: 0 Zeilen, 1 verschiedene Zeile oder mehr?
    // Dafür reichen die erste Zeile und die erste Zeile, die anders ist als die erste.
    // Der Rest wird erst in Schritt 4 beim Lesen verteilt.

    erste ← nächste Zeile aus stdin (z. B. mit getline)
    wenn EOF dann
        beende mit EXIT_SUCCESS                // 0 Zeilen: leerer Quadrant, keine Ausgabe

    (ok, ex, ey) ← ParsePunkt(erste)
    wenn nicht ok dann
        schreibe Fehlermeldung auf stderr
        beende mit EXIT_FAILURE                // noch KEINE Kinder gestartet, nichts aufzuräumen

    zweite ← nichts
    solange eine weitere Zeile z gelesen werden kann (bis EOF) tue
        (ok, x, y) ← ParsePunkt(z)
        wenn nicht ok dann
            schreibe Fehlermeldung auf stderr
            beende mit EXIT_FAILURE            // auch hier noch KEINE Kinder gestartet
        wenn nicht GleicherPunkt(x, y, ex, ey) dann
            zweite ← (z, x, y)
            verlasse die Schleife              // jetzt ist klar: es gibt mehr als 1 verschiedene Zeile

    // ---------- Schritt 3: Abbruchfall (hier endet die Rekursion) ----------
    wenn zweite = nichts dann                  // nur 1 verschiedene Zeile (evtl. mehrfach vorhanden)
        PruefeUndGibAus(erste, ex, ey, goal_x, goal_y, distance)
        beende mit EXIT_SUCCESS

    // ---------- Schritt 4: rekursiver Fall ----------
    mid_x ← (min_x + max_x) / 2
    mid_y ← (min_y + max_y) / 2

    grenzen[0] ← (min_x, mid_x, min_y, mid_y)     // BL
    grenzen[1] ← (mid_x, max_x, min_y, mid_y)     // BR
    grenzen[2] ← (min_x, mid_x, mid_y, max_y)     // TL
    grenzen[3] ← (mid_x, max_x, mid_y, max_y)     // TR

    // Phase A: alle 4 Kinder STARTEN (noch nichts schreiben/lesen/warten!)
    für i von 0 bis 3 tue
        pipeA[i] ← pipe()                          // Eltern → Kind
        pipeB[i] ← pipe()                          // Kind → Eltern
        pid[i]   ← fork()

        wenn pid[i] < 0 dann
            Aufräumen(); beende mit EXIT_FAILURE

        wenn pid[i] = 0 dann                       // ----- wir sind das KIND -----
            // zuerst ALLE geerbten Eltern-Enden schließen (eigene und die der früheren Kinder)
            für j von 0 bis i tue
                schließe pipeA[j].schreiben
                schließe pipeB[j].lesen
            dup2(pipeA[i].lesen,     STDIN)
            dup2(pipeB[i].schreiben, STDOUT)
            schließe pipeA[i].lesen
            schließe pipeB[i].schreiben
            execlp(argv[0], argv[0], argv[1], argv[2], grenzen[i] als Text ...)
            // nur wenn exec fehlschlägt:
            schreibe Fehlermeldung auf stderr
            _exit(EXIT_FAILURE)

        // ----- wir sind noch der ELTERNPROZESS -----
        schließe pipeA[i].lesen                    // das gehört dem Kind
        schließe pipeB[i].schreiben                // das gehört dem Kind

    // Phase B: Zeilen SOFORT beim Lesen an die Kinder verteilen (Folie 31)
    SendeZeile(erste, ex, ey)
    SendeZeile(zweite.text, zweite.x, zweite.y)
    solange eine weitere Zeile z aus stdin gelesen werden kann (bis EOF) tue
        (ok, x, y) ← ParsePunkt(z)
        wenn nicht ok dann
            schreibe Fehlermeldung auf stderr
            Aufräumen(); beende mit EXIT_FAILURE   // Kinder laufen schon, siehe 4.3
        wenn nicht GleicherPunkt(x, y, ex, ey) dann
            SendeZeile(z, x, y)                    // Duplikate der ersten Zeile werden verworfen
    für i von 0 bis 3 tue
        schließe pipeA[i].schreiben                // Kinder sehen jetzt EOF und können ausgeben

    // Phase C: Ergebnisse in fester Reihenfolge einsammeln
    für i von 0 bis 3 tue                          // BL → BR → TL → TR
        solange read(pipeB[i].lesen) Daten liefert tue
            write(STDOUT, Daten)                   // unverändert durchreichen
        schließe pipeB[i].lesen

    // Phase D: auf alle Kinder warten
    fehler ← falsch
    für i von 0 bis 3 tue
        waitpid(pid[i], status)
        wenn Kind nicht normal beendet oder status ≠ EXIT_SUCCESS dann
            fehler ← wahr

    gib den Zeilenpuffer frei
    wenn fehler dann beende mit EXIT_FAILURE
    beende mit EXIT_SUCCESS
```

```
Prozedur SendeZeile(text, x, y)                    // benutzt mid_x, mid_y, pipeA aus main
    i ← Quadrant(x, y, mid_x, mid_y)
    schreibe text (ohne '\n') + "\n" in pipeA[i].schreiben    // in einer Schleife, bis alles geschrieben ist
    wenn write fehlschlägt (z. B. EPIPE) dann
        Aufräumen(); beende mit EXIT_FAILURE
```

(In C ohne globale Variablen: `mid_x`, `mid_y` und die Pipes als Parameter oder in einer kleinen Struktur übergeben.)

### 4.3 Aufräumen bei Fehlern (nach dem Start von Kindern)

Ein einziger Aufräum-Pfad für alle Fehler ab Phase A (`pipe`/`fork`/`write` schlägt fehl **oder** eine Eingabezeile ist fehlerhaft):

```
Prozedur Aufräumen()
    schließe alle noch offenen Pipe-Enden        // Kinder sehen EOF und beenden sich selbst, KEIN kill!
    für jedes bereits gestartete Kind tue
        waitpid(pid)                             // keine Zombies
    gib den Zeilenpuffer frei
```

Phase C wird in diesem Fall **nicht** erreicht. Die Ausgabe der Kinder liegt nur in den Pipes und wird verworfen, auf `stdout` kommt also nichts. Die Kinder bekommen beim Schreiben `EPIPE` (weil `SIGPIPE` ignoriert wird, auch in ihnen), melden das als Fehler und beenden sich.

---

## 5. Ablauf an einem Beispiel

`./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` mit den Punkten `1,1`, `2,-2`, `-3,3`, `-4,-4`:

```
Prozess 1 (Grenzen -10..10) liest 1,1 (erste) und 2,-2 (verschieden von der ersten) → mid = 0,0
 startet 4 Kinder, schickt 1,1 → TR und 2,-2 → BR, liest weiter und verteilt -3,3 → TL, -4,-4 → BL
 ├─ BL-Kind bekommt -4,-4 → 1 Zeile: Distanz 5.66 > 4.5 → gibt nichts aus
 ├─ BR-Kind bekommt  2,-2 → 1 Zeile: gibt "2.828: 2,-2" aus
 ├─ TL-Kind bekommt -3,3  → 1 Zeile: gibt "4.243: -3,3" aus
 └─ TR-Kind bekommt  1,1  → 1 Zeile: gibt "1.414: 1,1" aus
Prozess 1 liest BL, BR, TL, TR in dieser Reihenfolge und gibt alles aus
```

Die Reihenfolge `2.828`, `4.243`, `1.414` in der Angabe kommt also **nicht** von der Distanz, sondern von der Quadranten-Reihenfolge. Bekommt ein Kind mehr als 1 verschiedene Zeile, macht es dasselbe noch einmal mit seinem kleineren Rechteck und startet 4 eigene Kinder.

---

## 6. Pipes im Überblick

Pro Kind gibt es 2 Pipes, insgesamt also 8 Pipes und 4 `fork`s:

```
Eltern --schreibt--> [Pipe A] --liest--> stdin  des Kindes
Eltern <--liest----- [Pipe B] <--schreibt-- stdout des Kindes
```

Alles liegt in Arrays mit einem Platz pro Quadrant (Index 0..3 = BL, BR, TL, TR). Mit demselben Index überall werden die Pipes nie vertauscht.

Die Eltern halten die Eingabe nicht im Speicher. Jede Zeile wird gelesen, dem Quadranten zugeordnet, in die passende Pipe A geschrieben und danach nicht mehr gebraucht. Nur die erste Zeile (zum Vergleich) bleibt erhalten.

---

## 7. Typische Fehler (hier bleibt man meistens hängen)

1. **Pipe-Enden nicht geschlossen.** Ein Kind sieht erst EOF, wenn *alle* Schreib-Enden seiner Eingabe-Pipe zu sind. Kind 2 erbt aber die offenen Eltern-Enden von Kind 1. Darum schließt jedes Kind **zuerst** alle Eltern-Enden, **dann** `dup2`, dann die Originale schließen, dann `execlp`. Die Eltern schließen die Kind-Enden direkt nach jedem `fork`.
2. **Kinder nacheinander statt gleichzeitig.** Wer pro Kind forkt, füttert, liest und wartet, lässt die Kinder hintereinander laufen. Die Angabe verlangt aber gleichzeitiges Laufen. Richtig ist: alle 4 forken → alle 4 füttern → alle 4 lesen → alle 4 `waitpid`.
3. **Kein Deadlock beim Füttern.** Jeder Prozess liest seine ganze Eingabe bis EOF, bevor er etwas ausgibt, und die Eltern lesen die Ergebnisse erst in Phase C, nachdem alle Eingabe-Pipes geschlossen sind. Ein Kind blockiert also nie beim Schreiben, während die Eltern noch füttern. Das Füttern wartet höchstens kurz, bis das Kind die nächste Zeile liest.
4. **Falsche Lesereihenfolge.** Immer BL, BR, TL, TR.
5. **`<` statt `<=`.** Dann stimmt Testcase 03 nicht.
6. **Endlose Rekursion bei Duplikaten.** Ohne Vergleich mit der ersten Zeile hört `1,1` + `1,1` nie auf.
7. **SIGPIPE.** Stirbt ein Kind früh, tötet ein `write` in dessen Pipe sonst die Eltern. Darum am Anfang `signal(SIGPIPE, SIG_IGN)` und `EPIPE` als Fehler behandeln. „Ignorieren“ bleibt über `fork` und `exec` erhalten, gilt also in allen Prozessen des Baums.
8. **stdio und `read`/`write` mischen.** Pro Dateideskriptor eines von beiden benutzen (Guideline 29). `getline` auf `stdin` plus `write` auf die Pipe-Fds ist in Ordnung, weil das verschiedene Fds sind.
9. **Debug-Ausgaben.** Nur unter `#ifdef DEBUG`, sonst scheitert der automatische Test.
10. **Erst alles einlesen, dann forken.** Das ist auf Folie 31 ausdrücklich als „Do NOT“ markiert. Die Eltern würden das ganze Einlesen abwarten, bevor ein Kind etwas tut. Richtig: nur die ersten Zeilen vorab lesen (bis zur ersten verschiedenen) und den Rest beim Lesen verteilen.
11. **Fehler mitten im Stream.** Ist erst eine spätere Zeile kaputt, laufen die Kinder schon. Dann muss `Aufräumen()` alle Pipe-Enden schließen und alle Kinder einsammeln, und die Ausgabe der Kinder darf **nicht** auf `stdout` landen (Phase C wird gar nicht erreicht).
12. **Teilweises `write`.** `write` kann weniger Bytes schreiben als verlangt. In einer Schleife wiederholen, bis die ganze Zeile draußen ist.

---

## 8. Entscheidungen, die nicht in der Angabe stehen

| Frage | Antwort |
|---|---|
| Reihenfolge der Grenzen | `min_x max_x min_y max_y` (laut SYNOPSIS) |
| 0 Zeilen | keine Ausgabe, `EXIT_SUCCESS` (natürliche Lesart, leere Quadranten sind normal) |
| Eingabe lesen | zeilenweise (`getline`). Vor dem ersten `fork` nur die erste Zeile und die erste davon verschiedene Zeile, der Rest wird beim Lesen verteilt (Folie 31) |
| Erste Zeile bei Duplikaten | bleibt genau einmal erhalten, nur spätere gleiche Zeilen (Zahlenvergleich) werden verworfen |
| Alle Zeilen gleich | die eine übrige Zeile wird wie der 1-Zeilen-Fall geprüft und ausgegeben („additional-1“) |
| Punkt auf der Mittellinie | `<=` → links bzw. unten (Testcase 03) |
| falsche `argc` | Usage auf stdout, `EXIT_FAILURE` (Testcase 08) |
| fehlerhafte Zeile wie `1,` | Meldung auf stderr, `EXIT_FAILURE`, nichts auf stdout (Testcase 09). Liegt sie vor dem ersten `fork`, gibt es nichts aufzuräumen, sonst läuft `Aufräumen()` |

Die Angabe sagt bei „alle Zeilen gleich“ nur „exit with EXIT_SUCCESS“, ohne etwas auszugeben. „additional-1“ erwartet aber `1.414: 1,1`, also gilt die Lesart oben.

Offen (optional beim Tutor nachfragen): „additional-2“ erwartet `2.828: 2,2` bei Distanz 1.5 vom Ursprung, das kann nicht stimmen (`2,2` ist 2.828 entfernt). Die Befehle dort sind auch kaputt (`echo -n` mit `\n`, `>` statt `|`). Der Test wird nicht automatisch geprüft.

---

## 9. In dieser Reihenfolge bauen

1. Argumente (+ Usage) + `ParsePunkt` + erste Zeile(n) von `stdin` lesen. Der 1-Zeilen-Fall funktioniert damit schon (Testcases 01, 08, 09).
2. Vergleich mit der ersten Zeile und Quadranten **ohne Prozesse** bestimmen (nur ausgeben, in welchen Quadranten jede Zeile geht, und die `<=`-Regel prüfen).
3. **Ein** Kind mit beiden Pipes: eine Zeile hin und wieder zurück.
4. **Vier** Kinder gleichzeitig, Zeilen direkt beim Lesen verteilen, Ausgabe in der Reihenfolge BL, BR, TL, TR.
5. Exit-Status, Aufräum-Pfad (auch für Fehler mitten im Stream), SIGPIPE.
6. Makefile, Doxygen-Kommentare, `-DDEBUG` nur für Debug-Ausgaben.

---

## 10. Checkliste Stil und Build

- Compilieren ohne Warnungen: `gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c quadtree.c`, linken mit `-lm`.
- Makefile: `all` als erstes Target, `clean` idempotent, Header mit Autor und Programmname.
- Doxygen-Header pro Datei (`@author`, `@brief`, `@details`, `@date`) und jede Funktion (auch `static`) dokumentiert.
- Konstanten als Makros (Anzahl Argumente = 7, Anzahl Kinder = 4, Indizes BL/BR/TL/TR), keine globalen Variablen.
- Kein `scanf`, `atoi`, `gets`.
- Rückgabewerte von `fork`, `pipe`, `dup2`, `execlp`, `write`, `getline`/`malloc` prüfen. Den `getline`-Puffer am Ende (und in `Aufräumen()`) freigeben.
- Fehler- und Debugausgaben nur auf `stderr` (die Usage-Meldung ist die Ausnahme, die geht auf `stdout`).

---

## 11. Testen

- Alle Testcases 01–09 aus der Angabe, auch die Exit-Codes (`echo $?`). Testcase 03 zuerst, weil er Randregel und Reihenfolge prüft.
- `./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` → `2.828: 2,-2`, `4.243: -3,3`, `1.414: 1,1`.
- Dasselbe mit `| sort` und mit `diff` vergleichen.
- Eigene Fälle: leere Eingabe, 1 Zeile, 2 gleiche Zeilen (`1,1` zweimal → `1.414: 1,1` bei Ziel 0,0 und Distanz 1.5), Punkte auf den Mittellinien, falsche Argumente (kein Komma, Buchstaben, falsche `argc`).
- Kaputte Zeile an verschiedenen Stellen: als erste Zeile (vor jedem `fork`), als dritte oder spätere Zeile (die Kinder laufen schon). Beide Male: Meldung auf stderr, `EXIT_FAILURE`, **nichts** auf stdout, danach kein hängender Prozess.
- Die „additional“-Tests mit 2500 und 10000 Punkten erst, wenn alles andere läuft.
- `strace -f` zeigt Forks und Pipes, `valgrind --trace-children=yes --track-fds=yes` zeigt Leaks und offene Fds.
- Nach jedem Lauf: `pgrep quadtree` darf nichts anzeigen (keine Zombies oder hängenden Kinder).