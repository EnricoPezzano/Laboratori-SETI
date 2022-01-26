declare pippo;
#!/bin/bash #è una direttiva 
# "#!" è detto shebang, /bin/bash indica l'interprete di comandi usato, in questo caso proprio bash

# 1. Leggere dal file throughput.dat (generato mediante una interazione con il server Pong su webdev.disi.unige.it) la 
# prima e l'ultima riga ed estrarre i valori N1 e N2, T(N1) e T(N2). Si vedano i comandi head e tail per leggere le 
# righe dal file.

set -e

declare -a arr=("tcp" "udp")
readonly DataDir='../data'
declare -a L0
declare -a B
declare -a N1
declare -a N2
declare -a T1
declare -a T2
declare -a Denominatore


for ProtocolName in "${arr[@]}"
do
    echo -e "\e[96mProtocollo\e[0m: $ProtocolName"
    declare InputFile="${DataDir}/${ProtocolName}_throughput.dat"
    declare OutputPngFile="${DataDir}/${ProtocolName}_banda_latenza.png"
    declare OutputDatFile="${DataDir}/${ProtocolName}_delay.dat"

    if [ -e $OutputDatFile ]
    then
        rm $OutputDatFile $OutputPngFile -f
    fi
    #ottengo parametri
    #f2 0 f3 cioè devo usare T come valore mediano o medio?

	# estraggo no byte del minimo:
    N1=$(head -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f1)
    # estraggo delay del minimo:
    T1=$(head -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f2)

	# estraggo no byte del massimo:
    N2=$(tail -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f1)
    # estraggo delay del massimo:
    T2=$(tail -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f2)

    #calcolo le costanti
    echo 'calcolo costanti'
    # il delay è legato al thorughtput medio e al numero di byte dalla formula T*D=N -> D=N/T
    DelayMin=$(bc <<<"scale=20;var1=${N1};var2=${T1};var1 / var2")
    DelayMax=$(bc <<<"scale=20;var1=${N2};var2=${T2};var1 / var2")
    Denominatore=$(bc <<< "scale=20;${DelayMax}-${DelayMin}")





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
