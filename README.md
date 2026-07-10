### PROGETTO DAMA - Intelligent Web - ###

## DAMA ITALIANA
La Dama Italiana è un gioco da tavolo di strategia a informazione perfetta, 
caratterizzato da un insieme di regole complesse e dinamiche. 
L'implementazione software di questo gioco non si limita alla semplice rappresentazione grafica
della scacchiera, ma richiede un motore logico rigoroso e algoritmi di intelligenza artificiale
avanzati per gestire lo spazio degli stati, che cresce in modo esponenziale ad ogni mossa.

Questo progetto presenta lo sviluppo di un'applicazione software completa, scritta in linguaggio C, 
che implementa un motore di gioco della Dama Italiana conforme al regolamento ufficiale della 
Federazione Italiana Dama (FID). Il sistema integra un'interfaccia grafica interattiva, 
un'architettura modulare per l'Intelligenza Artificiale basata sul Monte Carlo Tree Search (MCTS)
e un sistema automatizzato di gestione tornei a eliminazione diretta.

## 1 - COSTRUZIONE DELLA DAMA
La scacchiera della Dama Italiana è una griglia 8×8 composta da 64 caselle, 
di cui solo 32 (quelle scure) sono utilizzabili per il posizionamento dei pezzi.
La funzione **init_board()** posiziona i 24 pezzi iniziali (12 bianchi + 12 neri) 
rispettando la regola delle caselle scure:
   0  1  2  3  4  5  6  7
0 [ ][N][ ][N][ ][N][ ][N]   ← Nero (righe 0-2)
1 [N][ ][N][ ][N][ ][N][ ]
2 [ ][N][ ][N][ ][N][ ][N]
3 [ ][ ][ ][ ][ ][ ][ ][ ]   ← Zona neutra (righe 3-4)
4 [ ][ ][ ][ ][ ][ ][ ][ ]
5 [B][ ][B][ ][B][ ][B][ ]   ← Bianco (righe 5-7)
6 [ ][B][ ][B][ ][B][ ][B]
7 [B][ ][B][ ][B][ ][B][ ]

Il rendering della scacchiera è gestito dalla libreria **Raylib**, che fornisce primitive 2D efficienti 
e cross-platform.

