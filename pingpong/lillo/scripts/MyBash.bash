#!/bin/bash

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

#scale = 9 stabilisce quanti decimali
#calcolo l0
l0=$(bc -l <<< "scale = 9;((($delayn1*$n2tcp) - ($delayn2*$n1tcp)) / ($n2tcp-$n1tcp))")

#calcolo B
B=$(bc -l <<< "scale = 9;(($n2tcp-$n1tcp) / ($delayn2-$delayn1))")

gnuplot <<-eNDgNUPLOTcOMMAND
	set term png size 900, 700 #determino size png
	set output "../data/throughputTCP.png" #determino nome del png in uscita
	set logscale x 2
	set logscale y 10
	set xlabel "msg size (B)"
	set ylabel "throughput (KB/s)"
	lbf(x) = x / ( $l0 + x / $B )
	plot "../data/tcp_throughput.dat" using 1:3 title "TCP median Throughput" \
			with linespoints, \
		 lbf(x) title "Latency-Bandwidth model with L=$l0 and B=$B" with linespoints
	clear
eNDgNUPLOTcOMMAND

readonly InputUDPFile="../data/udp_throughput.dat"

declare -a firstudp=($(head --lines=1 ${InputUDPFile}))
declare -a lastudp=($(tail --lines=1 ${InputUDPFile}))

declare -i n1udp=${firstudp[0]}
declare -i n2udp=${lastudp[0]}

declare -a medianudp1=${firstudp[2]}
declare -a medianudp2=${lastudp[2]}

delayn3=$(bc -l <<< "($n1tcp/$medianudp1)")
delayn4=$(bc -l <<< "($n2tcp/$medianudp2)")

l0=$(bc -l <<< "scale = 9;((($delayn3*$n2udp) - ($delayn4*$n1udp)) / ($n2udp-$n1udp))")

B=$(bc -l <<< "scale = 9;(($n2udp-$n1udp) / ($delayn4-$delayn3))")

gnuplot <<-eNDgNUPLOTcOMMAND
	set term png size 900, 700
	set output "../data/throughputUDP.png"
	set logscale x 2
	set logscale y 10
	set xlabel "msg size (B)"
	set ylabel "throughput (KB/s)"
	lbf(x) = x / ( $l0 + x / $B )
	plot "../data/udp_throughput.dat" using 1:3 title "UDP median Throughput" \
			with linespoints, \
		 lbf(x) title "Latency-Bandwidth model with L=$l0 and B=$B" with linespoints
	clear
eNDgNUPLOTcOMMAND
