# quadtree – Algorithmus und Pseudocode (Übung 2, 191.002 Betriebssysteme)

## 1. Worum geht es?

Das Programm bekommt viele 2D-Punkte über `stdin` (eine Zeile pro Punkt, z. B. `1,1`), einen Zielpunkt `goal` und eine Maximaldistanz `distance`. Es gibt alle Punkte aus, die höchstens `distance` vom Ziel entfernt sind, im Format `1.414: 1,1`.

Eine einfache Schleife würde dafür reichen. Das Ziel der Übung ist aber, **fork, exec und Pipes** zu üben. Darum muss die Suche als **Baum aus Prozessen** laufen: Jeder Prozess ist ein Knoten und startet sich selbst vier Mal neu (einmal pro Quadrant).

**Wichtig:** Es gibt keine Quadtree-Datenstruktur (keine Knoten, keine Zeiger). Der „Baum“ existiert nur als Prozessbaum, solange das Programm läuft.

| normale Rekursion | in dieser Aufgabe |
|---|---|
| Funktion ruft sich selbst auf | Programm startet sich selbst (`fork` + `execlp(argv[0], ...)`) |
| Parameter (Array, Grenzen) | `stdin` (die Punkte) + Kommandozeilenargumente (Ziel, Distanz, Grenzen) |
| Rückgabewert | `stdout` (Ergebniszeilen) + Exit-Status |
| Aufrufstack | Prozessbaum |
| Abbruchbedingung `if (n <= 1) return` | 0 oder 1 Zeile: ausgeben und beenden, kein `fork` |

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
Funktion EntferneDuplikate(zeilen) → zeilen'
    erste ← zeilen[0]
    ergebnis ← [erste]                          // die erste Zeile bleibt genau einmal
    für i von 1 bis Länge(zeilen) − 1 tue
        wenn zeilen[i].x ≠ erste.x oder zeilen[i].y ≠ erste.y dann
            hänge zeilen[i] an ergebnis an
    gib ergebnis zurück
```

Es wird mit den **Zahlenwerten** verglichen, nicht mit dem Text (`1.0,1` = `1,1`). Nur Duplikate der **ersten** Zeile fliegen raus. Duplikate anderer Punkte laufen weiter nach unten und werden dort entfernt, wenn sie in einem Kind die erste Zeile sind.

```
Prozedur PruefeUndGibAus(zeile, goal_x, goal_y, distance)
    d ← Distanz(zeile.x, zeile.y, goal_x, goal_y)
    wenn d ≤ distance dann
        schreibe  "%.3f: <zeile.text>\n"  auf stdout    // Originaltext der Zeile!
```

### 4.2 Hauptprogramm (ein Prozess = eine Ebene)

```
Prozedur main(argc, argv)

    // ---------- Schritt 1: Argumente ----------
    SIGPIPE ignorieren
    wenn argc ≠ 7 dann
        schreibe Usage (mit argv[0]) auf stdout
        beende mit EXIT_FAILURE

    (ok, goal_x, goal_y) ← ParsePunkt(argv[1])
    distance, min_x, max_x, min_y, max_y ← strtof(argv[2..6])
    wenn irgendetwas nicht parst dann
        schreibe Fehlermeldung (mit argv[0]) auf stderr
        beende mit EXIT_FAILURE

    // ---------- Schritt 2: Eingabe lesen und prüfen ----------
    zeilen ← leere dynamische Liste
    solange eine Zeile aus stdin gelesen werden kann (bis EOF) tue
        (ok, x, y) ← ParsePunkt(zeile)
        wenn nicht ok dann
            schreibe Fehlermeldung auf stderr
            beende mit EXIT_FAILURE          // noch KEINE Kinder gestartet, nichts aufzuräumen
        hänge (text = zeile, x, y) an zeilen an

    // ---------- Schritt 3: Abbruchfälle (hier endet die Rekursion) ----------
    wenn Länge(zeilen) = 0 dann
        beende mit EXIT_SUCCESS              // leerer Quadrant, keine Ausgabe

    zeilen ← EntferneDuplikate(zeilen)

    wenn Länge(zeilen) = 1 dann
        PruefeUndGibAus(zeilen[0], goal_x, goal_y, distance)
        beende mit EXIT_SUCCESS

    // ---------- Schritt 4: rekursiver Fall (mehr als 1 verschiedene Zeile) ----------
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

    // Phase B: jedem Kind seine Punkte schicken
    für i von 0 bis 3 tue
        für jede zeile in zeilen tue
            wenn Quadrant(zeile.x, zeile.y, mid_x, mid_y) = i dann
                write(pipeA[i].schreiben, zeile.text + "\n")
        schließe pipeA[i].schreiben                // Kind sieht jetzt EOF und kann loslegen

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

    gib zeilen frei
    wenn fehler dann beende mit EXIT_FAILURE
    beende mit EXIT_SUCCESS