## 2 - MOSSE E REGOLE DELLA DAMA
Il modulo moves.c costituisce il cuore normativo del progetto, implementando integralmente
il regolamento ufficiale della Federazione Italiana Dama (FID). Questo modulo è responsabile di:
Validare ogni mossa proposta (sia dal giocatore umano che dall'IA)
Generare l'insieme delle mosse legali in una data posizione
Applicare le conseguenze di una mossa (spostamento, rimozione pezzi catturati, promozione)
Rilevare le condizioni di fine partita (vittoria, patta)
Il design segue il principio di separazione delle responsabilità: le regole sono completamente
disaccoppiate dall'interfaccia utente e dal motore di ricerca IA, permettendo a entrambi di utilizzare
lo stesso insieme di funzioni di validazione.

La pedina è il pezzo fondamentale della Dama Italiana. Le sue capacità di movimento sono limitate e direzionali:
**pedina nera** = 2
**pedina bianca** = 1

La pedina non può:
Muoversi all'indietro
Muoversi di più di 1 casella
Saltare pezzi (se non per catturare)
Muoversi su caselle occupate

La dama è il pezzo più potente della scacchiera, ottenuto promuovendo una pedina che raggiunge la base avversaria.
Le sue capacità sono significativamente superiori:
**dama nera** = 4
**dama bianca** = 3

 la dama si muove di una sola casella per volta (a differenza della Dama Internazionale dove può muoversi di più caselle).
 Tuttavia, può muoversi in tutte e 4 le direzioni diagonali.

La cattura è l'aspetto più complesso e strategico della Dama Italiana. 
Il regolamento impone obblighi rigorosi che il motore deve rispettare

**Presa obbligatoria**
Regola: Se un giocatore ha la possibilità di catturare uno o più pezzi avversari, deve obbligatoriamente eseguire
la cattura. Non è permesso effettuare una mossa semplice se esiste almeno una cattura disponibile.
**Cattura singola**
Una cattura avviene quando un pezzo salta sopra un pezzo avversario adiacente, atterrando sulla casella vuota immediatamente successiva.
**Priorità**
Quando esistono multiple opzioni di cattura, il regolamento impone una gerarchia di priorità:
Regola 1: Massimo numero di pezzi
Il giocatore deve scegliere la sequenza di catture che elimina il maggior numero di pezzi avversari.
Regola 2: Priorità alle dame
A parità di numero di pezzi catturati, si deve dare precedenza alla cattura di dame avversarie
rispetto alle pedine semplici.
**Presa multipla obbligatoria**
Regola del "Pezzo Toccato": Quando un pezzo inizia una sequenza di catture, deve completarla interamente,
continuando a catturare finché ci sono pezzi avversari raggiungibili
**Promozione**
Regola: Una pedina che raggiunge la base avversaria (ultima riga) viene immediatamente promossa a dama.
Pedina bianca → raggiunge riga 0 → diventa Dama bianca (valore 3)
Pedina nera → raggiunge riga 7 → diventa Dama nera (valore 4)
Regola speciale: Se una pedina raggiunge la base avversaria durante una sequenza di catture, viene 
immediatamente promossa a dama e continua la sequenza come dama (potendo quindi catturare anche all'indietro).
**Terminazione**
Regola: Un giocatore vince quando l'avversario non ha più pezzi sulla scacchiera.
Regola: Un giocatore vince quando l'avversario, pur avendo ancora pezzi, non ha nessuna mossa legale
disponibile (né semplici né catture).
Regola: Se vengono eseguite 40 mosse consecutive senza che nessuno dei giocatori catturi un pezzo, la partita è dichiarata patta
**Patta**
Regola: Se la partita raggiunge 400 mosse totali (somma di tutte le mosse di entrambi i giocatori), è dichiarata patta.
**Mossa invalida**
Regola: Se un giocatore (tipicamente l'IA) restituisce una mossa invalida 
*(codice speciale Move{255, 255})*, l'avversario vince automaticamente

## 3 - ALGORITMO DI MCTS SU PEDINE NERE
 Il MCTS è un algoritmo di ricerca stocastica che non si basa sulla valutazione statica della scacchiera
 , ma sulla simulazione statistica. In parole semplici: l'IA "immagina" migliaia di partite giocate a caso (o con leggera euristica)
 partendo dalla posizione attuale e sceglie la mossa che ha portato alla percentuale di vittorie più alta.

 Il MCTS è particolarmente adatto perché:
Non richiede una funzione di valutazione complessa (basta sapere se alla fine si è vinto, perso o pareggiato).
Si adatta dinamicamente alle regole complesse (come la presa obbligatoria) poiché si basa sul generatore di mosse legali.
È "anytime": può essere interrotto in qualsiasi momento (quando scade il tempo a disposizione) e restituisce comunque la mossa migliore trovata finora.

Per implementare il MCTS in modo efficiente in C, sono state definite strutture dati ottimizzate 
per minimizzare l'overhead di allocazione della memoria.
**Il Nodo dell'Albero (MCTSNode)**
Ogni nodo rappresenta uno stato specifico della scacchiera e una mossa che ci ha portato a quello stato.
**Il Pool di Memoria (MemoryPool)**
Chiamare malloc() per ogni nuovo nodo durante la ricerca rallenterebbe drasticamente l'algoritmo.
Il MemoryPool prealloca un grande array di nodi in memoria statica.

# CICLO MCTS
**Selezione (Selection)**
Partendo dal nodo radice (la posizione attuale), l'algoritmo scende nell'albero scegliendo il figlio "migliore"
fino a raggiungere un nodo foglia (un nodo non ancora completamente espanso).
La scelta del figlio "migliore" bilancia Esplorazione (provare mosse poco visitate) ed Exploitation
(approfondire le mosse che stanno vincendo). Questo bilanciamento è gestito dalla formula UCB1 (Upper Confidence Bound) o PUCT

**Espansione (Expansion)**
Quando la fase di selezione raggiunge un nodo foglia che ha mosse legali non ancora esplorate,
il MCTS chiama la funzione *generate_legal_moves()*.

**Simulazione (Rollout / Playout)**
Dal nuovo nodo espanso, l'IA gioca una partita "leggera" (simulazione) fino a raggiungere uno stato finale 
(vittoria, sconfitta o patta).
Per accelerare il processo, la simulazione non è puramente casuale ma usa una politica di rollout leggera:
Se ci sono catture obbligatorie, vengono eseguite (rispettando le regole).
Se non ci sono catture, viene scelta una mossa semplice a caso tra quelle legali.

**Retropropagazione (Backpropagation)**
Una volta terminata la simulazione, il risultato (es. +1 se il Nero vince, -1 se vince il Bianco, 0 per patta) 
viene propagato all'indietro lungo il percorso dalla foglia fino alla radice, aggiornando le statistiche di ogni nodo attraversato.

Quando il tempo a disposizione *(aiTimeLimit)* scade, il ciclo MCTS si interrompe. L'IA deve ora decidere quale mossa giocare.
La strategia standard è la "Robust Child": scegliere il nodo figlio della radice con il maggior numero di visite (visits),
non necessariamente quello con il tasso di vittoria più alto. Un alto numero di visite indica che l'algoritmo ha "fiducia"
in quella mossa, avendola esplorata ripetutamente perché promettente.

Il flusso di esecuzione quando tocca al Nero (IA) nel gioco singolo è il seguente:
Conversione Stato: La matrice board[8][8] viene convertita in Bitboard tramite board_to_bitboard().
Inizializzazione Radice: Viene creato il nodo radice del MCTS con lo stato attuale.
Loop di Ricerca.
Esecuzione: La mossa restituita da *select_best_move()* viene applicata alla scacchiera tramite *apply_ai_move()*.

## 4 - ALGORITMI UCB1 E PUCT + VARIANTI
Nel cuore dell'algoritmo MCTS, durante la Fase 1 (Selezione), l'algoritmo deve decidere quale nodo figlio visitare. 
Si trova di fronte al classico dilemma del Multi-Armed Bandit (MAB)
Sfruttamento (Exploitation): Visitare il nodo che ha mostrato il miglior tasso di vittoria finora.
Esplorazione (Exploration): Visitare i nodi che sono stati visitati poche volte,
perché potrebbero nascondere una mossa vincente che non abbiamo ancora scoperto.

Se l'IA sfrutta troppo, potrebbe perdere una mossa geniale ma controintuitiva.
Se esplora troppo, spreca tempo calcolando mosse palesemente sbagliate.
Per risolvere questo dilemma, il progetto implementa due distinte politiche di selezione: UCB1 e PUCT.

# ucb1
L'UCB1 è l'algoritmo storico e più diffuso per la selezione nel MCTS. La sua formula matematica calcola un 
"punteggio di urgenza" per ogni nodo figlio.

# puct
Il PUCT è l'evoluzione moderna dell'UCB1, resa celebre da AlphaGo e AlphaZero di DeepMind.
La differenza fondamentale è l'introduzione di una Probabilità a Priori (P).

Il sistema di torneo mette a confronto 8 "motori" IA. Tutti usano il MCTS, ma differiscono per la politica 
di selezione (UCB1 vs PUCT) e per i parametri interni.

UCB1 Classic: Implementazione standard con costante teorica funge da baseline per bilanciare esplorazione e sfruttamento.
UCB1 Delta: Adatta dinamicamente la costante di esplorazione in base alla varianza dei risultati per ridurre l'incertezza statistica.
UCB1 Alpha: Riduce progressivamente la costante di esplorazione con la profondità, favorendo mosse ampie all'inizio e scelte greedy alla fine.
UCB1 Fast: Ottimizzata per la velocità con costante ridotta e rollout semplificati, ideale per tempi di calcolo brevissimi.
PUCT Standard: Utilizza la formula polinomiale con probabilità a priori uniformi e costante 1.2 per un'esplorazione fluida e continua.
PUCT Explorative: Imposta un'altissima costante di esplorazione (2.5) per forzare l'analisi di rami insoliti e scovare trappole a lungo termine.
PUCT Heuristic: Guida la ricerca fin dalla radice assegnando probabilità a priori più alte alle mosse tattiche e al controllo del centro.
PUCT Balanced: Approccio conservativo con costante bassa (1.0) che privilegia lo sfruttamento di mosse solide e posizionali.

## 5 - ORGANIZZAZIONE TORNEO
Il modulo tournament.c e tournament_main.c implementa un sistema di torneo che orchestra le partite, gestisce il tabellone
a eliminazione diretta e raccoglie le statistiche per produrre una classifica finale.
In questa sezione analizzeremo la struttura del torneo implementato, le motivazioni progettuali e il confronto con l'approccio
alternativo del Round Robin (girone all'italiana).

Il torneo implementato segue il formato classico dell'eliminazione diretta a 8 partecipanti (single-elimination bracket)
┌─────────────────────────────────────────────────────────────────┐
│                    FASE 1: QUARTI DI FINALE                      │
│                    (4 match, 8 IA partecipanti)                  │
├─────────────────────────────────────────────────────────────────┤
│  Match 1: IA0 vs IA1                                            │
│  Match 2: IA2 vs IA3                                            │
│  Match 3: IA4 vs IA5                                            │
│  Match 4: IA6 vs IA7                                            │
└────────────────────────────┬────────────────────────────────────┘
                             │ Vincitori avanzano
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    FASE 2: SEMIFINALI                            │
│                    (2 match, 4 IA partecipanti)                  │
├─────────────────────────────────────────────────────────────────┤
│  Match 5: Vincitore Q1 vs Vincitore Q2                          │
│  Match 6: Vincitore Q3 vs Vincitore Q4                          │
└────────────────────────────┬────────────────────────────────────┘
                             │ Vincitori e perdenti separati
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    FASE 3: FINALI                                │
│                    (2 match)                                     │
├─────────────────────────────────────────────────────────────────┤
│  Match 7 (FINALE):         Vincitore S1 vs Vincitore S2         │
│  Match 8 (3° POSTO):       Perdente S1  vs Perdente S2          │
└─────────────────────────────────────────────────────────────────┘
La logica di progressione è gestita nel main() di *tournament_main.c* attraverso il contatore *current_match*:

# PUNTEGGI

Oltre al risultato binario (vittoria/sconfitta), il torneo implementa un sistema di punteggio cumulativo per 
produrre una classifica più sfumata e statisticamente significativa.
Il punteggio di ogni IA in un match è calcolato come il numero di pedine avversarie catturate:
Se alla fine del match il Bianco ha ancora 8 pezzi e il Nero ne ha 4:
Punti del Bianco: 12 - 4 = 8 (ha mangiato 8 pezzi neri)
Punti del Nero: 12 - 8 = 4 (ha mangiato 4 pezzi bianchi)
I punti vengono sommati match dopo match nell'array globale *engine_points[8]*.
La classifica finale è ordinata per punti cumulativi decrescenti, non per vittorie.

# FLUSSO TORNEP
┌─────────────────────────────────────────────────────────────────┐
│ AVVIO TORNEO                                                    │
│ - Inizializza engine_points[8] = {0}                            │
│ - Inizializza engine_wins[8] = {0}                              │
│ - setup_bracket() con le 8 IA registrate                        │
└──────────────────────────┬──────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│ LOOP: current_match da 0 a 7                                    │
│                                                                 │
│   play_tournament_game(p1, p2, ...)                             │
│       │                                                         │
│       ├─► Calcola pts1, pts2 (pedine mangiate)                  │
│       ├─► Determina vincitore (res >= 0.9 o <= 0.1)             │
│       ├─► engine_points[idx] += punti                           │
│       ├─► engine_wins[idx]++ se vincitore                       │
│       └─► matches[current_match].winner = ...                   │
│                                                                 │
│   current_match++                                               │
│                                                                 │
│   Se current_match == 4: genera semifinali                      │
│   Se current_match == 6: genera finali                          │
└──────────────────────────┬──────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────────┐
│ TORNEO COMPLETATO                                               │
│                                                                 │
│   Ordina engine_points[] in ordine decrescente                  │
│   Disegna classifica finale con draw_report()                   │
│                                                                 │
│   Output:                                                       │
│   #1 - PUCT Heuristic - 45 punti - 7 vittorie                   │
│   #2 - UCB1 Classic   - 38 punti - 5 vittorie                   │
│   #3 - PUCT Balanced  - 32 punti - 4 vittorie                   │
│   ...                                                           │
└─────────────────────────────────────────────────────────────────┘

## 6 - CONCLUSIONI
Questo progetto ha dimostrato come sia possibile implementare un sistema completo di gioco della Dama Italiana che unisce rigore normativo, intelligenza artificiale avanzata e architettura software modulare in un'unica applicazione coesa scritta in C.
Gli obiettivi iniziali sono stati pienamente raggiunti:
Regolamento ufficiale implementato: Il motore di gioco gestisce correttamente tutte le regole complesse della Dama Italiana, dalla presa obbligatoria alle priorità di cattura, dalla presa multipla alle condizioni di terminazione (vittoria per sfinimento, blocco, patta per stallo o limite mosse).
Sistema IA modulare e scalabile: L'architettura a plugin ha permesso di sviluppare e far competere 8 varianti distinte di algoritmi MCTS (4 UCB1 + 4 PUCT) senza modificare il motore di ricerca centrale, dimostrando la validità del pattern Strategy.
Torneo automatizzato funzionante: Il sistema orchestra correttamente un tabellone a eliminazione diretta con 8 partecipanti, calcola punteggi cumulativi basati sulle performance e genera report finali ordinati.
Performance ottimizzate: L'uso di Bitboard e Memory Pool ha permesso di eseguire migliaia di simulazioni MCTS al secondo, mantenendo tempi di risposta dell'IA inferiori al secondo anche con configurazioni complesse.
