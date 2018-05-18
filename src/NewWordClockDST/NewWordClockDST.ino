
// MIT License
//
// Copyright (c) 2017 Tullio Benzi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ================================================================================
// Scopo
// visualizzazione dell'ora mendiante scritte che vengono illuminate dall'accensione
// di led sottostanti:
//                      .------------------------------.
//                      |  IT IS       HALF       TEN  |
//                      |                              |
//                      |      QUARTER     TWENTY      |
//                      |                              |
//                      |  FIVE       MINUTES     TO   |
//                      |                              |
//                      |  PAST        ONE      THREE  |
//                      |                              |
//                      |  TWO        FOUR       FIVE  |
//                      |                              |
//                      |  SIX        SEVEN      EIGH  |
//                      |                              |
//                      |  NINE        TEN     ELEVEN  |
//                      |                              |
//                      |  TWELVE     O’CLOCK     DST  |
//                      |                              |
//                      |  AM   •    •    •    •   PM  |
//                      '------------------------------'
// Le scritte sono in lingua inglese, la visualizzazione ha una granularità
// principale di cinque minuti (o'clock - five - ten ...)
// sono aggiunti del puntini (4 puntini) che portano la granularità al minuto
// è indicato se AM o PM e se è attivo il DST ("Daylight Saving time" ossia ora legale)
// esempi
// 5:00 it is "five" "o'clock"
// 5:27 it is "twenty" "five" "minutes" "past" "five" "." "." -> 5:25 + 2 puntini
// 5:41 it is "quarter" "to" "five" "."  "."  "." "."         -> 5:15 + 4 puntini
//
// è prevista una gestione automatica dell'ora legale valida fino al 2031
//
// Oltre alla board Arduino è previsto l'utilizzo di
//  Real Time Clock(RTC) con modulo 3231
//  Bluetooth con modulo HTC05 o HTC06
//
// per gestire l'orologio sono previsti 4 pulsanti:
//      MODE    effettua il cambio di modalità
//      SET     imposta l'RTC
//      +       incrementa il valore delle ore e dei minuti
//      -       decrementa il valore delle ore e dei minuti
//
// collegando una seriale si:
//
//  - visualizza l'ora ed i minuti letti da RTC(in modalità STANDARTD)
//               l'ora ed i minuti che si imposteranno (in modalità SET_CLOCK)
//               la loro codifica in testo
//
//   possono dare dei comandi per:
//  - impostare l'RTC completamente anno, mese, mese, giorno, ore, minuti
//          inviando una stringa da 14 caratteri (aa,mm,gg,hh,mm)
//  - abilitare il test delle uscite
//          inviando una stringa da 2 caratteri ET (Enable Test) oppure qualunque coppia di caratteri per disabilitare
//  - variare la lunminosità dei led impostandi il duty cycle dell'uscta PWM utilizzata per variare (tramite darlington)
//          la tensione di alimentazione dei LED
//
// comandi previsti da seriale:
//  - una stringa da 14 caratteri (aa,mm,gg,hh,mm) set RTC
//  - una stringa da 2 caratteri ET (Enable Test) oppure qualunque coppia di caratteri
//       per disabilitare il test
//  - una stringa da 1 carattere: N (Next) per passare al prossimo LED in caso di Test LED con step singolo
//                               (qualsiasi diverso da N) per l'help dei comandi
//  - una stringa da 4 caretteri Lnnn dove nnn va da 000 a 255 valore duty cycle uscita PWM
//
// Connettendosi in bluetooth si possono fare le stesse cose che si fanno con la
// seriale PC
//
// Le scritte verso bluetooth sono pensate per utilizzare l'app "Bluetooth Electronics" presente su PlayStore
//  in cui ho fatto una maschera che ha:
//  .-------------------------------------------.
//  |      Titolo                               |
//  '-------------------------------------------'
//  .-------------------------------------------.
//  |                                           |
//  |    terminale                              |   Monitor 14x48
//  |                                           !
//  '-------------------------------------------'
//  .------------------------------------.------.
//  |    stringa da inviare              | SEND |
//  '------------------------------------'------'
//  I testi che vanno nel Titolo sono preceduti da *# e terminano con *
//  I testi che vanno sul terminale sono preceduti da *@
// ================================================================================
// Modalità previste:
//  STANDARD:   normale visualizzazione dell'orario
//  SET_CLOCK:  impostazione dell'orologio
//  LED_TEST:   test ciclico dei led che si accendono tutti in sequenza
//
// Le modalità sono gestite con una macchina a stati pricipale (vedi myStatus Status[NUM_MODE])
//
// Le fasi della modalità set sono gestite con un'altra macchina a stati (vedi myStatus SetClockSubStatus[NUM_SETCLOCK_SUBSTATUS])
//
// ================================================================================
// Gestione pulsanti
//
// sono previsti 4 pulsanti:
//
// in modalità STANDARD (visualizzazione orario):
//
//      premere il pulsante MODE per almeno MIN_MS_TO_SET_MODE ms e si entra
//      in modalità SET_CLOCK
//
//      permere il pulsante SET e se entra in LED_TEST
//
//
// in modalità SET_CLOCK (impostazione orologio):
//
//      è attiva per prima l'impostazione dell'ora
//      impostazione ora:
//          le ore lampeggiano
//          premere + o - per aumentare  diminuire il valore dell'ora
//          quando si preme SET si memorizza l'ora visualizzata e
//          si passa all'impostazione dei minuti
//      impostazione minuti:
//          i minuti lampeggiano
//          premere + o - per aumentare  diminuire il valore dei minuti
//          quando si preme SET si memorizzano i minuti visualizzati,
//              si imposta l'orologio, si esce da SET_CLOCK e
//              si ritorna in modalità STANDARD
//
//      se si preme il pulsante MODE si esce da SET_CLOCK e
//          si ritorna in modalità STANDARD
//
//      se passano più di MAX_MS_X_SET_CLOCK senza premere uno dei pulsanti
//      +, -, SET si esce da SET_CLOCK e si ritorna in modalità STANDARD
//
// in modalità LED_TEST (test dei led):
//
//      se si preme un pulsante qualsiasi o comunque dopo MAX_MS_X_LED_TEST
//      si esce da SET_CLOCK e si ritorna in modalità STANDARD
//
// ================================================================================
// HW utilizzato:
//      1 x Arduino Mega 2560
//      4 x uln2803 (Darlington transistor array - pilotaggio dei led)
//      1 x Modulo RTC DS3231 (Real Time Clock)
//      1 x Modulo HC-06 (Bluetooth)
//      1 x TIP122 Darlington (per la varaizione lunminosità dei led)
//      Stricia LED tagliata in segmenti opportuni per le varie parole
//
// Connessioni
// Arduino [pin]
//             .----------.                         .----------.
//            -|32K       |              [11] -->---|RX  HC-06 |
//            -|SQW RTC   |              [12] -->---|TX  Blue  |
//  [SCL] ->- -|SCL DS3231|              [GND] ->- -|GND tooth |
//  [SDA] ->- -|SDA       |              [5V] -->- -|VCC       |
//  [5V] -->- -|VCC       |                         '----------'
//  [GND] ->- -|GND       |
//             '----------'
//                                       ____ puls MODE
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [22] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//
//                                       ____ puls NEXT_HM
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [23] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//
//                                       ____ puls PREV_HM
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [24] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//
//                                       ____ puls SET
//                               .-------o  o--------[5V]
//                .---------.    |   .---------.
//  [25] --<------| 220 Ohm |----'---| 10K Ohm |-----[GND]
//                '---------'        '---------'
//                .---v---.
//  [26] -->-- 1B-|       |-1C -->-- LED_HALF ------------<|---(*)
//  [27] -->-- 2B-|   A   |-2C -->-- LED_TEN_MIN ---------<|---(*)
//  [28] -->-- 3B-|       |-3C -->-- LED_QUARTER ---------<|---(*)
//  [29] -->-- 4B-|       |-4C -->-- LED_TWENTY ----------<|---(*)
//  [30] -->-- 5B-|ULN2003|-5C -->-- LED_FIVE_MIN --------<|---(*)
//  [31] -->-- 6B-|       |-6C -->-- LED_MINUTES ---------<|---(*)
//  [32] -->-- 7B-|       |-7C -->-- LED_TO --------------<|---(*)
//  [33] -->-- 8B-|       |-8C -->-- LED_PAST ------------<|---(*)
//  [GND] ->- GND-|       |-COM-
//                '-------'
//                .---v---.
//  [34] -->-- 1B-|       |-1C -->-- LED_ONE -------------<|---(*)
//  [35] -->-- 2B-|   B   |-2C -->-- LED_THREE -----------<|---(*)
//  [36] -->-- 3B-|       |-3C -->-- LED_TWO -------------<|---(*)
//  [37] -->-- 4B-|       |-4C -->-- LED_FOUR ------------<|---(*)
//  [38] -->-- 5B-|ULN2003|-5C -->-- LED_FIVE ------------<|---(*)
//  [39] -->-- 6B-|       |-6C -->-- LED_SIX -------------<|---(*)
//  [40] -->-- 7B-|       |-7C -->-- LED_SEVEN -----------<|---(*)
//  [41] -->-- 8B-|       |-8C -->-- LED_EIGHT -----------<|---(*)
//  [GND] ->- GND-|       |-COM
//                '-------'
//                .---v---.
//  [42] -->-- 1B-|       |-1C -->-- LED_NINE ------------<|---(*)
//  [43] -->-- 2B-|   C   |-2C -->-- LED_TEN -------------<|---(*)
//  [44] -->-- 3B-|       |-3C -->-- LED_ELEVEN ----------<|---(*)
//  [45] -->-- 4B-|       |-4C -->-- LED_TWELVE ----------<|---(*)
//  [46] -->-- 5B-|ULN2003|-5C -->-- LED_OCLOCK ----------<|---(*)
//  [47] -->-- 6B-|       |-6C -->-- LED_DST -------------<|---(*)
//  [48] -->-- 7B-|       |-7C -->-- LED_AM --------------<|---(*)
//  [49] -->-- 8B-|       |-8C -->-- LED_DOT1 ------------<|---(*)
//  [GND] ->- GND-|       |-COM
//                '-------'
//                .---v---.
//  [50] -->-- 1B-|       |-1C -->-- LED_DOT2 ------------<|---(*)
//  [51] -->-- 2B-|       |-2C -->-- LED_DOT3 ------------<|---(*)
//  [52] -->-- 3B-|       |-3C -->-- LED_DOT4 ------------<|---(*)
//  [53] -->-- 4B-|       |-4C -->-- LED_PM --------------<|---(*)
//             5B-|ULN2003|-5C
//             6B-|       |-6C
//             7B-|       |-7C      .--------.            .--------.
//  [02]       8B-|       |-8C -->--| 1K Ohm |--------- b-|        | .--. |
//  [GND] ->- GND-|       |-COM     '--------'   +12V-- c-| TIP122 | |  | |
//                '-------'                        (*)- e-|        | '--' |
//                                                        '--------'------'
//
// ================================================================================

