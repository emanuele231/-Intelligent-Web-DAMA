#ifndef MOVES_H
#define MOVES_H

/*MOVIMENTI

1- Movimento pedine: in diagonale dx/sx
2- Cattura: salto della pedina avversaria 
    if (in diagonale sx/dx ce una pedina bianca)
          salta pedina 
          cancella pedina bianca
          punti +1
3- Movimento dama: if (pedina arriva al bordo opposto della tavola)
                         diventa dama
                         sblocca Movimento dxa sxa dxd sxd
4- presa multipla: continua a mangiare se rileva pedine nel suo quadrante (diag)
*/