/*
 * ============================================================
 *  TESTE - Leitura dos 5 Sensores Flex
 * ============================================================
 *  Lê os 5 sensores flex conectados ao ESP32-C3 SuperMini
 *  e exibe os valores brutos (0-4095) no Serial Monitor.
 *
 *  Pinos:
 *   - Polegar:   GPIO 0
 *   - Indicador: GPIO 1
 *   - Médio:     GPIO 2
 *   - Anelar:    GPIO 3
 *   - Mínimo:    GPIO 4
 * ============================================================
 */

#define PINO_POLEGAR   0
#define PINO_INDICADOR 1
#define PINO_MEDIO     2
#define PINO_ANELAR    3
#define PINO_MINIMO    4

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PINO_POLEGAR,   INPUT);
  pinMode(PINO_INDICADOR, INPUT);
  pinMode(PINO_MEDIO,     INPUT);
  pinMode(PINO_ANELAR,    INPUT);
  pinMode(PINO_MINIMO,    INPUT);

  Serial.println("========================================");
  Serial.println("  TESTE - Sensores Flex");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Polegar\t| Indicador\t| Medio\t| Anelar\t| Minimo");
  Serial.println("-------\t| ---------\t| -----\t| ------\t| ------");
}

void loop() {
  uint16_t polegar   = analogRead(PINO_POLEGAR);
  uint16_t indicador = analogRead(PINO_INDICADOR);
  uint16_t medio     = analogRead(PINO_MEDIO);
  uint16_t anelar    = analogRead(PINO_ANELAR);
  uint16_t minimo    = analogRead(PINO_MINIMO);

  Serial.print(polegar);   Serial.print("\t| ");
  Serial.print(indicador); Serial.print("\t\t| ");
  Serial.print(medio);     Serial.print("\t| ");
  Serial.print(anelar);    Serial.print("\t\t| ");
  Serial.println(minimo);

  delay(200);
}