#include <stdio.h>          // for function sprintf
#include <DS3231.h>         // library for RTC
#include <Wire.h>           // to communicate with I2C
#include <SoftwareSerial.h> // to communicate with bluetooth

#define NUM_STATES 3
#include "StateMachine.h"

// indici dei led
enum E_LED {
    LED_DST,      //  0 26
    LED_AM,       //  1 27
    LED_PM,       //  2 28
    LED_HALF,     //  3 29
    LED_TEN_MIN,  //  4 30
    LED_QUARTER,  //  5 31
    LED_TWENTY,   //  6 32
    LED_FIVE_MIN, //  7 33
    LED_MINUTES,  //  8 34
    LED_TO,       //  9 35
    LED_PAST,     // 10 36
    LED_ONE,      // 11 37
    LED_TWO,      // 12 38
    LED_THREE,    // 13 39
    LED_FOUR,     // 14 40
    LED_FIVE,     // 15 41
    LED_SIX,      // 16 42
    LED_SEVEN,    // 17 43
    LED_EIGHT,    // 18 44
    LED_NINE,     // 19 45
    LED_TEN,      // 20 46
    LED_ELEVEN,   // 21 47
    LED_TWELVE,   // 22 48
    LED_OCLOCK,   // 23 49
    LED_DOT1,     // 24 50
    LED_DOT2,     // 25 51
    LED_DOT3,     // 26 52
    LED_DOT4,     // 27 53
    NUM_LED       // 28
};

// indici dei pulsanti
enum E_PULS {
    PULS_MODE,    // 0
    PULS_NEXT_HM, // 1
    PULS_PREV_HM, // 2
    PULS_SET,     // 3
    NUM_PULS      // 4
};


// struttura associa ad ogni LED
struct myLed {
    boolean       bAssociateToMinutes;    // è associato al LED "minutes"
    const char*   associateString;        // testo associato
    byte          outputAddress;          // indirizzo dell'output associato
    boolean       bOut;                   // valore della uscita attuale
    boolean       bOldOut;                // valore della uscita al giro precedente
    boolean       bBlinkOut;              // led che deve lampeggiare
};

// struttura associa ad ogni PULSANTE
struct myPuls {
    byte    inputAddress;         // indirizzo dell'input associato
    boolean bIn;                  // valore dell'ingresso al giro attuale
    boolean bOldIn;               // valore dell'ingresso al giro precedente
    boolean bRiseEdge;            // fronte di salita nel ciclo precedente e livello 1 nell'attuale
    boolean bFallEdge;            // fronte di discesa nel ciclo precedente e livello 0 nell'attuale
    boolean bTmpRiseEdge;         // fronte di salita nel ciclo attuale
    boolean bTmpFallEdge;         // fronte di discesa nel ciclo attuale
    int     msInLevel;            // numero di ms nel livello
};

// definizione e inizializzazione dei pulsanti
myPuls Puls[NUM_PULS] = {
//   add  in     oldIn  Rise   Fall   RisTmp FalTmp ms
    {22,  false, false, false, false, false, false, 0}, // MODE
    {23,  false, false, false, false, false, false, 0}, // NEXT_HM
    {24,  false, false, false, false, false, false, 0}, // PREV_HM
    {25,  false, false, false, false, false, false, 0}  // SET
};

// definizione e inizializzazione dei led
myLed Led[NUM_LED] = {
//   toMin    string     add   out   OldOut blink
    {false, " DST",       47, false, false, false},   //  0  LED_DST      
    {false, " AM",        48, false, false, false},   //  1  LED_AM       
    {false, " PM",        53, false, false, false},   //  2  LED_PM       
    {true,  " half",      26, false, false, false},   //  3  LED_HALF     
    {true,  " ten",       27, false, false, false},   //  4  LED_TEN_MIN  
    {true,  " quarter",   28, false, false, false},   //  5  LED_QUARTER  
    {true,  " twenty",    29, false, false, false},   //  6  LED_TWENTY   
    {true,  " five",      30, false, false, false},   //  7  LED_FIVE_MIN 
    {true,  " minutes",   31, false, false, false},   //  8  LED_MINUTES  
    {true,  " to",        32, false, false, false},   //  9  LED_TO       
    {true,  " past",      33, false, false, false},   // 10  LED_PAST     
    {false, " one",       34, false, false, false},   // 11  LED_ONE      
    {false, " two",       36, false, false, false},   // 12  LED_TWO      
    {false, " three",     35, false, false, false},   // 13  LED_THREE    
    {false, " four",      37, false, false, false},   // 14  LED_FOUR     
    {false, " five",      38, false, false, false},   // 15  LED_FIVE     
    {false, " six",       39, false, false, false},   // 16  LED_SIX      
    {false, " seven",     40, false, false, false},   // 17  LED_SEVEN    
    {false, " eight",     41, false, false, false},   // 18  LED_EIGHT    
    {false, " nine",      42, false, false, false},   // 19  LED_NINE     
    {false, " ten",       43, false, false, false},   // 20  LED_TEN      
    {false, " eleven",    44, false, false, false},   // 21  LED_ELEVEN   
    {false, " twelve",    45, false, false, false},   // 22  LED_TWELVE   
    {true,  " o'clock",   46, false, false, false},   // 23  LED_OCLOCK   
    {true,  " .",         49, false, false, false},   // 24  LED_DOT1     
    {true,  ".",          50, false, false, false},   // 25  LED_DOT2     
    {true,  ".",          51, false, false, false},   // 26  LED_DOT3     
    {true,  ".",          52, false, false, false},   // 27  LED_DOT4     
};

