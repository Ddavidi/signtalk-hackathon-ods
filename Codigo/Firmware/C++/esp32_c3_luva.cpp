/*
 * ============================================================
 *  SIGN TALK - ESP32-C3 (Transmissor / Luva)
 * ============================================================
 *  Lê os 5 sensores flex + MPU6050 e envia via ESP-NOW
 *  para o ESP32-S3 (receptor central).
 * 
 *  Hardware:
 *   - 5x Sensores Flex (ADC): pinos 0, 1, 2, 3, 4
 *   - 1x MPU6050 (I2C):       SDA=8, SCL=9
 * 
 *  IMPORTANTE: Antes de gravar, altere o MAC_RECEPTOR
 *  com o endereço MAC do seu ESP32-S3.
 * ============================================================
 */

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>

// ============================================================
//  CONFIGURAÇÕES - ALTERE AQUI
// ============================================================

// MAC Address do ESP32-S3 (receptor)
// Use o sketch "descobrir_mac.ino" para obter este endereço
// Exemplo: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
uint8_t MAC_RECEPTOR[] = {0x3C, 0xDC, 0x75, 0x5C, 0x79, 0x68};

// Identificador desta luva: 'E' = Esquerda, 'D' = Direita
// Altere conforme o braço onde esta luva será usada
#define ID_LUVA 'E'

// ============================================================
//  PINOS DOS SENSORES FLEX
// ============================================================
#define PINO_POLEGAR   0
#define PINO_INDICADOR 1
#define PINO_MEDIO     2
#define PINO_ANELAR    3
#define PINO_MINIMO    4

// ============================================================
//  ESTRUTURA DE DADOS PARA ENVIO
// ============================================================
// Esta struct DEVE ser idêntica no código do receptor (ESP32-S3)
typedef struct DadosLuva {
  char     id;             // 'E' ou 'D' (esquerda/direita)
  // Sensores Flex (valores brutos do ADC: 0-4095)
  uint16_t flex_polegar;
  uint16_t flex_indicador;
  uint16_t flex_medio;
  uint16_t flex_anelar;
  uint16_t flex_minimo;
  // Aceleração calibrada (m/s²)
  float    acel_x;
  float    acel_y;
  float    acel_z;
  // Giroscópio calibrado (rad/s)
  float    giro_x;
  float    giro_y;
  float    giro_z;
  // Ângulos calculados (graus)
  float    roll;           // inclinação lateral
  float    pitch;          // inclinação frente/trás
} DadosLuva;

// ============================================================
//  VARIÁVEIS GLOBAIS
// ============================================================
Adafruit_MPU6050 mpu;
DadosLuva dados;

// Offsets de calibração do MPU6050
float erroAcelX = 0, erroAcelY = 0, erroAcelZ = 0;
float erroGiroX = 0, erroGiroY = 0, erroGiroZ = 0;

// Controle de status do envio
bool envioOk = false;

// ============================================================
//  CALLBACK - Chamado após cada envio ESP-NOW
// ============================================================
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  envioOk = (status == ESP_NOW_SEND_SUCCESS);
  if (!envioOk) {
    Serial.println("[ESP-NOW] Falha no envio!");
  }
}

// ============================================================
//  CALIBRAÇÃO DO MPU6050
// ============================================================
void calibrarMPU() {
  Serial.println("[MPU6050] Calibrando... MANTENHA O SENSOR PARADO E PLANO!");
  delay(2000);

  sensors_event_t a, g, temp;
  const int AMOSTRAS = 100;

  erroAcelX = 0; erroAcelY = 0; erroAcelZ = 0;
  erroGiroX = 0; erroGiroY = 0; erroGiroZ = 0;

  for (int i = 0; i < AMOSTRAS; i++) {
    mpu.getEvent(&a, &g, &temp);
    erroAcelX += a.acceleration.x;
    erroAcelY += a.acceleration.y;
    erroAcelZ += a.acceleration.z - 9.81; // Z desconta a gravidade
    erroGiroX += g.gyro.x;
    erroGiroY += g.gyro.y;
    erroGiroZ += g.gyro.z;
    delay(10);
  }

  erroAcelX /= AMOSTRAS;
  erroAcelY /= AMOSTRAS;
  erroAcelZ /= AMOSTRAS;
  erroGiroX /= AMOSTRAS;
  erroGiroY /= AMOSTRAS;
  erroGiroZ /= AMOSTRAS;

  Serial.println("[MPU6050] Calibração concluída!");
  Serial.print("  Offsets Acel -> X: "); Serial.print(erroAcelX);
  Serial.print(" Y: "); Serial.print(erroAcelY);
  Serial.print(" Z: "); Serial.println(erroAcelZ);
  Serial.print("  Offsets Giro -> X: "); Serial.print(erroGiroX);
  Serial.print(" Y: "); Serial.print(erroGiroY);
  Serial.print(" Z: "); Serial.println(erroGiroZ);
}

