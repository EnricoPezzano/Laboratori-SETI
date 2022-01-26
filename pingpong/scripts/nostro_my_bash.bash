#!/bin/bash 
#...è una direttiva 
# "#!" è detto shebang, /bin/bash indica l'interprete di comandi usato, in questo caso proprio bash

# 1) devo raccogliere i dati sia per tcp che udp 
#		dal file *_throughput.dat
#		--> declare -a arra=("tcp" "udp")
#		--> readonly TcpDataFilename && UdpDataFilename;

#	2) dichiaro il resto delle variabili (array 2 elem) che 
#		serviranno (N1, N2, T1, T2, L0, Bandw, Delay)
#
#		!! (cut -d " " -fX) -f è un po l index dell array di str;
#
#	3) eseguo tutte le seguenti OP sia per arra[tcp] che udp;
#
#	4) ottengo msg_size e throughput MEDIO
#		sia per head che tail;
#
#	5) usando la formula inversa (Delay = msg_size/T) calcolo 
#		DelayMin & DelayMax
#
#		!! use byte calculator (man bc);
#
#	6) Usando formule aulaweb calcolo L0 e Bandw;
#
#	7) Creo grafici tcp e udp in scala log con Gnuplot;

	set -e 
#1) Raccolgo i dati sia per tcp che udp dal file 
#	*_throughput.dat
#	--> declare -a arra=("tcp" "udp");
# 2) dichiaro il resto delle variabili (array 2 elem)
#	che serviranno (N1, N2, T1, T2, L0, Bandw, Delay)';

	declare -a arra=("tcp" "udp")
	declare  N1
	declare  N2
	declare  Th_n1
	declare  Th_n2


	#3) eseguo tutte le seguenti OP sia per arra[tcp] che udp;
	#4) ottengo msg_size e throughput MEDIO sia per head che tail;

	for index in "${arra[@]}"
	do
		N1=$(head -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f1)
		N2=$(tail -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f1)

		Th_n1=$(head -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f3)
		Th_n2=$(tail -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f3)

	#The -e option is used to enable echo's interpretation
	#of additional instances of the newline character as 
	#well as the interpretation of other special characters,
	#such as a horizontal tab, which is represented by \t.
	echo "$index":
	echo size min: "$N1" 
	echo size max: "$N2"
	echo throughput min: "$Th_n1"
	echo throughput max: "$Th_n2"


	#5) usando la formula inversa (Delay = msg_size/T) calcolo 
	#	DelayMin & DelayMax

	#	!! use byte calculator (man bc);
	
	declare  DelayMin
	declare  DelayMax
	DelayMin=$(bc <<<"scale=10; var1=${N1}; var2=${Th_n1}; var1/var2")
	DelayMax=$(bc <<<"scale=10; var1=${N2}; var2=${Th_n2}; var1/var2")

	echo DelayMin: "$DelayMin"
	echo DelayMax: "$DelayMax"

	#6) Usando formule aulaweb calcolo L0 e Bandw
	# Bandw = (N2 - N1) / ( D(N2) - D(N1) ); 
	declare Bandw 
	declare L0
	Bandw=$(bc <<<"scale=10; n2=${N2}; n1=${N1}; dmin=${DelayMin}; dmax=${DelayMax}; ((n2-n1)/(dmax-dmin))")
	L0=$(bc <<< "scale=10; n2=${N2}; n1=${N1}; dmin=${DelayMin}; dmax=${DelayMax}; ( (dmin*n2) - (dman*n1) ) / (n2-n1)")
	
	echo Banda: "$Bandw"
	echo Latenza 0: "$L0"


	#7) Elimino old files && Grafico;
	if test -f "${index}_banda_latenza.png"; then rm "${index}_banda_latenza.png"
	fi

	#creo il nuovo grafico
	gnuplot <<-eNDgNUPLOTcOMMAND
		set term png size 900, 700 
		set output "../data/${index}_banda_latenza.png"
		set logscale y 10
		set logscale x 2
		set xlabel "msg size (B)"
		set ylabel "throughput (KB/s)"
		lbf(x) = x / ($L0 + x / $Bandw)
		plot "../data/${index}_throughput.dat" using 1:2 title "${index} ping-pong average Throughput" \
			with linespoints, \
		lbf(x) title "Latency-Bandwidth model with L=${L0} and B=${Bandw}" \
			with linespoints
		clear
	eNDgNUPLOTcOMMAND
done