// --------------------------------------------------------------------
// modi di funzionamento
typedef enum E_MODE {
    STANDARD_MODE,  // visualizzazione orario
    SET_CLOCK_MODE, // set dell'orologio
    LED_TEST_MODE,  // test ciclico delle uscite
    NUM_MODE
} ENUM_MODE;

CStateMachine States;

// --------------------------------------------------------------------
// sottostati di SET_CLOCK_MODE
typedef enum E_SETCLOCK_SUBSTATUS {
    SET_HOUR,
    SET_MINUTES,
    SET_FINISH,
    NUM_SETCLOCK_SUBSTATUS
} ENUM_SETCLOCK_SUBSTATUS;

CStateMachine SetClockSubStates;

// --------------------------------------------------------------------
// definizione di costanti
#define MS_CYCLE 100
#define MIN_MS_TO_SET_MODE 3000
#define MS_X_REFRESH 1000
#define MS_X_BLINK 500
#define CICLE_REFRESH_X_WRITE_ON_SERIAL 5

typedef enum E_SERIAL_TYPE {
    SERIAL_LINE,    // hardware serial
    BLUETOOTH_LINE, // bluetooth serial
} ENUM_SERIAL_TYPE;

//canali i/o utilizzati per il colloquio Arduino <-> modulo Bluetooth
typedef enum E_BLUETOOTH_PIN {
    BT_RX_PIN = 11,
    BT_TX_PIN = 12
} ENUM_BLUETOOTH_PIN;

typedef enum E_SERIAL_CMD {
    SERIAL_CMD_1CHAR          = 1,  //  comando da 1 carattere (diverso da N) H help comandi seriale
                                    //  comando da 1 carattere N Next per passare al prossimo led test
    SERIAL_CMD_ENABLE_LEDTEST = 2,  //  comando da 2 caratteri ET (Enable Test) per abilitare il test delle uscite
                                    //  oppure qualunque coppia di caratteri per disabilitare
    SERIAL_CMD_ENABLE_SINGLE_LEDTEST = 3,  //  comando da 2 caratteri EST (Enable Single Test) per abilitare il test delle uscite
                                           // per passare alla successiva N
                                           // qualunque coppia di caratteri per disabilitare
    SERIAL_CMD_SET_CLOCK      = 14, //  comando da 14 caratteri (aa,mm,gg,hh,mm) per set clock
    SERIAL_CMD_LIGHT_SET      =  4, //  comando da 4 caratteri Lnnn (L000 - L255) per variare la luminosità dei led
    SERIAL_CMD_MAX_LEN        = 14
} ENUM_SERIAL_CMD;

DS3231 Clock;   // real time clock

SoftwareSerial bluetooth  = SoftwareSerial(BT_RX_PIN, BT_TX_PIN);   // seriale bluetooth

// struttura dati associata ad ogni minuto
struct myLightLedOnMinutes {
    byte    led;        // Led da accendere in funzione del valore dei minuti:
                        //  se vale 255 accende LED_TWENTY e LED_FIVE_MIN
                        //  se vale 254 accende solo i led puntini
    boolean ledMinutes; // in funzione del valore dei minuti abilita l'accensione Led "minutes"
    byte    numDots;    // numero dei punti da accendere in funzione del valore dei minuti
};

myLightLedOnMinutes lightOnMinutes [60] = {
//  led         ledMinutes      numDots
    {LED_OCLOCK,    false,      0}, //  0
    {LED_OCLOCK,    false,      1}, //  1
    {LED_OCLOCK,    false,      2}, //  2
    {LED_OCLOCK,    false,      3}, //  3
    {LED_OCLOCK,    false,      4}, //  4
    {LED_FIVE_MIN,  true,       0}, //  5
    {LED_FIVE_MIN,  true,       1}, //  6
    {LED_FIVE_MIN,  true,       2}, //  7
    {LED_FIVE_MIN,  true,       3}, //  8
    {LED_FIVE_MIN,  true,       4}, //  9
    {LED_TEN_MIN,   true,       0}, // 10
    {LED_TEN_MIN,   true,       1}, // 11
    {LED_TEN_MIN,   true,       2}, // 12
    {LED_TEN_MIN,   true,       3}, // 13
    {LED_TEN_MIN,   true,       4}, // 14
    {LED_QUARTER,   false,      0}, // 15
    {LED_QUARTER,   false,      1}, // 16
    {LED_QUARTER,   false,      2}, // 17
    {LED_QUARTER,   false,      3}, // 18
    {LED_QUARTER,   false,      4}, // 19
    {LED_TWENTY,    true,       0}, // 20
    {LED_TWENTY,    true,       1}, // 21
    {LED_TWENTY,    true,       2}, // 22
    {LED_TWENTY,    true,       3}, // 23
    {LED_TWENTY,    true,       4}, // 24
    {255,           true,       0}, // 25   accesi contemporaneamente LED_TWENTY e LED_FIVE_MIN
    {255,           true,       1}, // 26   accesi contemporaneamente LED_TWENTY e LED_FIVE_MIN
    {255,           true,       2}, // 27   accesi contemporaneamente LED_TWENTY e LED_FIVE_MIN
    {255,           true,       3}, // 28   accesi contemporaneamente LED_TWENTY e LED_FIVE_MIN
    {255,           true,       4}, // 29   accesi contemporaneamente LED_TWENTY e LED_FIVE_MIN
    {LED_HALF,      false,      0}, // 30
    {LED_HALF,      false,      1}, // 31
    {LED_HALF,      false,      2}, // 32
    {LED_HALF,      false,      3}, // 33
    {LED_HALF,      false,      4}, // 34
    {255,           true,       0}, // 35
    {LED_TWENTY,    true,       4}, // 36
    {LED_TWENTY,    true,       3}, // 37
    {LED_TWENTY,    true,       2}, // 38
    {LED_TWENTY,    true,       1}, // 39
    {LED_TWENTY,    true,       0}, // 40
    {LED_QUARTER,   true,       4}, // 41
    {LED_QUARTER,   true,       3}, // 42
    {LED_QUARTER,   true,       2}, // 43
    {LED_QUARTER,   true,       1}, // 44
    {LED_QUARTER,   false,      0}, // 45
    {LED_TEN_MIN,   false,      4}, // 46
    {LED_TEN_MIN,   false,      3}, // 47
    {LED_TEN_MIN,   false,      2}, // 48
    {LED_TEN_MIN,   false,      1}, // 49
    {LED_TEN_MIN,   true,       0}, // 50
    {LED_FIVE_MIN,  true,       4}, // 51
    {LED_FIVE_MIN,  true,       3}, // 52
    {LED_FIVE_MIN,  true,       2}, // 53
    {LED_FIVE_MIN,  true,       1}, // 54
    {LED_FIVE_MIN,  true,       0}, // 55
    {254,           true,       4}, // 56   accesi solo i led puntini
    {254,           true,       3}, // 57   accesi solo i led puntini
    {254,           true,       2}, // 58   accesi solo i led puntini
    {254,           true,       1}  // 59   accesi solo i led puntini
};

// valori letti da RTC e usati per set RTC
boolean Century = false;
boolean h12;
boolean PM;
byte    second;
byte    minute;
byte    hour;
byte    day;
byte    month;
byte    year;

boolean PMNoDst;
byte    secondNoDst;
byte    minuteNoDst;
byte    hourNoDst;
byte    dayNoDst;
byte    monthNoDst;
byte    yearNoDst;
// byte  temperature;   NON UTILIZZATO

boolean bLedBlink     = false;  // abilitazione al lampeggio dei LED (modalità SET_CLOCK_MODE)
boolean bLedOn        = true;   // led lampeggianti accesi (modalità SET_CLOCK_MODE)
boolean bToSet        = false;  // abilitazione al set dell'RTC in uscita dalla modalità SET_CLOCK_MODE
byte    ledInTest     = 255;    // numero del LED in test (modalità LED_TEST_MODE)
boolean bSingleStepLedTest = false; // abilita l'avanzamento su comando del LED in test (modalità LED_TEST_MODE)

static bool USB_SERIAL = true;
static bool BLUETOOTH_SERIAL = false;

bool bErrStates = false;    // controllo sulla corretta inizializzazine delle macchine a stati

int mode;                   // modalità attiva
int oldMode;                // modalità attiva precedente

