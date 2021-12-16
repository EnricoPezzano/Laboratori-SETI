# to do list: :)

# 1. Leggere dal file throughput.dat (generato mediante una interazione con il server Pong su webdev.disi.unige.it) la 
# prima e l'ultima riga ed estrarre i valori N1 e N2, T(N1) e T(N2). Si vedano i comandi head e tail per leggere le 
# righe dal file.

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


declare pippo;
