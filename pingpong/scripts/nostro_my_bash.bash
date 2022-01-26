#declare pippo;
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
L=$(bc <<< "scale=20;var1=${DelayMin}*${N2};var2=${DelayMax}*${N1};var3=var1-var2;var3/${Denominatore}")
B=$(bc <<< "scale=20;var1=${N2}-${N1};var1/${Denominatore}")

echo bandwiwidth: $B
echo latency: $L
#stampa i valori Numero_byte e Latenza sul file .dat
echo 'stampo i valori Numero_byte e Latenza sul file .dat'
N_LINEE_FILE=$(wc -l "${DataDir}/${ProtocolName}_throughput.dat" | cut -d ' ' -f1)
NUMERO_LINEA=1

while [ $NUMERO_LINEA -lt $N_LINEE_FILE ]
do
    N=$(sed "${NUMERO_LINEA}q;d" ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f1)
    D=$(bc <<<"scale=20;var1=${L};var2=${N};var3=${B};var1 + (var2 / var3)")
    echo $N $D
    printf "$N $D \n" >> ${OutputDatFile}
    ((NUMERO_LINEA++))
done

echo ridimensiono costanti:
L0=$(bc <<< "scale=2;$L")
B0=$(bc <<< "scale=2;$B")

echo "L0=$L0"
echo "B0=$B0"


# 3. Realizzare due grafici (uno per TCP e uno per UDP) come quelli riportati nelle due figure; dalla 
# documentazione di gnuplot (oppure dagli script ...) ricavate i comandi per disegnare curve in scala logaritmica.
# Suggerimento: il modo più semplice per rappresentare la formula Banda-Latenza in gnuplot dovrebbe 
# essere mediante la definizione di una funzione, per esempio:
#   lbf(x) = x / ( $myL + x / $myB )
#       plot lbf(x) title "Latency-Bandwidth model with L=$myL and B=$myB" with linespoints
gnuplot <<-eNDgNUPLOTcOMMAND
    set term png size 900,700
    
    set logscale x 2
	set logscale y 10
	set xlabel "msg size (B)"
	set ylabel "throughput (KB/s)"
	
	set output "../data/$OutputPngFile"
	plot "../data/${OutputDatFile}" using 1:2 title "Latency-Bandwidth model with L=${Latenza} and B=${Banda}"\
	    with linespoint, \
	     "../data/${InputFile}" using 1:3 title "${ProtocolName} ping-pong Throughput (average)" \
			with linespoints
	clear
eNDgNUPLOTcOMMAND
    echo 'grafico creato e disponibile nella cartella: data'