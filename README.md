# WordClock

## Scopo
visualizzazione dell'ora mendiante scritte che vengono illuminate dall'accensione
di led sottostanti:
*                    .------------------------------.
                     |  IT IS       HALF       TEN  |
                     |                              |
                     |      QUARTER     TWENTY      |
                     |                              |
                     |  FIVE       MINUTES     TO   |
                     |                              |
                     |  PAST        ONE       TWO   |
                     |                              |
                     |  THREE      FOUR       FIVE  |
                     |                              |
                     |  SIX        SEVEN      EIGH  |
                     |                              |
                     |  NINE        TEN     ELEVEN  |
                     |                              |
                     |  TWELVE     O’CLOCK     DST  |
                     |                              |
                     |  AM   •    •    •    •   PM  |
                     '------------------------------'

Le scritte sono in lingua inglese, la visualizzazione ha una granularità
principale di cinque minuti (o'clock - five - ten ...)

Sono aggiunti del puntini (4 puntini) che portano la granularità al minuto
è indicato se AM o PM e se è attivo il DST ("Daylight Saving time" ossia ora legale)

esempi

* 5:00 it is "five" "o'clock"
* 5:27 it is "twenty" "five" "minutes" "past" "five" "." "." -> 5:25 + 2 puntini
* 5:41 it is "quarter" "to" "five" "."  "."  "." "."         -> 5:15 + 4 puntini

è prevista una gestione automatica dell'ora legale valida fino al 2031

## HW utilizzato:
  * 1 x Arduino Mega 2560
  * 4 x uln2803 (Darlington transistor array)
  * 1 x Modulo RTC DS3231 (Real Time Clock)
  * 1 x Modulo HC-06 (Bluetooth)
  * Stricia LED tagliata in segmenti opportuni per le varie parole
  * 1 x TIP122 (Power Darlington)
  * 4 x pulsanti
  * resistenze
  * alimentatore