int setClockSubStatus;      // sotto stato set_clock
int oldsSetClockSubStatus;  // sotto stato set_clock set precedente

boolean bChangeToSTANDARD_MODE = false;     // cambio mode da seriale
boolean bChangeToLED_TEST_MODE = false;     // cambio mode da seriale

byte    setHours = 0;          // ora da impostare in modalità SET_CLOCK_MODE
byte    setMin   = 0;          // minuti da impostare in modalità SET_CLOCK_MODE

int cntRefreshXWriteOnSerial = 0;   // contatore ms per calcolo ciclo di scrittura su seriale
int cntMsRefresh = 0;               // contatore ms per calcolo ciclo di refresh
int msBlink      = 0;               // millisecondi in blink

boolean bRefreshCycle  = false; // ciclo di refresh attivo: lettura di RTC, ricalcolo output, test LED
boolean bWriteOnSerial = false; // cliclo di scrittura valori RTC e WordClock su seriale attivo

byte day4Month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

 enum E_CHECKDATATIME_CODE {
    CHECKDATATIME_OK,         // 0
    CHECKDATATIME_WRONG_YY,   // 1
    CHECKDATATIME_WRONG_MM,   // 2
    CHECKDATATIME_WRONG_DD,   // 3
    CHECKDATATIME_WRONG_hh,   // 4
    CHECKDATATIME_WRONG_mm,   // 5
    CHECKDATATIME_WRONG_ss,   // 6
    NUM_CHECKDATATIME_CODE
};

struct a {
    const char* s;
};
a checkDateDimeString[NUM_CHECKDATATIME_CODE] = {
    {"OK"},
    {"wrong years value. Must be in 17 .. 31 range"},
    {"wrong months value"},
    {"wrong days value"},
    {"wrong hours value"},
    {"wrong minutes value"},
    {"wrong seconds value"},
};

// ***************************************************************************************************
// Gestione PWM per variare la luminosità dei led
//
#define OUT_PWM 2
byte PWM_dutyCycle;
byte old_PWM_dutyCycle;

// ***************************************************************************************************
// GESTIONE ORA LEGALE
/*
  nel 2010 l'Italia con l'art. 22 della legge 96, recependo la direttiva 2000/84/CE del Parlamento europeo
  (direttiva a firma del presidente N. FONTAINE),
  fissò:
    l'inizio dell'ora legale alle ore 2:00 del mattino dell'ultima domenica di marzo e
    il termine alle 3:00 del mattino dell'ultima domenica di ottobre
    NOTARE che le 3:00 sono in oraa legale quaindi riferendosi all'ora solare il termine è alle ore 2:00
*/
#define HOURSTARTDST 2
#define HOURSTOPDST 2

byte monthStartDST = 3; // marzo    mese di inizio
byte monthStopDST = 10; // ottobre  mese di fine

struct myDayDST {
    byte start; // giorno di inizio dell'ora legale in funzione dell'anno alle ore 02:00 mettere l’orologio un’ora avanti
    byte stop;  // giorno di fine dell'ora legale in funzione dell'anno alle ore 03:00 mettere l’orologio un’ora indietro
};

myDayDST dayDST[] = {
//  start stop
    {25,   28},   // 2018
    {31,   27},   // 2019
    {29,   25},   // 2020
    {28,   31},   // 2021
    {27,   30},   // 2022
    {26,   29},   // 2023
    {31,   27},   // 2024
    {30,   26},   // 2025
    {29,   25},   // 2026
    {28,   31},   // 2027
    {26,   29},   // 2028
    {25,   28},   // 2029
    {31,   27},   // 2030
    {30,   26}    // 2031
};

// ###################################################################################################
//
//      CODICE
//
// ###################################################################################################

/* ***************************************************************************************
  lettura degli input
  aggiornamento dei fronti di salita e di discesa e del tempo di permanenza
  nel livello dei segnali
 * **************************************************************************************/
void ReadInput ()
{
    register myPuls* pPuls = &Puls[0];
    boolean bprint = false;
    for (byte item = 0; item < NUM_PULS; item ++, pPuls++)
    {
        pPuls->bOldIn = pPuls->bIn;                         // valore al giro precedente
        pPuls->bIn    = digitalRead(pPuls->inputAddress);   // lettura input

        // se ho un fronte lo memorizzo, se al giro dopo ho il livello congruente con il fronte
        // rilevato dico di avere avuto un fronte, altrimenti nessun fronte
        if (pPuls->bIn != pPuls->bOldIn)
        { // calcolo fronti di salita e discesa temporanei
            bprint = true;
            pPuls->msInLevel = 0;
            pPuls->bRiseEdge = false;
            pPuls->bFallEdge = false;
            pPuls->bTmpRiseEdge = pPuls->bIn;
            pPuls->bTmpFallEdge = !pPuls->bIn;
        }
        else
        { // calcolo permanenza nel livello e fronti e livelli filtrati
            if (pPuls->bTmpRiseEdge)
            {
                bprint = true;
                pPuls->bRiseEdge = pPuls->bIn;
                pPuls->bTmpRiseEdge = false;
                pPuls->bTmpFallEdge = false;
            }
            else if (pPuls->bTmpFallEdge)
            {
                bprint = true;
                pPuls->bFallEdge = !pPuls->bIn;
                pPuls->bTmpRiseEdge = false;
                pPuls->bTmpFallEdge = false;
            }
            else
            {
                pPuls->msInLevel += MS_CYCLE;
                pPuls->bTmpRiseEdge = false;
                pPuls->bTmpFallEdge = false;
                pPuls->bRiseEdge = false;
                pPuls->bFallEdge = false;
            }
        }
        if (bprint)
        {
            char txt[64];
            sprintf(txt, "input:%2d value:%1d (old:%1d) RiseEdge:%1d FallEdge:%1d",
            pPuls->inputAddress,
            pPuls->bIn,
            pPuls->bOldIn,
            pPuls->bRiseEdge,
            pPuls->bFallEdge);
            Serial.println (txt);
            bprint = false;
        }
    }
}

/* ***************************************************************************************
  Scrittura degli output
  Sono scritti solo se diversi dal giro precedente
 * **************************************************************************************/
void WriteOutput ()
{
    register myLed* pLed = &Led[0];

    if (PWM_dutyCycle != old_PWM_dutyCycle)
    {
        analogWrite(OUT_PWM, PWM_dutyCycle);
        old_PWM_dutyCycle = PWM_dutyCycle;
    }

    for (byte item = 0; item < NUM_LED; item ++, pLed++)
    {
        if (pLed->bOut != pLed->bOldOut)
        {
            digitalWrite(pLed->outputAddress, pLed->bOut ? HIGH : LOW);
            pLed->bOldOut = pLed->bOut;
        }
    }

    // scrittura su seriale dell'ora
    if (bWriteOnSerial)
    {
        char txt [64];
        Serial.print(States.GetStatusName());
        Serial.print(" - ");
        if (mode != LED_TEST_MODE)
        {
            Serial.print("It is");
        }
        pLed = &Led[0];
        for (byte item = 0; item < NUM_LED; item ++, pLed++)
        {
            if (pLed->bOut)
            {
                if (mode == LED_TEST_MODE)
                {
                    sprintf (txt, "led#:%d out addr:%d -> ", item, pLed->outputAddress);
                    Serial.print (txt);
                }
                Serial.print(pLed->associateString);
                if(bSingleStepLedTest)
                {
                    Serial.print(" - type N to test next LED");
                }
            }
        }
        Serial.print("\n");
        sprintf (txt, "*#WordClock - %s *", States.GetStatusName());
        bluetooth.print(txt);
    }
}

/* ***************************************************************************************
  Calcolo ora legale
 * **************************************************************************************/
bool NowInDST()
{
    if ((monthStartDST < monthNoDst) && (monthNoDst < monthStopDST))
    {
        return true;
    }
    byte ind = yearNoDst - 18;
    if (monthStartDST == monthNoDst)
    {
        if(dayNoDst > dayDST[ind].start)
        {
            return true;
        }
        if(dayNoDst < dayDST[ind].start)
        {
            return false;
        }
        byte hour24 = (PM==0)? hourNoDst : hourNoDst +12;
        if (hour24 < HOURSTARTDST)
        {
            return false;
        }
        return true;
    }

    if (monthStopDST == monthNoDst)
    {
        if(dayNoDst > dayDST[ind].stop)
        {
            return false;
        }
        if(dayNoDst < dayDST[ind].stop)
        {
            return true;
        }
        byte hour24 = (PM==0)? hourNoDst : hourNoDst +12;
        if (hour24 < HOURSTOPDST)
        {
            return true;
        }
        if (hour24 >= HOURSTOPDST)
        {
            return false;
        }
        return false;
    }
    return false;
}

