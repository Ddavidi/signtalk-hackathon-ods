/*
 * ============================================================
 *  SIGN TALK - ESP32-S3 (Receptor / Cérebro)
 *  Versão para Arduino IDE + Edge Impulse Arduino Library
 * ============================================================
 *
 *  Como usar:
 *    1. No Edge Impulse, exporte em Deployment -> Arduino library.
 *    2. Na Arduino IDE, instale o ZIP em:
 *       Sketch -> Include Library -> Add .ZIP Library...
 *    3. Troque o include abaixo para o nome correto da sua biblioteca,
 *       caso seja diferente de sign_talk_inferencing.h.
 *    4. Selecione a placa ESP32S3 Dev Module.
 *    5. Abra o Serial Monitor em 115200 baud.
 *
 *  Comandos pelo Serial Monitor:
 *    DEBUG   -> Modo visualização
 *    COLETA  -> Modo coleta CSV
 *    IA      -> Modo inferência com Edge Impulse
 *    STATUS  -> Ver status
 *    PING    -> Teste de comunicação
 *
 *  IMPORTANTE:
 *    A struct DadosLuva precisa ser idêntica à struct do transmissor ESP32-C3.
 *    Se no transmissor o campo id ainda for char, troque para int também.
 *
 *  Identificação da luva:
 *    0 -> Mão direita
 *    1 -> Mão esquerda
 * ============================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// ============================================================
// EDGE IMPULSE
// ============================================================
// Troque este include pelo nome correto gerado pelo Edge Impulse, se necessário.
// Exemplo: #include <luva_libras_inferencing.h>
#include <ArielInacio-project-1_inferencing.h>

// ============================================================
// IDENTIFICADORES DAS LUVAS
// ============================================================
#define ID_LUVA_DIREITA  'D'
#define ID_LUVA_ESQUERDA 'E'

#define TIMEOUT_CONEXAO 2000UL
#define NUM_EIXOS 5

// Confiança mínima para aceitar uma classificação
#define CONFIANCA_MINIMA 0.85f

// Quantas vezes a mesma letra precisa aparecer para ser registrada
#define REPETICOES_ESTAVEIS 3

// ============================================================
// ESTRUTURA DE DADOS - PRECISA SER IDÊNTICA AO TRANSMISSOR
// ============================================================
typedef struct DadosLuva {
  char      id;              // D = Direita, E = Esquerda

  uint16_t flex_polegar;
  uint16_t flex_indicador;
  uint16_t flex_medio;
  uint16_t flex_anelar;
  uint16_t flex_minimo;

  float    acel_x;
  float    acel_y;
  float    acel_z;

  float    giro_x;
  float    giro_y;
  float    giro_z;

  float    roll;
  float    pitch;
} DadosLuva;

// ============================================================
// MODOS DE OPERAÇÃO
// ============================================================
enum Modo {
  MODO_DEBUG,
  MODO_COLETA,
  MODO_INFERENCIA
};

Modo modoAtual = MODO_DEBUG;

// ============================================================
// VARIÁVEIS GLOBAIS
// ============================================================
DadosLuva luvaDireita;
DadosLuva luvaEsquerda;

volatile bool novoDadoDireita  = false;
volatile bool novoDadoEsquerda = false;

volatile unsigned long ultimoRecebidoDireita  = 0;
volatile unsigned long ultimoRecebidoEsquerda = 0;

// Buffer temporal do Edge Impulse.
// Seu modelo espera EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE features.
// Cada amostra da luva possui 13 eixos.
static float ei_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t ei_buffer_index = 0;

// Controle de estabilidade da classificação
static String ultimaLetra = "";
static int contadorEstavel = 0;

// Controle para mensagens periódicas
static unsigned long ultimoStatusDebug = 0;

// ============================================================
// FUNÇÕES AUXILIARES
// ============================================================
bool luvaConectada(unsigned long ultimoRecebido) {
  if (ultimoRecebido == 0) return false;
  return (millis() - ultimoRecebido) < TIMEOUT_CONEXAO;
}

const char* idParaTexto(int id) {
  if (id == ID_LUVA_DIREITA) return "D";
  if (id == ID_LUVA_ESQUERDA) return "E";
  return "?";
}

const char* modoParaTexto(Modo modo) {
  switch (modo) {
    case MODO_DEBUG:      return "DEBUG";
    case MODO_COLETA:     return "COLETA";
    case MODO_INFERENCIA: return "INFERENCIA";
    default:              return "DESCONHECIDO";
  }
}

void imprimirDadosLuva(const DadosLuva &luva) {
  Serial.print("[");
  Serial.print(idParaTexto(luva.id));
  Serial.print("] Flex: P=");
  Serial.print(luva.flex_polegar);
  Serial.print(" I=");
  Serial.print(luva.flex_indicador);
  Serial.print(" M=");
  Serial.print(luva.flex_medio);
  Serial.print(" A=");
  Serial.print(luva.flex_anelar);
  Serial.print(" m=");
  Serial.print(luva.flex_minimo);

  Serial.print(" | Acel: X=");
  Serial.print(luva.acel_x, 2);
  Serial.print(" Y=");
  Serial.print(luva.acel_y, 2);
  Serial.print(" Z=");
  Serial.print(luva.acel_z, 2);

  Serial.print(" | Giro: X=");
  Serial.print(luva.giro_x, 2);
  Serial.print(" Y=");
  Serial.print(luva.giro_y, 2);
  Serial.print(" Z=");
  Serial.print(luva.giro_z, 2);

  Serial.print(" | Roll=");
  Serial.print(luva.roll, 1);
  Serial.print(" Pitch=");
  Serial.println(luva.pitch, 1);
}

void enviarCSV(const DadosLuva &luva) {
  Serial.print("DATA,");
  Serial.print(luva.id);
  Serial.print(",");
  Serial.print(luva.flex_polegar);
  Serial.print(",");
  Serial.print(luva.flex_indicador);
  Serial.print(",");
  Serial.print(luva.flex_medio);
  Serial.print(",");
  Serial.print(luva.flex_anelar);
  Serial.print(",");
  Serial.print(luva.flex_minimo);
  Serial.print(",");
  Serial.print(luva.acel_x, 4);
  Serial.print(",");
  Serial.print(luva.acel_y, 4);
  Serial.print(",");
  Serial.print(luva.acel_z, 4);
  Serial.print(",");
  Serial.print(luva.giro_x, 4);
  Serial.print(",");
  Serial.print(luva.giro_y, 4);
  Serial.print(",");
  Serial.print(luva.giro_z, 4);
  Serial.print(",");
  Serial.print(luva.roll, 2);
  Serial.print(",");
  Serial.println(luva.pitch, 2);
}

void imprimirCabecalhoCSV() {
  Serial.println("HEADER,id,flex_polegar,flex_indicador,flex_medio,flex_anelar,flex_minimo,acel_x,acel_y,acel_z,giro_x,giro_y,giro_z,roll,pitch");
}

void resetarInferencia() {
  ei_buffer_index = 0;
  contadorEstavel = 0;
  ultimaLetra = "";

  for (size_t i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
    ei_buffer[i] = 0.0f;
  }
}

// ============================================================
// ESP-NOW - CALLBACK COMPATÍVEL COM ARDUINO-ESP32 2.x E 3.x
// ============================================================
void tratarDadosRecebidos(const uint8_t *dados, int tamanho) {
  if (tamanho != sizeof(DadosLuva)) {
    Serial.print("[ESP-NOW] Tamanho incorreto. Esperado: ");
    Serial.print(sizeof(DadosLuva));
    Serial.print(" | Recebido: ");
    Serial.println(tamanho);
    return;
  }

  DadosLuva temp;
  memcpy(&temp, dados, sizeof(DadosLuva));

  if (temp.id == ID_LUVA_DIREITA) {
    memcpy(&luvaDireita, dados, sizeof(DadosLuva));
    novoDadoDireita = true;
    ultimoRecebidoDireita = millis();
  }
  else if (temp.id == ID_LUVA_ESQUERDA) {
    memcpy(&luvaEsquerda, dados, sizeof(DadosLuva));
    novoDadoEsquerda = true;
    ultimoRecebidoEsquerda = millis();
  }
  else {
    Serial.print("[ESP-NOW] ID de luva desconhecido: ");
    Serial.println(temp.id);
  }
}

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
void onDataRecv(const esp_now_recv_info_t *info, const uint8_t *dados, int tamanho) {
  (void)info;
  tratarDadosRecebidos(dados, tamanho);
}
#else
void onDataRecv(const uint8_t *mac, const uint8_t *dados, int tamanho) {
  (void)mac;
  tratarDadosRecebidos(dados, tamanho);
}
#endif

// ============================================================
// EDGE IMPULSE - BUFFER TEMPORAL
// ============================================================
void adicionarAmostraAoBuffer(const DadosLuva &luva) {
  if (ei_buffer_index + NUM_EIXOS > EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
    ei_buffer_index = 0;
  }

  ei_buffer[ei_buffer_index++] = (float)luva.flex_polegar;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_indicador;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_medio;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_anelar;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_minimo;

}

bool bufferProntoParaInferencia() {
  return ei_buffer_index >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
}

void executarInferenciaBuffer(const DadosLuva &luva) {
  signal_t signal;

  int err = numpy::signal_from_buffer(
    ei_buffer,
    EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
    &signal
  );

  if (err != 0) {
    Serial.println("[IA] Erro ao criar signal para Edge Impulse");
    resetarInferencia();
    return;
  }

  ei_impulse_result_t result = { 0 };

  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  if (res != EI_IMPULSE_OK) {
    Serial.print("[IA] Erro ao executar classificador: ");
    Serial.println((int)res);
    resetarInferencia();
    return;
  }

  const char *melhorLetra = "";
  float maiorConfianca = 0.0f;

  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    float valor = result.classification[i].value;

    if (valor > maiorConfianca) {
      maiorConfianca = valor;
      melhorLetra = result.classification[i].label;
    }
  }

  Serial.print("[IA] Melhor: ");
  Serial.print(melhorLetra);
  Serial.print(" | confianca: ");
  Serial.println(maiorConfianca, 2);

  if (maiorConfianca < CONFIANCA_MINIMA) {
    contadorEstavel = 0;
    ultimaLetra = "";
    ei_buffer_index = 0;
    return;
  }

  if (ultimaLetra == melhorLetra) {
    contadorEstavel++;
  }
  else {
    ultimaLetra = melhorLetra;
    contadorEstavel = 1;
  }

  if (contadorEstavel >= REPETICOES_ESTAVEIS) {
    Serial.print("[LETRA] ");
    Serial.print(melhorLetra);
    Serial.print(" | confianca: ");
    Serial.print(maiorConfianca, 2);

    Serial.print(" | Flex: ");
    Serial.print("Polegar=");
    Serial.print(luva.flex_polegar);

    Serial.print(" Indicador=");
    Serial.print(luva.flex_indicador);

    Serial.print(" Medio=");
    Serial.print(luva.flex_medio);

    Serial.print(" Anelar=");
    Serial.print(luva.flex_anelar);

    Serial.print(" Minimo=");
    Serial.println(luva.flex_minimo);

    contadorEstavel = 0;
    ultimaLetra = "";

    delay(1000);
  }

  // Versão simples: usa janelas sem sobreposição.
  // Depois de classificar, zera para juntar uma nova janela.
  ei_buffer_index = 0;
}

void processarInferencia(const DadosLuva &luva) {
  adicionarAmostraAoBuffer(luva);

  if (bufferProntoParaInferencia()) {
    executarInferenciaBuffer(luva);
  }
}

// ============================================================
// SERIAL - COMANDOS
// ============================================================
void processarComandoSerial() {
  if (!Serial.available()) return;

  String comando = Serial.readStringUntil('\n');
  comando.trim();
  comando.toUpperCase();

  if (comando == "DEBUG") {
    modoAtual = MODO_DEBUG;
    Serial.println("[MODO] Debug ativado");
  }
  else if (comando == "COLETA") {
    modoAtual = MODO_COLETA;
    Serial.println("[MODO] Coleta CSV ativada");
    imprimirCabecalhoCSV();
  }
  else if (comando == "IA") {
    modoAtual = MODO_INFERENCIA;
    resetarInferencia();
    Serial.println("[MODO] Inferencia ativada");
  }
  else if (comando == "STATUS") {
    Serial.print("[STATUS] Modo: ");
    Serial.println(modoParaTexto(modoAtual));

    Serial.print("[STATUS] Luva Direita: ");
    Serial.println(luvaConectada(ultimoRecebidoDireita) ? "ONLINE" : "OFFLINE");

    Serial.print("[STATUS] Luva Esquerda: ");
    Serial.println(luvaConectada(ultimoRecebidoEsquerda) ? "ONLINE" : "OFFLINE");

    Serial.print("[STATUS] EI input frame size: ");
    Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);

    Serial.print("[STATUS] EI label count: ");
    Serial.println(EI_CLASSIFIER_LABEL_COUNT);
  }
  else if (comando == "PING") {
    Serial.println("PONG");
  }
  else {
    Serial.print("[SERIAL] Comando desconhecido: ");
    Serial.println(comando);
  }
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("========================================");
  Serial.println(" SIGN TALK - Cerebro ESP32-S3");
  Serial.println(" Arduino IDE + Edge Impulse");
  Serial.println("========================================");
  Serial.println("Comandos:");
  Serial.println(" DEBUG  - Modo visualizacao");
  Serial.println(" COLETA - Modo coleta CSV");
  Serial.println(" IA     - Modo inferencia");
  Serial.println(" STATUS - Ver status");
  Serial.println(" PING   - Teste");
  Serial.println("========================================");

  Serial.print("[IA] EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE: ");
  Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);

  Serial.print("[IA] EI_CLASSIFIER_LABEL_COUNT: ");
  Serial.println(EI_CLASSIFIER_LABEL_COUNT);

  Serial.print("[IA] Eixos por amostra: ");
  Serial.println(NUM_EIXOS);

  if (EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE % NUM_EIXOS == 0) {
    Serial.print("[IA] Amostras por janela: ");
    Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE / NUM_EIXOS);
  }
  else {
    Serial.println("[AVISO] O tamanho da entrada nao e multiplo de 13.");
    Serial.println("[AVISO] Verifique os eixos usados no Edge Impulse e no codigo.");
  }

  resetarInferencia();

  WiFi.mode(WIFI_STA);

  Serial.print("[WiFi] MAC deste ESP32-S3: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO ao inicializar");
    while (true) {
      delay(100);
    }
  }

  esp_err_t cb_result = esp_now_register_recv_cb(onDataRecv);
  if (cb_result != ESP_OK) {
    Serial.print("[ESP-NOW] ERRO ao registrar callback: ");
    Serial.println((int)cb_result);
    while (true) {
      delay(100);
    }
  }

  Serial.println("[ESP-NOW] Receptor iniciado");
  Serial.println("[SISTEMA] Aguardando dados das luvas...");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  processarComandoSerial();

  bool temDireita = false;
  bool temEsquerda = false;

  if (novoDadoDireita) {
    novoDadoDireita = false;
    temDireita = true;
  }

  if (novoDadoEsquerda) {
    novoDadoEsquerda = false;
    temEsquerda = true;
  }

  if (modoAtual == MODO_DEBUG) {
    if (temDireita && luvaConectada(ultimoRecebidoDireita)) {
      imprimirDadosLuva(luvaDireita);
    }

    if (temEsquerda && luvaConectada(ultimoRecebidoEsquerda)) {
      imprimirDadosLuva(luvaEsquerda);
    }

    if (millis() - ultimoStatusDebug > 3000UL) {
      ultimoStatusDebug = millis();

      bool dir = luvaConectada(ultimoRecebidoDireita);
      bool esq = luvaConectada(ultimoRecebidoEsquerda);

      if (!dir && !esq) {
        Serial.println("[AVISO] Nenhuma luva conectada...");
      }
    }
  }

  else if (modoAtual == MODO_COLETA) {
    if (temDireita && luvaConectada(ultimoRecebidoDireita)) {
      enviarCSV(luvaDireita);
    }

    if (temEsquerda && luvaConectada(ultimoRecebidoEsquerda)) {
      enviarCSV(luvaEsquerda);
    }
  }

  else if (modoAtual == MODO_INFERENCIA) {
    // Por enquanto a inferência usa a luva direita.
    // Se quiser usar a esquerda ou as duas mãos, o modelo precisa ter sido treinado para isso.
    if (temDireita && luvaConectada(ultimoRecebidoDireita)) {
      processarInferencia(luvaDireita);
    }
  }

  delay(10);
}