```

### 4.3 Aufräumen bei Fehlern (nach dem Start von Kindern)

Ein einziger Aufräum-Pfad für alle Fehler ab Phase A (`pipe`/`fork`/`write` schlägt fehl):

```
Prozedur Aufräumen()
    schließe alle noch offenen Pipe-Enden        // Kinder sehen EOF und beenden sich selbst, KEIN kill!
    für jedes bereits gestartete Kind tue
        waitpid(pid)                             // keine Zombies
    gib zeilen frei
```

---

## 5. Ablauf an einem Beispiel

`./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` mit den Punkten `1,1`, `2,-2`, `-3,3`, `-4,-4`:

```
Prozess 1 (Grenzen -10..10, 4 Zeilen) → mid = 0,0
 ├─ BL-Kind bekommt -4,-4 → 1 Zeile: Distanz 5.66 > 4.5 → gibt nichts aus
 ├─ BR-Kind bekommt  2,-2 → 1 Zeile: gibt "2.828: 2,-2" aus
 ├─ TL-Kind bekommt -3,3  → 1 Zeile: gibt "4.243: -3,3" aus
 └─ TR-Kind bekommt  1,1  → 1 Zeile: gibt "1.414: 1,1" aus
Prozess 1 liest BL, BR, TL, TR in dieser Reihenfolge und gibt alles aus
```

Die Reihenfolge `2.828`, `4.243`, `1.414` in der Angabe kommt also **nicht** von der Distanz, sondern von der Quadranten-Reihenfolge. Bekommt ein Kind mehr als 1 Zeile, macht es dasselbe noch einmal mit seinem kleineren Rechteck und startet 4 eigene Kinder.

---

## 6. Pipes im Überblick

Pro Kind gibt es 2 Pipes, insgesamt also 8 Pipes und 4 `fork`s:

```
Eltern --schreibt--> [Pipe A] --liest--> stdin  des Kindes
Eltern <--liest----- [Pipe B] <--schreibt-- stdout des Kindes
```

Alles liegt in Arrays mit einem Platz pro Quadrant (Index 0..3 = BL, BR, TL, TR). Mit demselben Index überall werden die Pipes nie vertauscht.

---

## 7. Typische Fehler (hier bleibt man meistens hängen)

1. **Pipe-Enden nicht geschlossen.** Ein Kind sieht erst EOF, wenn *alle* Schreib-Enden seiner Eingabe-Pipe zu sind. Kind 2 erbt aber die offenen Eltern-Enden von Kind 1. Darum schließt jedes Kind **zuerst** alle Eltern-Enden, **dann** `dup2`, dann die Originale schließen, dann `execlp`. Die Eltern schließen die Kind-Enden direkt nach jedem `fork`.
2. **Kinder nacheinander statt gleichzeitig.** Wer pro Kind forkt, füttert, liest und wartet, lässt die Kinder hintereinander laufen. Die Angabe verlangt aber gleichzeitiges Laufen. Richtig ist: alle 4 forken → alle 4 füttern → alle 4 lesen → alle 4 `waitpid`.
3. **Kein Deadlock beim Füttern.** Jeder Prozess liest seine ganze Eingabe bis EOF, bevor er etwas ausgibt. Ein Kind blockiert also nie beim Schreiben, während die Eltern noch füttern.
4. **Falsche Lesereihenfolge.** Immer BL, BR, TL, TR.
5. **`<` statt `<=`.** Dann stimmt Testcase 03 nicht.
6. **Endlose Rekursion bei Duplikaten.** Ohne Entfernen der Duplikate der ersten Zeile hört `1,1` + `1,1` nie auf.
7. **SIGPIPE.** Stirbt ein Kind früh, tötet ein `write` in dessen Pipe sonst die Eltern. Darum am Anfang `signal(SIGPIPE, SIG_IGN)` und `EPIPE` als Fehler behandeln.
8. **stdio und `read`/`write` mischen.** Pro Dateideskriptor eines von beiden benutzen (Guideline 29). `getline` auf `stdin` plus `write` auf die Pipe-Fds ist in Ordnung, weil das verschiedene Fds sind.
9. **Debug-Ausgaben.** Nur unter `#ifdef DEBUG`, sonst scheitert der automatische Test.

