#!/bin/bash

#Licence
#This program by Michael Krause is licenced under the terms of the GPLv3.

DCF77=$1

decode() {
    local value=0
    local bitstring=$1
    local -a weights=(1 2 4 8 10 20 40 80)
    for ((i=0; i<${#bitstring}; i++)); do
        if [ "${bitstring:$i:1}" == "1" ]; then
            value=$((value + weights[i]))
        fi
    done
    echo $value
}

# Extrahiere die relevanten Bits für Stunden, Minuten und Tag/Monat/Jahr
HOUR=$(echo $DCF77 | cut -c 30-35)
MINUTE=$(echo $DCF77 | cut -c 22-28)
DAY=$(echo $DCF77 | cut -c 37-42)
MONTH=$(echo $DCF77 | cut -c 46-50)
YEAR=$(echo $DCF77 | cut -c 51-58)

# Dekodiere die Bitstrings
HOUR_DEC=$(decode $HOUR)
MINUTE_DEC=$(decode $MINUTE)
DAY_DEC=$(decode $DAY)
MONTH_DEC=$(decode $MONTH)
YEAR_DEC=$(decode $YEAR)

# Gib die Uhrzeit und das Datum aus
echo "Uhrzeit: $HOUR_DEC:$MINUTE_DEC"
echo "Datum: $DAY_DEC.$MONTH_DEC.20$YEAR_DEC"
echo "Stunde(binär): $HOUR"
echo "Minuten(binär): $MINUTE"
echo "Tag(binär): $DAY"
echo "Monat(binär): $MONTH"
echo "Jahr(binär): $YEAR"