/* =======================================================================================
   =======================================================================================
    GESTIONE Real Time Clock
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
  Set del Real Time Clock
 * **************************************************************************************/
void SetDS3231()
{
    if (NowInDST())
    {
        hourNoDst = hour - 1;
        if (hourNoDst == 255)
        {
            hourNoDst = 23;
        }
    }
    else
    {
        hourNoDst = hour;
    }
    Clock.setSecond(00);        //Set the second
    Clock.setMinute(minute);    //Set the minute
    Clock.setHour(hourNoDst);   //Set the hour
//  Clock.setDoW(1);            //Set the day of the week
    Clock.setDate(day);         //Set the date of the month
    Clock.setMonth(month);      //Set the month of the year
    Clock.setYear(year);        //Set the year (Last two digits of the year)
    Clock.setClockMode(true);
}

/* ***************************************************************************************
    Legge i valori da RTC e
    li scrive su seriale se il modo è STANDARD_MODE o SET_CLOCK_MOSE e se
    il ciclo è abilitato alla scrittura su seriale
* **************************************************************************************/
void ReadDS3231()
{
    secondNoDst = Clock.getSecond();
    minuteNoDst = Clock.getMinute();
    hourNoDst   = Clock.getHour(h12, PM);
    dayNoDst    = Clock.getDate();
    monthNoDst  = Clock.getMonth(Century);
    yearNoDst   = Clock.getYear();
//  temperature = Clock.getTemperature();
    second = secondNoDst;
    minute = minuteNoDst;
    day    = dayNoDst;
    month  = monthNoDst;
    year   = yearNoDst;
    if (NowInDST())
    {
        hour = hourNoDst + 1;
        if (hour >= 24)
        {
            hour = 1;
        }
    }
    else
    {
        hour = hourNoDst;
    }

    if (mode == LED_TEST_MODE)
    {
        return;
    }
    if (bWriteOnSerial)
    {
        ShowDateTimeOnSerial();
    }
}

/* ***************************************************************************************
    Scrive su seriale i valori letti da RTC
 * **************************************************************************************/
void ShowDateTimeOnSerial()
{
    char txt[64];
    sprintf(txt, "From RTC %2d-%2d-%2d %2d:%2d:%2d %s DST:%s transalte: ",
                    yearNoDst,
                    monthNoDst,
                    dayNoDst,
                    hourNoDst,
                    minuteNoDst,
                    secondNoDst,
                    (PM==1)?"PM":"AM",
                    NowInDST()? "YES":"NO");
    Serial.print (txt);
}

/* =======================================================================================
   =======================================================================================
    MACCHINA A STATI PRINCIPALE
   =======================================================================================
   ======================================================================================= */
 
/* =======================================================================================
   =======================================================================================
   Gestione di STANDARD MODE
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
    Gestione dello stato STANDARD_MODE
    Ad ogni ciclo di refresch aggiorna i valori di ore e minuti leggendo l'RTC
    Calcola i led da accendere in funzione del valore di ore e minuti
 * ***************************************************************************************/
void StandardModeStatus(void* pStructData)
{
    if (bRefreshCycle)
    {
        ReadDS3231();
    }
    CalcWordClock(minute, hour);
}

/* ***************************************************************************************
    Gestione ingresso nello stato STANDARD_MODE
 * **************************************************************************************/
void StandardModePickUp(void* pStructData)
{
    bChangeToSTANDARD_MODE = false;
}

/* ***************************************************************************************
    Gestione cambio stato STANDARD_MODE:
    premendo il pulsante Mode per almeno MIN_MS_TO_SET_MODE ms si entra
    in modalità SET_CLOCK
    permendo il pulsante SET o ricevendo il comando "ET" da seriale si entra in LED_TEST
 * ***************************************************************************************/
int StandardModeChangeStatus(void* pStructData)
{
    if ((Puls[PULS_MODE].bRiseEdge || Puls[PULS_MODE].bIn))
    {
        if (Puls[PULS_MODE].msInLevel >= MIN_MS_TO_SET_MODE)    // il pulsante deve essere premuto almeno MIN_MS_TO_SET_MODE
        {
            return SET_CLOCK_MODE;
        }
    }
    else if ((Puls[PULS_SET].bRiseEdge) ||
             bChangeToLED_TEST_MODE)
    {
        return LED_TEST_MODE;
    }
    return STANDARD_MODE;
}

/* =======================================================================================
   =======================================================================================
   Gestione di SET_CLOCK MODE
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
  lampeggio dei led dei minuti o delle ore a seconda
  di cosa si sta settando
 * **************************************************************************************/
void BlinkSettingLed()
{
    register myLed* pLed = &Led[0];

    bLedOn = !bLedOn;
    msBlink = 0;
    if (!bLedOn)
    { // i led che lampeggiano devono essere spenti
        switch (setClockSubStatus)
        {
            case SET_HOUR:
                for (byte item = 0; item < NUM_LED; item++, pLed++)
                {
                    if (!pLed->bAssociateToMinutes)
                    {
                        pLed->bOut = false;
                    }
                }
                break;
            case SET_MINUTES:
                for (byte item = 0; item < NUM_LED; item++, pLed++)
                {
                    if (pLed->bAssociateToMinutes)
                    {
                        pLed->bOut = false;
                    }
                }
                break;
            default:
                break;
        }
    }
    else
    {
        for (byte item = 0; item < NUM_LED; item++, pLed++)
        {
            pLed->bOut = pLed->bBlinkOut;
        }
    }
}

/* ***************************************************************************************
    in funzione di quale pulsante è premuto incrementa (PULS_NEXT_HM) o
    decrementa (PULS_PREV_HM) il valore passato
    ad ogni pressione di pulsante di incremento o decremento
    azzera il contatore di permanenza nello stato SET_CLOCL_MODE
 * **************************************************************************************/
byte IncDecValue(byte value, byte maxVal)
{
    if (Puls[PULS_NEXT_HM].bRiseEdge)
    {
        value++;
        if (value >= maxVal)
        {
            value = 0;
        }
    }
    else if (Puls[PULS_PREV_HM].bRiseEdge)
    {
        value--;
        if (value == 255)
        {
            value = maxVal - 1;
        }
    }
    return value;
}

/* ***************************************************************************************
    Gestione stato SET_HOUR
    incrementa o decerementa il valore dell'ora
 * **************************************************************************************/
void SetHourStatus(void* pStructData)
{
    setHours = IncDecValue(setHours, 24);
}

/* ***************************************************************************************
    Gestione cambio stato SET_HOUR
    premendo il pulsate SET si passa ad impostare i minuti
 * **************************************************************************************/
int SetHourChangeStatus(void* pStructData)
{
    if (Puls[PULS_SET].bRiseEdge)
    {
        return SET_MINUTES;
    }
    return SET_HOUR;
}

/* ***************************************************************************************
    Gestione stato SET_MINUTES
    incrementa o decerementa il valore dei minuti
 * **************************************************************************************/
void SetMinutesStatus(void* pStructData)
{
    setMin = IncDecValue(setMin, 60);
}

/* ***************************************************************************************
    Gestione ingresso nello stato SET_MINUTES
    azzera il contatore di permanenza nello stato SET_CLOCL_MODE
 * **************************************************************************************/
void SetMinutesPickUp (void* pStructData)
{
}

/* ***************************************************************************************
    Gestione cambio stato SET_MINUTES
    premendo il pulsate SET si passa nello stato FINISH dove si imposta l'RTC
 * **************************************************************************************/
int SetMinutesChangeStatus(void* pStructData)
{
    if (Puls[PULS_SET].bRiseEdge)
    {
        return SET_FINISH;
    }
    return SET_MINUTES;
}

/* ***************************************************************************************
    Gestione ingresso nello stato SET_FINISH
 * **************************************************************************************/
void SetFinishPickUp(void* pStructData)
{
    bLedBlink = false;
    bToSet = true;
}