---

## 8. Entscheidungen, die nicht in der Angabe stehen

| Frage | Antwort |
|---|---|
| Reihenfolge der Grenzen | `min_x max_x min_y max_y` (laut SYNOPSIS) |
| 0 Zeilen | keine Ausgabe, `EXIT_SUCCESS` (natürliche Lesart, leere Quadranten sind normal) |
| Erste Zeile bei Duplikaten | bleibt genau einmal erhalten, nur spätere gleiche Zeilen fallen weg |
| Alle Zeilen gleich | die eine übrige Zeile wird wie der 1-Zeilen-Fall geprüft und ausgegeben („additional-1“) |
| Punkt auf der Mittellinie | `<=` → links bzw. unten (Testcase 03) |
| falsche `argc` | Usage auf stdout, `EXIT_FAILURE` (Testcase 08) |
| fehlerhafte Zeile wie `1,` | Fehler auf stderr, `EXIT_FAILURE`, **vor** jedem `fork` (Testcase 09) |

Die Angabe sagt bei „alle Zeilen gleich“ nur „exit with EXIT_SUCCESS“, ohne etwas auszugeben. „additional-1“ erwartet aber `1.414: 1,1`, also gilt die Lesart oben.

Offen (optional beim Tutor nachfragen): „additional-2“ erwartet `2.828: 2,2` bei Distanz 1.5 vom Ursprung, das kann nicht stimmen (`2,2` ist 2.828 entfernt). Die Befehle dort sind auch kaputt (`echo -n` mit `\n`, `>` statt `|`). Der Test wird nicht automatisch geprüft.

---

## 9. In dieser Reihenfolge bauen

1. Argumente (+ Usage) + `ParsePunkt` + `stdin` lesen. Der 1-Zeilen-Fall funktioniert damit schon (Testcases 01, 08, 09).
2. Duplikate entfernen und Quadranten **ohne Prozesse** bestimmen (nur ausgeben, in welchen Quadranten jede Zeile geht, und die `<=`-Regel prüfen).
3. **Ein** Kind mit beiden Pipes: eine Zeile hin und wieder zurück.
4. **Vier** Kinder gleichzeitig + Ausgabe in der Reihenfolge BL, BR, TL, TR.
5. Exit-Status, Aufräum-Pfad, SIGPIPE.
6. Makefile, Doxygen-Kommentare, `-DDEBUG` nur für Debug-Ausgaben.

---

## 10. Checkliste Stil und Build

- Compilieren ohne Warnungen: `gcc -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c quadtree.c`, linken mit `-lm`.
- Makefile: `all` als erstes Target, `clean` idempotent, Header mit Autor und Programmname.
- Doxygen-Header pro Datei (`@author`, `@brief`, `@details`, `@date`) und jede Funktion (auch `static`) dokumentiert.
- Konstanten als Makros (Start-Kapazität des Arrays, Anzahl Argumente = 7, Anzahl Kinder = 4, Indizes BL/BR/TL/TR), keine globalen Variablen.
- Kein `scanf`, `atoi`, `gets`.
- Rückgabewerte von `fork`, `pipe`, `dup2`, `execlp`, `write`, `malloc`/`realloc` prüfen.
- Fehler- und Debugausgaben nur auf `stderr`.

---

## 11. Testen

- Alle Testcases 01–09 aus der Angabe, auch die Exit-Codes (`echo $?`). Testcase 03 zuerst, weil er Randregel und Reihenfolge prüft.
- `./quadtree 0,0 4.5 -10 10 -10 10 < 1.txt` → `2.828: 2,-2`, `4.243: -3,3`, `1.414: 1,1`.
- Dasselbe mit `| sort` und mit `diff` vergleichen.
- Eigene Fälle: leere Eingabe, 1 Zeile, 2 gleiche Zeilen (`1,1` zweimal → `1.414: 1,1` bei Ziel 0,0 und Distanz 1.5), Punkte auf den Mittellinien, falsche Argumente (kein Komma, Buchstaben, falsche `argc`), kaputte Zeile mitten in der Eingabe (muss vor jedem `fork` scheitern).
- Die „additional“-Tests mit 2500 und 10000 Punkten erst, wenn alles andere läuft.
- `strace -f` zeigt Forks und Pipes, `valgrind --trace-children=yes --track-fds=yes` zeigt Leaks und offene Fds.
- Nach jedem Lauf: `pgrep quadtree` darf nichts anzeigen (keine Zombies oder hängenden Kinder).