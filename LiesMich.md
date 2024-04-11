*„Wenn die Realität nur eine Projektion ist, was passiert dann, wenn die Quelle, das Bewusstsein, abwesend ist?“*
<br>
<br>
:gb:[Switch to the English version](https://github.com/1ux/DCF77Decode)

# Basic DCF77 decode
Diese Arduino-Bibliothek implementiert alle von der „[Physikalisch-Technischen Bundesanstalt](https://www.ptb.de/cms/ptb/fachabteilungen/abt4/fb-44/ag-442/verbreitung-der-gesetzlichen-zeit/dcf77/zeitcode.html)“ öffentlich spezifizierten Bits, die innerhalb eines 60-Sekunden-Bitstroms des Zeitsignals liegen, mit Ausnahme der Schaltsekunde.
Es prüft auch auf Übertragungsfehler durch gerade Parität und fragt das 15-Bit ("call bit") ab, um Probleme mit dem Sender auszuschließen.
Der Schwerpunkt liegt auf sauberem Code, hervorragender Dokumentation und einfacher Benutzung dieser Bibliothek.
Die blockierende pulseIn()-Funktion, die Teil der Arduino-Standard-Bibliothek ist, bildet die Grundlage für diese Implementierung. Es sind keine weiteren Abhängigkeiten erforderlich.

<div align="center">
<table><tr>
<td> <img width="320" height="240" src="figures/short_and_long_DCF77_pulse.jpg"> </td>
<td> <img width="320" height="240" src="figures/minute_marker.jpg"> </td>
</tr></table>
</div>

Der 200-ms-Impuls steht für eine 1 und der 100-ms-Impuls für eine 0. Sie können die Impulse in der Abbildung links sehen.
Auf diese Weise können in einer Minute 59 Bits empfangen werden, die immer das aktuelle Datum und die Uhrzeit enthalten. Der 60. Impuls wird niedrig gehalten (außer bei Schaltsekunden). Dieser ist in der Abbildung rechts dargestellt.
Er zeigt das Ende der Minute an. Wenn Sie alle 59 Impulse als Bitfolge interpretieren, können Sie die Bedeutung der einzelnen Bits anhand der folgenden Abbildung dekodieren.

<p align="center">
    <img width="430" height="414" src="figures/dcf77kode2007.jpg">
</p>

Bildquelle: [Physikalisch-Technische Bundesanstalt](https://www.ptb.de/cms/ptb/fachabteilungen/abt4/fb-44/ag-442/verbreitung-der-gesetzlichen-zeit/dcf77/zeitcode.html)

Das folgende Beispiel zeigt eine vollständige DCF77-Bitfolge:
```
00111010000010100010110010011100110110010110111000001001001
                     ^^^^^^^
                     |

                     Minutenbits (21 bis 27)
```

Die für die Minutencodierung relevanten 7 Bits sind markiert. Beginnt man bei 0 und zählt von links nach rechts, so beginnt der markierte Bereich bei der 21sten Stelle. Wenn Sie nun die Bedeutung der Bits auf die obige Grafik beziehen, ergibt sich folgende Berechnung:

`(1*1)+(0*2)+(0*4)+(1*8)+(0*10)+(0*20)+(1*40)=49`

Wendet man dieses Prinzip auf die Dekodierung der Stunde an, so erhält man die Uhrzeit: 19:49 Uhr.

## Einrichtung der Hardware

Voraussetzung für die Nutzung dieser Bibliothek ist ein DCF77-Empfangsmodul zur Demodulation des AM-Signals aus Frankfurt.

Bitte beachten Sie, dass es neben [DCF77](https://de.wikipedia.org/wiki/DCF77) noch weitere Zeitzeichensender in anderen Ländern gibt, siehe z.B.: [WWVB in USA](https://en.wikipedia.org/wiki/WWV_(radio_station)), [MSF in Großbritannien](https://en.wikipedia.org/wiki/Time_from_NPL_(MSF)), [JJY in Japan](https://en.wikipedia.org/wiki/JJY), [BPM in China](https://en.wikipedia.org/wiki/BPM_(time_service)), [RWM in Russland](https://en.wikipedia.org/wiki/RWM).<br>
Wenn Sie einen guten Empfang oder einen hochwertigen Empfänger haben, können Sie ihn direkt an Ihren Mikrocontroller anschließen. Ich musste dem DCF-3850N-800 ein wenig beim Filtern helfen.

Die folgenden Komponenten sind in meinem Beispiel zu sehen:

- 1x [Arduino Uno](https://docs.arduino.cc/resources/datasheets/A000066-datasheet.pdf) oder viele andere
- 1x 1,5uF Kondensator
- 1x 10K-Widerstand
- 1x [CD40106B](https://www.ti.com/lit/ds/symlink/cd40106b.pdf?ts=1712053920606&ref_url=https%253A%252F%252Fwww.google.com%252F) Schmitt-Trigger Inverter IC (im Schaltungsbeispiel siehe U1A und U1B)
- 1x [DCF-3850N-800](https://www.google.de/search?q=DCF-3850N-800) DCF77-Empfänger oder viele andere

Um ein wenig zu filtern, verwende ich einen Tiefpassfilter mit einer [Grenzfrequenz](https://de.wikipedia.org/wiki/Grenzfrequenz) von etwa 10 Hz:


$$
C = \frac{1}{2 \pi f R} \\
= \frac{1}{2 \pi \cdot 10 \text{Hz} \cdot 10.000 \Omega} \\
= \frac{1}{628318,53} \\
\approx 1,59 \mu\text{F}
$$

Aus diesen Überlegungen ergibt sich der Wert für den Widerstand R1 und den Kondensator C1. Der Schmitt-Trigger wird verwendet, um die Flanken für den digitalen Eingang des Mikrocontrollers zu glätten.


<p align="center">
    <img width="430" height="437" src="figures/circuit_example.jpg">
</p>

## Überblick über die Bibliothek

```C
#ifndef basic_dcf77_h
#define basic_dcf77_h

#include <Arduino.h>

#define BIT_0_DURATION 130000 //Maximale Signallaufzeit für Interpretation einer 0
#define BIT_1_DURATION 240000 //Maximale Signallaufzeit für Interpretation einer 1
#define min_BIT_0_DURATION 20000 //Alles Kleinere wird als Störimpuls gewertet
#define DCF77_STRING_SIZE 59  //Größe der DCF77-Zeichenkette
#define TIMEOUT_DURATION 1600000 //Repräsentiert das Ende eines DCF77-Strings

struct TimeStampDCF77
{
    //DCF77-Datumswerte sind immer zweistellig
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t weekday;
    uint8_t month;
    uint8_t year;
    uint8_t A1; ///Bevorstehender Wechsel von MEZ zu MESZ oder umgekehrt.
    uint8_t CEST; //prüfe, ob es Sommerzeit ist
    uint8_t CET; //prüfe, ob es Winterzeit ist
    uint8_t transmitter_fault;//Nur zu beachten bei sehr gutem Empfang
};

//Pinzuweisung zwischen MCU und DCF77-Modul
void setupDCF77(uint8_t pin);
//Startet den Empfang eines demodulierten DCF77-Strings
int receiveDCF77(uint8_t* bitArray, uint8_t size);
//Interpretiert den DCF77-String und schreibt die dekodierten Werte in eine Struktur.
//Auswertung der Parität erfolgt über den Rückgabewert: SUCCESS/ERROR_INVALID_VALUE.
int decodeDCF77(uint8_t *bitArray, uint8_t size, TimeStampDCF77 *time);

```

## Anwendung

Hier ist eine minimale Beispielanwendung:

```C
#include <basic_dcf77.h>
#include <DebugProject.h>

uint8_t bitArray[DCF77_STRING_SIZE]; //Speicherplatz für den rohen DCF77-Bitstring
TimeStampDCF77 time;  //Variable zum Zwischenspeichern des dekodierten Signals

void setup()
{
  Serial.begin(115200);
  delay(7000);    //Der Start des DCF77-Moduls kann etwas dauern.
  setupDCF77(12); //Setzt den input Pin auf 12. Hier ist das DCF Modul angeschlossen
}

void loop()
{
  receiveDCF77(bitArray,DCF77_STRING_SIZE); //Startet den rohen DCF77-Empfang
  decodeDCF77(bitArray,DCF77_STRING_SIZE,&time); //Dekodierung der Binärwerte
  Serial.print(time.hour);
  Serial.print(":");
  Serial.println(time.minute);
}

```


Und hier ist ein Beispiel für die Verwendung der Bibliothek mit Fehlerbehandlung:

```C
#include <basic_dcf77.h>
#include <DebugProject.h>


uint8_t bitArray[DCF77_STRING_SIZE];
TimeStampDCF77 time;
int ReceiveDCF77;

char buffer[40];

void setup()
{
  Serial.begin(115200);
  delay(7000);
  setupDCF77(12);
}

void loop()
{
  ReceiveDCF77=receiveDCF77(bitArray,DCF77_STRING_SIZE); //Startet den DCF77-Empfang

  if(ReceiveDCF77==SUCCESS)
  {
    if(decodeDCF77(bitArray,DCF77_STRING_SIZE,&time)==SUCCESS)
    {
      snprintf(buffer,sizeof(buffer),"It is now %02d:%02d o'clock",time.hour,time.minute);
      Serial.println(buffer);
      snprintf(buffer,sizeof(buffer),"%02d.%02d.20%02d",time.day,time.month,time.year);
      Serial.println(buffer);
      snprintf(buffer, sizeof(buffer), "Weekday: %02d\n", time.weekday);
      Serial.println(buffer);
      if(time.transmitter_fault!=SUCCESS)
        Serial.println("Either their signal is noisy, or something is wrong in Germany.");
      else if(time.A1)
        Serial.println("Time change is coming up. (CET/CEST)");
    }
    else Serial.println("No doubt, signal unstable, please readjust antenna.");
  }
  else if(ReceiveDCF77==ERROR_TIMEOUT)
    Serial.println("\nDCF77 signal unstable, please wait or readjust antenna.");
}

```

Bitte beachten:
Die serielle Debug-Ausgabe ist standardmäßig aktiviert. Sie können Ressourcen sparen, indem Sie die Ausgabe deaktivieren. Sie erreichen dies, durch das Auskommentieren der symbolische Konstante "DEBUG_SERIAL" in der DebugProject.h. Dadurch wird sichergestellt, dass Teile des Codes nicht kompiliert werden (bedingte Kompilierung).

## Debugging

Bei der erstmaligen Verwendung des Moduls können Probleme auftreten, das ist normal. Falls Sie kein Oszilloskop zur Hand haben, hier ein paar Tipps:
Sofern Sie die Debug-Ausgabe in der Datei DebugProject.h nicht deaktiviert haben, werden Sie etwas wie das Folgende sehen:

```
 --=Debug Output Mode=--

Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
Wait for new minute...
00111010000010100010110010011100110110010110111000001001001

It is now 19:49 o'clock CET
Today is 29.3.2024
Weekday: 05
```

Die Debug-Ausgabe sollte sich jede Sekunde aktualisieren. Wenn die Ausgabe unregelmäßig ist, justieren Sie die Antenne, bis sie es ist.
Um den Beginn des DCF77-Strings zu finden, wartet die Bibliothek, bis die gestartete Minute abgelaufen ist. Dann zeichnet sie die Bitsequenz auf, die aus den oben genannten 59 Bits besteht.<br>


Da jeder Mikrocontrollertyp unterschiedlich auf die Funktion pulseIn() reagiert, welche das unsichtbare Herzstück dieser Bibliothek bildet, gibt es eine gewisse Variabilität bei der Messung der Pulslaufzeit.

Sie sollten mit den folgenden Definitionen in der Datei basic_dcf77.h etwas experimentieren:

```C
#define BIT_0_DURATION 100000 //maximum duration (in μs) to interpret a 0
#define BIT_1_DURATION 200000 //maximum duration (in μs) to interpret a 1
#define min_BIT_0_DURATION 20000 //ignores short-term bounces
#define TIMEOUT_DURATION 1000000 //Represents the end of a DCF77 BitString
```

Die obigen Werte stellen das Optimum dar, aber Ihr Mikrocontroller benötigt möglicherweise etwas weniger oder etwas mehr Zeit. Wenn Sie die passenden Werte nicht durch Ausprobieren herausfinden möchten, aktivieren Sie die erweiterte Debug-Ausgabe in der Implementierungsdatei (siehe basic_dcf77.cpp) am Ende der Funktion receiveDCF77(...).<br>


Wenn der Mikrocontroller niemals eine Bitfolge aufzeichnet, kann dies zwei Ursachen haben:

- TIMEOUT_DURATION ist zu hoch und erkennt nicht das Ende der DCF77-Bitfolge.
- Es liegt ein Hardware-Problem vor.

Für das externe Debuggung von DCF77-Bitfolgen können Sie meine Bash-Skripte unter Linux/Unix im Ordner ../examples verwenden.
Wenn Sie einer DCF77-Bitfolge an dcf77_string_decode.sh übergeben, erhalten Sie die Decodierung auf der Kommandozeile. Da es müsig ist, Bits zu zählen, um herauszufinden, welchen Wert sie haben (1 oder 0), können Sie extract_dcf77_BIT.sh verwenden.

## Danke an:
[wollewald](https://github.com/wollewald) für die Quellcode-Organisation und Ideen für zukünftige Arbeiten<br>
[klemens](https://github.com/klemens) für die Durchsicht des Quellcodes<br>

## Lizenz
Dieses Programm wurde von Michael Krause erstellt und unter der GPLv3 veröffentlicht.