// ============================================================
//  INICIALIZAÇÃO DO ESP-NOW
// ============================================================
void iniciarESPNOW() {
  // Coloca o Wi-Fi em modo Station (necessário para ESP-NOW)
  WiFi.mode(WIFI_STA);

  Serial.print("[WiFi] MAC deste ESP32-C3: ");
  Serial.println(WiFi.macAddress());

  // Inicializa ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO ao inicializar!");
    while (1) delay(10);
  }

  // Registra o callback de envio
  esp_now_register_send_cb(onDataSent);

  // Adiciona o ESP32-S3 como peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, MAC_RECEPTOR, 6);
  peerInfo.channel = 0;   // Canal automático
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO ao adicionar peer!");
    while (1) delay(10);
  }

  Serial.println("[ESP-NOW] Inicializado com sucesso!");
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  unsigned long tempoInicio = millis();
  // Espera no máximo 3000 milissegundos (3 segundos) pelo Serial
  while (!Serial && (millis() - tempoInicio < 3000)) {
    delay(10);
  }

  Serial.println("========================================");
  Serial.println("  SIGN TALK - Luva Transmissora");
  Serial.print("  ID da luva: ");
  Serial.println((char)ID_LUVA);
  Serial.println("========================================");

  // --- Inicializa I2C e MPU6050 ---
  Wire.begin(8, 9);

  if (!mpu.begin()) {
    Serial.println("[MPU6050] Falha ao conectar o módulo!");
    while (1) delay(10);
  }
  Serial.println("[MPU6050] Conectado!");

  // Configurações do MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // --- Pinos dos sensores flex ---
  pinMode(PINO_POLEGAR,   INPUT);
  pinMode(PINO_INDICADOR, INPUT);
  pinMode(PINO_MEDIO,     INPUT);
  pinMode(PINO_ANELAR,    INPUT);
  pinMode(PINO_MINIMO,    INPUT);

  // --- Calibração ---
  calibrarMPU();

  // --- Inicializa ESP-NOW ---
  iniciarESPNOW();

  // Define o ID fixo da luva
  dados.id = ID_LUVA;

  Serial.println("\n[SISTEMA] Pronto! Enviando dados...\n");
}

// ============================================================
//  LOOP PRINCIPAL
// ============================================================
void loop() {
  // --- 1. Ler sensores flex ---
  dados.flex_polegar   = analogRead(PINO_POLEGAR);
  dados.flex_indicador = analogRead(PINO_INDICADOR);
  dados.flex_medio     = analogRead(PINO_MEDIO);
  dados.flex_anelar    = analogRead(PINO_ANELAR);
  dados.flex_minimo    = analogRead(PINO_MINIMO);

  // --- 2. Ler e calibrar MPU6050 ---
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  dados.acel_x = a.acceleration.x - erroAcelX;
  dados.acel_y = a.acceleration.y - erroAcelY;
  dados.acel_z = a.acceleration.z - erroAcelZ;

  dados.giro_x = g.gyro.x - erroGiroX;
  dados.giro_y = g.gyro.y - erroGiroY;
  dados.giro_z = g.gyro.z - erroGiroZ;

  // --- 3. Calcular ângulos ---
  dados.roll  = atan2(dados.acel_y, dados.acel_z) * 180.0 / PI;
  dados.pitch = atan2(-dados.acel_x, sqrt(dados.acel_y * dados.acel_y + dados.acel_z * dados.acel_z)) * 180.0 / PI;

  // --- 4. Enviar via ESP-NOW ---
  esp_err_t resultado = esp_now_send(MAC_RECEPTOR, (uint8_t *)&dados, sizeof(DadosLuva));

  // --- 5. Debug no Serial Monitor (opcional, pode remover) ---
  Serial.print("[");
  Serial.print((char)dados.id);
  Serial.print("] Flex: ");
  Serial.print(dados.flex_polegar);   Serial.print(" | ");
  Serial.print(dados.flex_indicador); Serial.print(" | ");
  Serial.print(dados.flex_medio);     Serial.print(" | ");
  Serial.print(dados.flex_anelar);    Serial.print(" | ");
  Serial.print(dados.flex_minimo);
  Serial.print("  Roll: ");  Serial.print(dados.roll, 1);
  Serial.print("°  Pitch: "); Serial.print(dados.pitch, 1);
  Serial.print("°  Status: ");
  Serial.println(resultado == ESP_OK ? "OK" : "ERRO");

  // Envio a cada 50ms (~20 leituras por segundo)
  delay(50);
}