/* ***************************************************************************************
    Gestione cambio stato SET_FINISH
 * **************************************************************************************/
int SetFinishChangeStatus(void* pStructData)
{
    return SET_FINISH;
}

/* ***************************************************************************************
    Gestione dello stato SET_CLOCK_MODE
    abilita il lampeggio dei led che si stanno impostando
    gestisce il set di ore e minuti (macchina a stati)
    calcola i led da accendere in funzione dei valori di ore e minuti che si stanno impostando
 * **************************************************************************************/
void SetClockModeStatus(void* pStructData)
{
    register myLed* pLed = &Led[0];

    if (msBlink >= MS_X_BLINK)
    {
        BlinkSettingLed();
    }

    oldsSetClockSubStatus = SetClockSubStates.GetStatusInd();
    SetClockSubStates.Manage();
    setClockSubStatus = SetClockSubStates.GetStatusInd();

    CalcWordClock(setMin, setHours);

    for (byte item = 0; item < NUM_LED; item ++, pLed++)
    {
        pLed->bBlinkOut = pLed->bOut;
    }
}

/* ***************************************************************************************
    Gestione ingresso nello stato SET_CLOCK_MODE
 * **************************************************************************************/
void SetClockModePickUp (void* pStructData)
{
    register myLed* pLed = &Led[0];

    setClockSubStatus       = SET_HOUR;
    oldsSetClockSubStatus   = SET_HOUR;
    setMin      = minute;
    setHours    = (PM ? hour + 12 : hour);
    bLedBlink   = true;
    bLedOn      = true;
    bToSet      = false;
    for (byte item = 0; item < NUM_LED; item ++, pLed++)
    {
        pLed->bBlinkOut = pLed->bOut;
    }
}

/* ***************************************************************************************
    Gestione uscita dallo stato SET_CLOCK_MODE
    imposta l'RTC se deve farlo
    disabilita il lampeggio dei led
 * **************************************************************************************/
void SetClockModeDropOut (void* pStructData)
{
    if (bToSet)  // devo fare il set di RTC
    {
        bToSet = false;
        minute = setMin;
        hour   = setHours;
        SetDS3231();
    }
    bLedBlink = false;
}

/* ***************************************************************************************
    Gestione cambio stato SET_CLOCK_MODE
    si ritorna in STANDARD_MODE:
    - se si è concluso il set di ore e minuti (si esegue il set di RTC)
    - premendo il pulsnato MODE (non si esegue alcun set di RTC)
    - è superato il tempo massimo di permanenza nello stato (non si esegue alcun set di RTC)
 * **************************************************************************************/
int SetClockModeChangeStatus(void* pStructData)
{
    if ((setClockSubStatus == SET_FINISH) ||
        Puls[PULS_MODE].bRiseEdge // !!!!!!!!!!! ||
        )
    {
        return STANDARD_MODE;
    }
    return SET_CLOCK_MODE;
}

/* =======================================================================================
   =======================================================================================
   Gestione di LED_TEST_MODE
   =======================================================================================
   ======================================================================================= */
/* ***************************************************************************************
 * **************************************************************************************/
void NextLedInTest()
{
    if (ledInTest != 255)
    {
        Led[ledInTest].bOut = false;
    }

    ledInTest++;

    if (ledInTest >= NUM_LED)
    {
        ledInTest = 0;
    }

    Led[ledInTest].bOut = true;
}

/* ***************************************************************************************
   Gestione dello stato LED_TEST_MODE
   Accende, uno alla volta, tutti i led csambiando il led acceso ad ogni clclo di refresh
 * **************************************************************************************/
void LedTestModeStatus(void* pStructData)
{
    if (!bRefreshCycle)
    {
        return;
    }
    if (bSingleStepLedTest)
    {
        return;
    }
    NextLedInTest();
}

/* ***************************************************************************************
    Gestione ingresso nello stato LED_TEST_MODE
    spegne tutti i led
 * **************************************************************************************/
void LedTestModePickUp (void* pStructData)
{
    register myLed* pLed = &Led[0];

    bChangeToLED_TEST_MODE = false;
    for (byte item = 0; item < NUM_LED; item++, pLed++)
    {
        pLed->bOut = false;
    }
}

/* ***************************************************************************************
    Gestione cambio stato LED_TEST_MODE
    qualuque pulsante si prema o
    se è arrivato comando da seriale o
    se è passato il tempo massimo nello stato torna a STANDARD_MODE
 * **************************************************************************************/
int LedTestModeChangeStatus(void* pStructData)
{
    if (Puls[PULS_MODE].bRiseEdge       ||
        Puls[PULS_SET].bRiseEdge        ||
        Puls[PULS_NEXT_HM].bRiseEdge    ||
        Puls[PULS_PREV_HM].bRiseEdge    ||
        bChangeToSTANDARD_MODE // !!!!!!!!!!! ||
        )
    {
        return STANDARD_MODE;
    }
    return LED_TEST_MODE;
}

/* =======================================================================================
   =======================================================================================
   Gestione input da seriale
   =======================================================================================
   ======================================================================================= */

/* ***************************************************************************************
 * **************************************************************************************/
int CharAvailableOnSerial(bool bSerial)
{
    if (bSerial)
    {
        return Serial.available();
    }
    else
    {
        return bluetooth.available();
    }
}

/* ***************************************************************************************
 * **************************************************************************************/
char SerialRead(bool bSerial)
{
    if (bSerial)
    {
        return Serial.read();
    }
    else
    {
        return bluetooth.read();
    }
}

/* ***************************************************************************************
 * **************************************************************************************/
int SerialParseInt(bool bSerial)
{
    if (bSerial)
    {
        return Serial.parseInt();
    }
    else
    {
        return bluetooth.parseInt();
    }
}

/* ***************************************************************************************
 * **************************************************************************************/
size_t SerialPrintln(bool bSerial, const char* c)
{
    if (bSerial)
    {
        return Serial.println(c);
    }
    else
    {
        return bluetooth.println(c);
    }
}

/* ***************************************************************************************
 * **************************************************************************************/
size_t SerialPrintln(bool bSerial, int val, int format)
{
    if (bSerial)
    {
        return Serial.println(val, format);
    }
    else
    {
        return bluetooth.println(val, format);
    }
}

/* ***************************************************************************************
   controlla i valori di data ed ora passati, se ok return TRUE
 * **************************************************************************************/
byte CheckDateAndHourValue(byte yy, byte MM, byte dd, byte hh, byte mm, byte ss)
{
    if ((yy < 17) || (yy > 31))
    {
        return CHECKDATATIME_WRONG_YY;
    }
    if (MM > 12)
    {
        return CHECKDATATIME_WRONG_MM;
    }
    if (MM == 2)    // controllo su anno bisestile
    {
        if (dd > (((2000+yy)%4)==0) ? 29 : 28)
        {
            return CHECKDATATIME_WRONG_DD;
        }
    }
    else if (dd > day4Month [MM])
    {
        return CHECKDATATIME_WRONG_DD;
    }
    if (hh > 24)
    {
        return CHECKDATATIME_WRONG_hh;
    }
    if (mm > 60)
    {
        return CHECKDATATIME_WRONG_mm;
    }
    if (ss > 60)
    {
        return CHECKDATATIME_WRONG_ss;
    }
    return CHECKDATATIME_OK;
}

/* ***************************************************************************************
 * **************************************************************************************/
byte ParseToDateTime (bool bSerial)
{
    year    = SerialParseInt(bSerial);
    month   = SerialParseInt(bSerial);
    day     = SerialParseInt(bSerial);
    hour    = SerialParseInt(bSerial);
    minute  = SerialParseInt(bSerial);
    second  = SerialParseInt(bSerial);
    return CheckDateAndHourValue(year, month, day, hour, minute, second);
}

/* ***************************************************************************************
   Controlla se il carattere passato è corretto per il comando SERIAL_CMD_LIGHT_SET
   return true se ok, false altrimenti
 * **************************************************************************************/
bool IsCharSetLightLevel(char c)
{
    return ((c == 'L') || (c == 'l'));
}

/* ***************************************************************************************
 * **************************************************************************************/
bool ParseSetLightLevel(bool bSerial)
{
    char c1 = SerialRead(bSerial);
    if (IsCharSetLightLevel(c1))
    {
        PWM_dutyCycle = SerialParseInt(bSerial);
        return true;
    }
    return false;
}

