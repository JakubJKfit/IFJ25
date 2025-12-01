/**
 * @name IFJ25
 * 
 * @file err.h
 * @author Jakub Jan Kupčík xkupcij00
 * @brief Hlavičkový soubor deklarující všechny návratové hodnoty
 */

#ifndef ERR_H
#define ERR_H

#define ERR_LEX 1 // lexikalni chyba
#define ERR_SYN 2 // syntakticka chyba
#define ERR_SEM_UNDEFINED 3 // nedefinovana fce/promenna
#define ERR_SEM_REDEFINED 4 // redefinice fce/promenne
#define ERR_SEM_PARAM 5 // spatny pocet argumentnu / typ parametru
#define ERR_SEM_INCOMP 6 // nekompatibilni typy
#define ERR_SEM_OTHER 10 // ostatni chyby
#define ERR_RSEM_PARAM 25 // spatny typ parametru vestavene fce, za behu
#define ERR_RSEM_INCOMP 26 // nekompatibilni typy, za behu
#define ERR_INTERNAL 99 // interni chyba

void ifjexit(int err_code);

#endif // ERR_H