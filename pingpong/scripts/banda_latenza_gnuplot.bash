#!/bin/bash
if [[ $# != 0 ]]; then
	printf "\nError: Didn't expected parameters\n\n";
	exit 1;
fi

set -e
#The -e option is used to enable echo's interpretation
	#of additional instances of the newline character as 
	#well as the interpretation of other special characters,
	#such as a horizontal tab, which is represented by \t.

#raccolgo i dati dai rispettivi file sia per tcp che per udp
declare -a protocol=("tcp" "udp")
declare  N1
declare  N2
declare  Th_n1
declare  Th_n2

#ciclo for "doppio" per le operazioni ripetute sia per tcp che per udp
for index in "${protocol[@]}"
do
    #ottengo msg_size e throughput MEDIO sia per head che tail;
    N1=$(head -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f1) #cut -d ' ' -fn è come se fosse "l'indice" dell'array
    N2=$(tail -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f1)

    Th_n1=$(head -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f3)
    Th_n2=$(tail -n 1 ../data/"${index}"_throughput.dat | cut -d ' ' -f3)


    echo "$index":
    echo size min: "$N1" 
    echo size max: "$N2"
    echo throughput min: "$Th_n1"
    echo throughput max: "$Th_n2"


    #5) usando la formula inversa (Delay = msg_size/T) calcolo DelayMin & DelayMax

    
    declare  DelayMin
    declare  DelayMax
    DelayMin=$(bc <<<"scale=10; var1=${N1}; var2=${Th_n1}; var1/var2")
    DelayMax=$(bc <<<"scale=10; var1=${N2}; var2=${Th_n2}; var1/var2")

    echo DelayMin: "$DelayMin"
    echo DelayMax: "$DelayMax"

    #6) Usando formule aulaweb calcolo L0 e Bandw
    # Bandw = (N2 - N1) / ( D(N2) - D(N1) ); 
    #	6) Usando formule aulaweb calcolo L0 e Bandw;
    declare Bandw 
    declare L0
    Bandw=$(bc <<<"scale=10; n2=${N2}; n1=${N1}; dmin=${DelayMin}; dmax=${DelayMax}; ((n2-n1)/(dmax-dmin))")
    L0=$(bc <<< "scale=10; n2=${N2}; n1=${N1}; dmin=${DelayMin}; dmax=${DelayMax}; ( (dmin*n2) - (dman*n1) ) / (n2-n1)")
    
    echo Banda: "$Bandw"
    echo Latenza 0: "$L0"


    #7) Elimino old files && Grafico;
    #	7) Creo grafici tcp e udp in scala log con Gnuplot;
    if test -f "${index}_banda_latenza.png"; then
        rm "${index}_banda_latenza.png"
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
#...or simply don't indent the end token:

done
xdg-open throughput.png, tcp_banda_latenza, udp_banda_latenza &
#una volta finito di controllare il codice, rifai test e grafici su ubuntu