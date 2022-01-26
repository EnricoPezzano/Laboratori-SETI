declare pippo;
#!/bin/bash #è una direttiva 
# "#!" è detto shebang, /bin/bash indica l'interprete di comandi usato, in questo caso proprio bash

# 1. Leggere dal file throughput.dat (generato mediante una interazione con il server 
# Pong su webdev.disi.unige.it) la prima e l'ultima riga ed estrarre i valori N1 e N2,
# T(N1) e T(N2). Si vedano i comandi head e tail per leggere le righe dal file.

set -e

#controllo se i file .dat sono presenti nella cartella data
if [ ! -f ../data/tcp_throughput.dat ]; then
    printf "\nUnable to find tcp_throughput source file\n\n"
    exit 1
fi

if [ ! -f ../data/udp_throughput.dat ]; then
    printf "\nUnable to find udp_throughput source file\n\n"
    exit 1
fi

#crea variabile di sola lettura
readonly InputTCPFile="../data/tcp_throughput.dat"

#creo variabili con prima e ultima riga del file input
declare -a firsttcp=($(head -n 1 ${InputTCPFile}))  
declare -a lasttcp=($(tail -n 1 ${InputTCPFile}))

#crea due interi in cui mette il primo numero di ciascuna riga
declare -i n1tcp=${firsttcp[0]}
declare -i n2tcp=${lasttcp[0]}

#variabile che contiene il secondo numero di ciascuna riga
declare -a mediantcp1=${firsttcp[2]}
declare -a mediantcp2=${lasttcp[2]}

#n1tcp/mediantcp
#bc -> fai operazione matematica
#-l -> libreria mathlib
#D = N/T
declare delayn1=$(bc -l <<< "($n1tcp/$mediantcp1)")
declare delayn2=$(bc -l <<< "($n2tcp/$mediantcp2)")





# 2. Ricavare i parametri B e L0 che caratterizzano la formula del modello Banda-Latenza.
# Attenzione: per il calcolo dei valori in floating point non si può usare direttamente bash (che definisce 
# solo variabili di tipo intero o di tipo stringa). Si suggerisce quindi di lanciare l'applicazione bc (vedi 
# man bc) che permette di effettuare calcoli in virgola mobile con precisione arbitrariamente predefinita (e di 
# effettuare calcoli con una precisione di almeno 9 cifre decimali).




# 3. Realizzare due grafici (uno per TCP e uno per UDP) come quelli riportati nelle due figure; dalla 
# documentazione di gnuplot (oppure dagli script ...) ricavate i comandi per disegnare curve in scala logaritmica.
# Suggerimento: il modo più semplice per rappresentare la formula Banda-Latenza in gnuplot dovrebbe 
# essere mediante la definizione di una funzione, per esempio:
#   lbf(x) = x / ( $myL + x / $myB )
#       plot lbf(x) title "Latency-Bandwidth model with L=$myL and B=$myB" with linespoints
