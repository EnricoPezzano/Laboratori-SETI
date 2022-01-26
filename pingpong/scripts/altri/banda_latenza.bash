#!/bin/bash

set -e

declare -a N1
declare -a N2
declare -a arr=("tcp" "udp")
readonly DataDir='../data' 
declare -a DeltaDelay
declare -a DeltaByte
declare -a L
declare -a B
declare -a T1
declare -a T2

for ProtocolName in "${arr[@]}"
do
    echo -e "\e[96mProtocollo\e[0m: $ProtocolName"
    declare InputFile="${DataDir}/${ProtocolName}_throughput.dat"
    declare OutputPngFile="${DataDir}/${ProtocolName}_band_lat.png"
    declare OutputDatFile="${DataDir}/${ProtocolName}_band_lat.dat"

    if [ -e $OutputDatFile ] 
    then
        rm $OutputDatFile $OutputPngFile -f 
    fi

    # memorizzazione del numero byte minimo e del suo throughput
    N1=$(head -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f1)
    T1=$(head -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f2)
    
    # memorizzazione del numero byte minimo e del suo throughput
    N2=$(tail -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f1)
    T2=$(tail -n 1 ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f2)
    

    DelayMin=$(bc <<<"scale=10;var1=${N1};var2=${T1};var1 / var2")
    DelayMax=$(bc <<<"scale=10;var1=${N2};var2=${T2};var1 / var2")
    DeltaByte=$(bc <<< "scale=10;${N2}-${N1}")
    DeltaDelay=$(bc <<< "scale=10;${DelayMax}-${DelayMin}")

    #calcolo latenza e banda
    L=$(bc <<< "scale=10;var1=${DelayMin}*${N2};var2=${DelayMax}*${N1};var3=var1-var2;var3/${DeltaByte}")
    B=$(bc <<< "scale=10;var1=${N2}-${N1};var1/${DeltaDelay}")

    N_LINEE_FILE=$(wc -l "${DataDir}/${ProtocolName}_throughput.dat" | cut -d ' ' -f1)
    ((N_LINEE_FILE++))
    NUMERO_LINEA=1

    #salvo valori sul file che andrà a costruire il grafico (num byte, throughput)
    while [ $NUMERO_LINEA -lt $N_LINEE_FILE ]
    do 
        N=$(sed "${NUMERO_LINEA}q;d" ${DataDir}/${ProtocolName}_throughput.dat | cut -d' ' -f1)
        D=$(bc <<<"scale=10;var1=${L};var2=${N};var3=${B};var2/(var1 + (var2 / var3))")
        printf "$N $D \n" >> ${OutputDatFile}
        ((NUMERO_LINEA++))
    done
    
    Latenza=$(bc <<< "scale=2;$L")
    Banda=$(bc <<< "scale=2;$B")
    echo "Latenza=$Latenza"
    echo "Banda=$Banda"
    echo 'sto creando il grafico...'
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

done