/* ***************************************************************************************
   abilita o disabilita il LED TEST in funzine dei valori letti
       "ET" abilita led test mode (case insensitive)
       qualunque altra coppia di caratteri lo disabilite
 * **************************************************************************************/
void CheckLedTestEnable (char c1, char c2)
{
    if (((c1 == 'E') || (c1 == 'e')) &&
        ((c2 == 'T') || (c2 == 't')))
    {
        bChangeToLED_TEST_MODE = true;
        bSingleStepLedTest = false;
    }
    else
    {
        bChangeToSTANDARD_MODE = true;
        bSingleStepLedTest = false;
    }
}

/* ***************************************************************************************
   abilita o disabilita il LED TEST in funzine dei valori letti
       "EST" abilita led test mode (case insensitive)
 * **************************************************************************************/
void CheckSingleStepLedTestEnable (char c1, char c2, char c3)
{
    if (((c1 == 'E') || (c1 == 'e')) &&
        ((c2 == 'S') || (c2 == 's')) &&
        ((c2 == 'T') || (c2 == 't')))
    {
        bChangeToLED_TEST_MODE = true;
        bSingleStepLedTest = true;
    }
}

/* ***************************************************************************************
 * **************************************************************************************/
void ParseLedTestEnable(bool bSerial)
{
    char c1 = SerialRead(bSerial);
    char c2 = SerialRead(bSerial);
    CheckLedTestEnable (c1, c2);
}

/* ***************************************************************************************
 * **************************************************************************************/
void ParseSingleCharCommand(bool bSerial)
{
    char c1 = SerialRead(bSerial);
    if ((c1 == 'N') || (c1 == 'n'))
    {
        NextLedInTest();
    }
    else
    {
        (bSerial) ? SerialHelp() : BluetoothHelp();
    }
}

/* ***************************************************************************************
 * **************************************************************************************/
void ParseSingleStepLedTestEnable(bool bSerial)
{
    char c1 = SerialRead(bSerial);
    char c2 = SerialRead(bSerial);
    char c3 = SerialRead(bSerial);
    CheckSingleStepLedTestEnable (c1, c2, c3);
}

/* ***************************************************************************************
 * **************************************************************************************/
void SerialHelp()
{
    Serial.read();
    Serial.println("*@Available command: into []");
    Serial.println("*@[any 1 char but not 'N'] this help");
    Serial.println("*@[ET] to Enable Led Test");
    Serial.println("*@     any other couple of char disable Led Test");
    Serial.println("*@[EST] to Enable Single Step Led Test");
    Serial.println("*@[N]   Next Single Step Led Test");
    Serial.println("*@      any other couple of char disable Single Led Test");
    Serial.println("*@[AA,MM,DD,hh,mm] to clock set");
    Serial.println("*@[Lnnn] to change Led light intensity");
    Serial.println("*@       nnn value from 000 (off) to 255 (full light)");
}

/* ***************************************************************************************
   Bluetoot HELP
 * **************************************************************************************/
void BluetoothHelp()
{
    //char c1 = bluetooth.read();
    bluetooth.read();
    bluetooth.println("*@Available command: into []");
    bluetooth.println("*@[any 1 char but not 'N'] this help");
    bluetooth.println("*@[ET] to Enable Led Test");
    bluetooth.println("*@     any other couple of char disable Led Test");
    bluetooth.println("*@[EST] to Enable Single Step Led Test");
    bluetooth.println("*@[N]   Next Single Step Led Test");
    bluetooth.println("*@      any other couple of char disable Single Led Test");
    bluetooth.println("*@[AA,MM,DD,hh,mm] to clock set");
    bluetooth.println("*@[Lnnn] to change Led light intensity");
    bluetooth.println("*@       nnn value from 000 (off) to 255 (full light)");
}

void ManageAllSerialCommand(bool bSerial)
{
    static byte serialNumLoop = 0;
    static byte bluetoothNumLoop = 0;
    static byte maxLoop = 5;
    byte numLoop = (bSerial) ? serialNumLoop : bluetoothNumLoop;
    char txt[64];

    switch (CharAvailableOnSerial(bSerial))
    {
        int nc;
        nc = CharAvailableOnSerial(bSerial);
        if ((nc > SERIAL_CMD_MAX_LEN) || (numLoop > maxLoop))
        {
            while (CharAvailableOnSerial(bSerial))
            {
                 SerialRead(bSerial);
            }
            if (bSerial) sprintf(txt, "flushing %d chararacters", nc);
            else sprintf(txt, "*@flushing %d chararacters", nc);
            SerialPrintln(bSerial, txt);
        }
        case SERIAL_CMD_SET_CLOCK: // acquisisce la stringa con data ed ora
                numLoop = 0;
                byte retCode;
                retCode = ParseToDateTime(bSerial);
                if (retCode == CHECKDATATIME_OK)
                {
                    SetDS3231();
                    if (bSerial) sprintf(txt, "--- Well done, clock set! :)");
                    else sprintf(txt, "*@--- Well done, clock set! :)");
                    SerialPrintln(bSerial, txt);
                }
                else
                {
                    if (bSerial)
                    {
                        Serial.println("*** ERROR parsing command!***");
                        Serial.println(checkDateDimeString[retCode].s);
                        Serial.println("check the command string");
                        Serial.println("must be in YY;MM;DD;hh:mm format");
                        Serial.println("and have valid values");
}
                    else
                    {
                        bluetooth.println("*@*** ERROR parsing command!***");
                        sprintf(txt, "*@%s",checkDateDimeString[retCode].s);
                        bluetooth.println(txt);
                        bluetooth.println("*@check the command string");
                        bluetooth.println("*@must be in YY;MM;DD;hh:mm format");
                        bluetooth.println("*@and have valid values");
                    }
                }
                break;
        case SERIAL_CMD_1CHAR:
                Serial.println("SERIAL_CMD_HELP");
                delay(100);
                numLoop = 0;
                ParseSingleCharCommand(bSerial);
                break;
        case SERIAL_CMD_ENABLE_LEDTEST: // controlla se e' in arrivo una stringa da 2 caratteri ET (Enable Test) oppure qualunque coppia di caratteri per disabilitare
                Serial.println("SERIAL_CMD_ENABLE_LEDTEST");
                delay(100);
                numLoop = 0;
                ParseLedTestEnable(bSerial);
                break;
        case SERIAL_CMD_LIGHT_SET:
                Serial.println("SERIAL_CMD_LIGHT_SET");
                delay(100);
                if (!ParseSetLightLevel(bSerial))
                {
                    if (bSerial) Serial.println("*** ERROR parsing command!***");
                    else bluetooth.println("*@*** ERROR parsing command!***");
                }
                else
                {
                    if (bSerial) Serial.println("--- Well done, duty cycle uptated! :)");
                    else bluetooth.println("*@--- Well done, duty cycle uptated! :)");
                }
                break;
        default:
                numLoop++;
                break;
    }
    (bSerial) ? (serialNumLoop = numLoop) : (bluetoothNumLoop = numLoop);
}

/* =======================================================================================
   =======================================================================================
    Gestione WORD CLOCK
   =======================================================================================
   ======================================================================================= */

/* ***************************************************************************************
    Se il ciclo attuale è un ciclo di refresh calcola i led da accendere
    in funzione dei valori di ore e minuti passati
    in STANDARD_MODE i valori delle ore arrivano a 12
    in SET_ClOCK_MODE i valori delle ore arrivano a 24
 * **************************************************************************************/
