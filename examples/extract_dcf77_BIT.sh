#!/bin/bash

#Licence
#This program by Michael Krause is licenced under the terms of the GPLv3.

#Dieses Skript gibt den Wert des n-ten Bits eines 60 Bit langen DCF77 Strings zurück.
#Bitte Beachten Sie, dass dieses Skript nicht von 0 Anfängt zu zählen, sondern von 1.

# Überprüfe die Anzahl der Parameter, mit der das Skript aufgerufen wurde.
if [ "$#" -ne 2 ]; then
    echo "Fehler: Falsche Anzahl von Parametern."
    echo "Verwendung: $0 <String> <Bitposition>"
    exit 1
fi

# Der erste Parameter ($1) ist der DCF77 String
# Der zweite Parameter ($2) ist die Position des Bits, das extrahiert werden soll

# Überprüfen, ob die Bitposition innerhalb der Länge des Strings liegt
if [ "$2" -gt "${#1}" ] || [ "$2" -lt 1 ]; then
    echo "Fehler: Bitposition außerhalb des gültigen Bereichs."
    exit 1
fi

bit=$(echo $1 | cut -c$2-$2)

echo "Das Bit an der Position $2 ist: $bit"
