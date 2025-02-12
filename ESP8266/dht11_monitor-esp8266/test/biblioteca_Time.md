| Função | Descrição | Exemplo |
|--------|------------|---------|
| `hour()` | Retorna a hora atual (0-23) | `int h = hour(); // h = 22` |
| `minute()` | Retorna os minutos atuais (0-59) | `int m = minute(); // m = 36` |
| `second()` | Retorna os segundos atuais (0-59) | `int s = second(); // s = 0` |
| `day()` | Retorna o dia atual (1-31) | `int d = day(); // d = 11` |
| `weekday()` | Retorna o dia da semana (1-7), onde domingo é 1 | `int wd = weekday(); // wd = 3` |
| `month()` | Retorna o mês atual (1-12) | `int mo = month(); // mo = 2` |
| `year()` | Retorna o ano completo (2009, 2010, etc.) | `int y = year(); // y = 2025` |
| `hourFormat12()` | Retorna a hora atual no formato de 12 horas | `int h12 = hourFormat12(); // h12 = 10` |
| `isAM()` | Retorna verdadeiro se o horário atual é AM | `bool am = isAM(); // am = false` |
| `isPM()` | Retorna verdadeiro se o horário atual é PM | `bool pm = isPM(); // pm = true` |
| `now()` | Retorna o tempo atual como segundos desde 1º de janeiro de 1970 | `time_t t = now(); // t = 1613086560` |
| `setTime(time_t t)` | Define o tempo do sistema para o valor fornecido | `setTime(1613086560);` |
| `setTime(hr, min, sec, day, mnth, yr)` | Define o tempo do sistema com valores específicos (ano pode ser 2 ou 4 dígitos) | `setTime(22, 36, 0, 11, 2, 2025);` |
| `adjustTime(long adjustment)` | Ajusta o tempo do sistema adicionando o valor fornecido | `adjustTime(3600); // Adiciona uma hora` |
| `timeStatus()` | Indica o status do tempo (timeNotSet, timeNeedsSync, timeSet) | `timeStatus_t status = timeStatus();` |
| `breakTime(time_t time, tmElements_t &tm)` | Divide o tempo em elementos (horas, minutos, segundos, dia, mês, ano) | `breakTime(t, tm); // tm contém os elementos de tempo` |