void CalcWordClock(byte vminute, byte vhour)
{
    boolean bPM;

    if (!bRefreshCycle)
    {
        return;
    }

    register myLed* pLed = &Led[0];
    for (byte item = 0; item < NUM_LED; item++, pLed++)
    {
        pLed->bOut = false;
    }

    Led[LED_DST].bOut = NowInDST();

    // led da accendere in funzione del valore dei minuti attuali
    byte led = lightOnMinutes[vminute].led;
    if (led == 255)
    {
        Led[LED_TWENTY].bOut   = true;
        Led[LED_FIVE_MIN].bOut = true;
    }
    else if(led != 254) // solo led puntini
    {
        Led[led].bOut = true;
    }
    // accensione del led MINUTES in funzione del valore dei minuti attuali
    Led[LED_MINUTES].bOut = lightOnMinutes[vminute].ledMinutes;

    if ((mode == SET_CLOCK_MODE) && bWriteOnSerial)
    {
        char txt[64];
        sprintf(txt, "%2d:%2d -> ", vhour, vminute);
        Serial.print (txt);
    }

    bPM = (vhour > 12);
    if (bPM)
    {
        vhour = vhour - 12;
    }
    // controllo per accensione led PAST e TO
    if (vminute > 4)
    {
        if (vminute <= 34)
        {
            Led[LED_PAST].bOut = true;
        }
        else
        {
            Led[LED_TO].bOut = true;
            vhour++;
            if (vhour > 12)
            {
                vhour = 1;
            }
        }
    }

    // accensione del led delle ore
    Led[LED_ONE + vhour - 1].bOut = true;

    if (vminute == 0)
    {
        Led[LED_OCLOCK].bOut = true;
    }

    // accensione dei led puntini
    switch (lightOnMinutes[vminute].numDots)
    {
        case 0: break;
        case 1:
            Led[LED_DOT1].bOut = true;
            break;
        case 2:
            Led[LED_DOT1].bOut = true;
            Led[LED_DOT2].bOut = true;
            break;
        case 3:
            Led[LED_DOT1].bOut = true;
            Led[LED_DOT2].bOut = true;
            Led[LED_DOT3].bOut = true;
            break;
        case 4:
            Led[LED_DOT1].bOut = true;
            Led[LED_DOT2].bOut = true;
            Led[LED_DOT3].bOut = true;
            Led[LED_DOT4].bOut = true;
            break;
    }
    if (mode == SET_CLOCK_MODE)
    {
        Led[LED_AM].bOut = !bPM;
        Led[LED_PM].bOut = bPM;
    }
    else
    {
        Led[LED_AM].bOut = !PM;
        Led[LED_PM].bOut = PM;
    }
}

/* ***************************************************************************************
    Calcoli vari in funzione dei cicli:
    - bRefreshCycle:  se il ciclo attuale se è un ciclo di refresh
    - bWriteOnSerial: se il ciclo attuale è abilitato alla scrittura su seriale
    - msBlink: se attivo il blink calcola il tempo di blink
 * **************************************************************************************/
void ManageCycleCounters()
{
    // determina se nel ciclo attuale deve essere fatto un refresh
    cntMsRefresh += MS_CYCLE;
    if (cntMsRefresh >= MS_X_REFRESH)
    {
        bRefreshCycle = true;
        cntMsRefresh = 0;
    }

    // in caso di modalità LED_TEST_MODE il ciclo di scrittura corrisponde a quello di refresh
    if (mode == LED_TEST_MODE)
    {
        cntRefreshXWriteOnSerial = 0;
        bWriteOnSerial = bRefreshCycle;
    }
    else
    {
        // determina se nel ciclo attuale deve essere fatta la scrittura su seriale
        if (bRefreshCycle)
        {
            cntRefreshXWriteOnSerial ++;
        }
        if (cntRefreshXWriteOnSerial >= CICLE_REFRESH_X_WRITE_ON_SERIAL)
        {
            bWriteOnSerial = true;
            cntRefreshXWriteOnSerial = 0;
        }
    }

    // se attivo il blink calcola il tempo di blink
    if (bLedBlink)
    {
        msBlink += MS_CYCLE;
    }
}

/* =======================================================================================
   =======================================================================================
    INIZIALIZZAZIONI
   =======================================================================================
   ======================================================================================= */
void setup()
{
    // Start the I2C interface
    Wire.begin();

    // Start the serial interface
    Serial.begin(115200);

    bluetooth.begin(9600);

    States.AssignData(nullptr, MS_CYCLE);
    
    States.EnableLog(true);
    
    States.AssignState( STANDARD_MODE,
                        StandardModeStatus,
                        nullptr,
                        nullptr,
                        StandardModePickUp,
                        StandardModeChangeStatus,
                        0,
                        0,
                        "Standard");
                        
    States.AssignState( SET_CLOCK_MODE,
                        SetClockModeStatus,
                        SetClockModeDropOut,
                        nullptr,
                        SetClockModePickUp,
                        SetClockModeChangeStatus,
                        300000L,
                        STANDARD_MODE,
                        "SetClock");

    States.AssignState( LED_TEST_MODE,
                        LedTestModeStatus,
                        nullptr,
                        nullptr,
                        LedTestModePickUp,
                        LedTestModeChangeStatus,
                        600000L,
                        STANDARD_MODE,
                        "TestLed");

    SetClockSubStates.EnableLog(true);
                        
    SetClockSubStates.AssignData(nullptr, MS_CYCLE);

    SetClockSubStates.AssignState( SET_HOUR,
                        SetHourStatus,
                        nullptr,
                        nullptr,
                        nullptr,
                        SetHourChangeStatus,
                        0,
                        0,
                        "SET_HOUR");

    SetClockSubStates.AssignState( SET_MINUTES,
                        SetMinutesStatus,
                        nullptr,
                        nullptr,
                        SetMinutesPickUp,
                        SetMinutesChangeStatus,
                        0,
                        0,
                        "SET_MINUTES");

    SetClockSubStates.AssignState( SET_FINISH,
                        nullptr,
                        nullptr,
                        nullptr,
                        SetFinishPickUp,
                        SetFinishChangeStatus,
                        0,
                        0,
                        "SET_FINISH");

    byte    item;
    myLed*  pLed  = &Led[0];
    myPuls* pPuls = &Puls[0];

    // inizializzazione input
    for (item = 0; item < NUM_PULS; item++, pPuls++)
    {
        pinMode(Puls->inputAddress, INPUT);
    //  digitalWrite(Puls->inputAddress, HIGH); le resistenze si pull-down sono hw
    }

    // inizializzazione output
    for (item = 0; item < NUM_LED; item++, pLed++)
    {
        pLed->bOut = false;
        pLed->bOldOut = false;
        pinMode(pLed->outputAddress, OUTPUT);
    }

    // duty cycle per variare luminosità dei led
    pinMode(OUT_PWM, OUTPUT);
    PWM_dutyCycle = 255;
    old_PWM_dutyCycle = 255;

    mode    = STANDARD_MODE;
    oldMode = STANDARD_MODE;
    cntMsRefresh = 0;
    msBlink      = 0;
    bLedBlink    = false;
    bLedOn       = true;
    bToSet       = false;

    delay(100);
    Serial.println("Type: AA;MM,GG,hh,mm to set Clock");
    Serial.println("      ET   (Enable Test) to change mode to ciclic LED_TEST ");
    Serial.println("      EST  (Enable Single Test) to change mode to single step LED_TEST ");
    Serial.println("      Lnnn to change led light intensity");
    Serial.println("           nnn value from 000 (off) to 255 (full light)");
    //Serial.println("      SET to change mode to SET");
    Serial.print("\n");
    delay(500);
    
    int err1 = States.GetInitError();
    int err2 = SetClockSubStates.GetInitError();
    
    if ((err1 != NO_ERROR) || (err2 != NO_ERROR))
    {
//      char txt[64];
//      sprintf(txt,"States err:%d - %s   SetClockSubStates:%d - %s", err1, States.GetInitErrorString(), err2, SetClockSubStates.GetInitErrorString());
//      Serial.println(txt);
        Serial.printf("States err:%d - %s   SetClockSubStates:%d - %s", err1, States.GetInitErrorString(), err2, SetClockSubStates.GetInitErrorString());
        bErrStates = true;
    }

    States.EnableLog(false);
    SetClockSubStates.EnableLog(false);
}

/* =======================================================================================
   =======================================================================================
    MAIN LOOP
   =======================================================================================
   ======================================================================================= */
void loop()
{
    if (bErrStates)
    {
        Serial.prinln(" ERRORE NELLA DEFINIZIONE DEGLI STATI ----- MORTO");
        return;
    }

    ManageCycleCounters();

    ManageAllSerialCommand(USB_SERIAL);
    ManageAllSerialCommand(BLUETOOTH_SERIAL);

    ReadInput();

    oldMode = States.GetStatusInd();
    States.Manage();
    mode = States.GetStatusInd();

    WriteOutput();

    bRefreshCycle  = false;
    bWriteOnSerial = false;

    delay(MS_CYCLE);
}
