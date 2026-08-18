/*
 * ============================================================================
 *  SIGN TALK - CÉREBRO INTEGRADO (RECEPTOR ESP-NOW + IA EDGE IMPULSE + LCD + ÁUDIO MP3 OFFLINE)
 *  Placa: ESP32-S3 (16MB Flash / 8MB PSRAM)
 *  Pasta: Codigo Antigo/testes/teste_cerebro_lcd_mp3
 * ============================================================================
 *
 *  OBJETIVO:
 *    Unificar o código oficial de inferência (esp32_s3_cerebro.ino) com o
 *    sistema de Display LCD 16x2 e Áudio Humano MP3 Offline (teste_lcd_mp3_offline).
 *    
 *    Quando você faz um sinal com a luva (ex: Letra A, B, C, L, V, OLA...):
 *      1. O ESP32-S3 recebe os sensores via rádio ESP-NOW da luva.
 *      2. A Inteligência Artificial (Edge Impulse) classifica o movimento.
 *      3. Ao confirmar o sinal com estabilidade, o LCD exibe a letra/palavra
 *         e o alto-falante pronuncia a voz humana real gravada na memória Flash!
 *
 *  ============================================================================
 *  BIBLIOTECAS NECESSÁRIAS:
 *  ============================================================================
 *    1. LiquidCrystal I2C (por Frank de Brabander)
 *    2. ESP8266Audio      (por Earle F. Philhower) -> Para MP3 e I2S
 *    3. Biblioteca Edge Impulse do seu projeto (ex: ArielInacio-project-1_inferencing.h)
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include "Codigo/Firmware/Bibliotecas/audios_libras.h" // Arquivo com 41 áudios MP3 gravados na Flash

// ============================================================
// EDGE IMPULSE (Inteligência Artificial)
// ============================================================
// Troque pelo nome exato da sua biblioteca gerada no Edge Impulse se necessário
#include <ArielInacio-project-1_inferencing.h>

// ============================================================
// CONFIGURAÇÃO DOS PINOS - ESP32-S3
// ============================================================
// 1. Display LCD 16x2 I2C
#define LCD_ENDERECO 0x27
#define LCD_COLUNAS  16
#define LCD_LINHAS   2
#define LCD_SDA      8
#define LCD_SCL      9

// 2. Áudio I2S (MAX98357A)
#define I2S_BCLK       16
#define I2S_LRC        17
#define I2S_DIN        18
#define I2S_SD         15
#define VOLUME_INICIAL 0.85f

// 3. Parâmetros de Conexão e IA
#define ID_LUVA_DIREITA  'D'
#define ID_LUVA_ESQUERDA 'E'
#define TIMEOUT_CONEXAO  2000UL
#define NUM_EIXOS        5

// Confiança mínima para aceitar uma classificação do Edge Impulse
#define CONFIANCA_MINIMA 0.85f

// Quantas vezes seguidas a mesma letra precisa ser vista para ser falada (filtra ruídos)
#define REPETICOES_ESTAVEIS 3

// ============================================================
// ESTRUTURA DE DADOS DA LUVA (Idêntica ao Transmissor C3)
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

// Modos de Operação do Sistema
enum Modo {
  MODO_DEBUG,       // Exibe dados brutos no Serial
  MODO_COLETA,      // Envia CSV para treinar no Edge Impulse
  MODO_INFERENCIA   // IA ativada: reconhece sinais e fala no alto-falante
};

Modo modoAtual = MODO_INFERENCIA; // Começa em modo de IA e Voz por padrão!
float volumeAtual = VOLUME_INICIAL;

// Variáveis Globais de Dados
DadosLuva luvaDireita;
DadosLuva luvaEsquerda;
volatile bool novoDadoDireita  = false;
volatile bool novoDadoEsquerda = false;
volatile unsigned long ultimoRecebidoDireita  = 0;
volatile unsigned long ultimoRecebidoEsquerda = 0;

// Buffer temporal do Edge Impulse
static float ei_buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static size_t ei_buffer_index = 0;
static String ultimaLetraIA = "";
static int contadorEstavel = 0;

// Controle de anti-repetição de voz (Debounce)
static String ultimoSinalAnunciado = "";
static unsigned long ultimoAudioTocado = 0;
static unsigned long ultimoStatusDebug = 0;

LiquidCrystal_I2C lcd(LCD_ENDERECO, LCD_COLUNAS, LCD_LINHAS);

// Protótipos de funções
void inicializarLCD();
void exibirNoLCD(String linha1, String linha2);
String removerAcentosLCD(String texto);
void ligarAmplificador(AudioOutputI2S *out);
void desligarAmplificador(AudioOutputI2S *out);
void reproduzirAudioPROGMEM(const unsigned char *dados, unsigned int tamanho, String texto);
void anunciarSinalLibras(String sinal);
bool letraEstaFuncionando(String letra);
void processarInferencia(const DadosLuva &luva);
void adicionarAmostraAoBuffer(const DadosLuva &luva);
bool bufferProntoParaInferencia();
void executarInferenciaBuffer(const DadosLuva &luva);
void resetarInferencia();
void tratarDadosRecebidos(const uint8_t *dados, int tamanho);
void processarComandoSerial();
void imprimirDadosLuva(const DadosLuva &luva);
void enviarCSV(const DadosLuva &luva);
void imprimirCabecalhoCSV();
void executarTesteAutomatico();
void listarVocabulario();
void realizarScanI2C();
void imprimirAjuda();
bool luvaConectada(unsigned long ultimoRecebido);

// ============================================================
// FILTRO DAS LETRAS QUE ESTÃO FUNCIONANDO / EM TESTE
// ============================================================
// Coloque aqui apenas as letras que já estão funcionando/treinadas no seu modelo Edge Impulse!
// Se quiser testar qualquer letra que a IA identificar, deixe retornando true.
bool letraEstaFuncionando(String letra) {
  letra.trim();
  letra.toUpperCase();

  // Ignora labels de silêncio ou ruído de fundo da IA se existirem no modelo
  if (letra == "IDLE" || letra == "RUIDO" || letra == "SILENCIO" || letra == "_NOISE" || letra == "FUNDO" || letra == "NEUTRO") {
    return false;
  }

  // Lista de verificação das letras. Você pode comentar/descomentar conforme for aperfeiçoando o treino:
  if (letra == "A" || letra == "B" || letra == "C" || letra == "D" || letra == "E" ||
      letra == "F" || letra == "G" || letra == "H" || letra == "I" || letra == "J" ||
      letra == "K" || letra == "L" || letra == "M" || letra == "N" || letra == "O" ||
      letra == "P" || letra == "Q" || letra == "R" || letra == "S" || letra == "T" ||
      letra == "U" || letra == "V" || letra == "W" || letra == "X" || letra == "Y" ||
      letra == "Z" || letra == "OLA" || letra == "SIM" || letra == "NAO" || letra == "OBRIGADO" ||
      letra == "AGUA" || letra == "AJUDA" || letra == "SIGN TALK" || letra == "OLA MUNDO") {
    return true;
  }

  return false;
}

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("   SIGN TALK - CÉREBRO ESP32-S3 (IA + LCD + VOZ OFFLINE)");
  Serial.println("==========================================================");
  Serial.printf("Pinos LCD: SDA=%d | SCL=%d | Endereço=0x%02X\n", LCD_SDA, LCD_SCL, LCD_ENDERECO);
  Serial.printf("Pinos Áudio: BCLK=%d | LRC=%d | DIN=%d | SD=%d\n", I2S_BCLK, I2S_LRC, I2S_DIN, I2S_SD);
  Serial.println("==========================================================\n");

  // 1. Inicializa controle de áudio Mute
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, LOW);

  // 2. Inicializa o Display LCD
  inicializarLCD();
  exibirNoLCD("== SIGN TALK ==", "Cerebro Ativo");

  // 3. Apresentação inicial em voz
  anunciarSinalLibras("SIGN TALK");
  delay(500);

  // 4. Inicializa Rádio ESP-NOW e Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.print("[WiFi] MAC Deste Receptor ESP32-S3: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESP-NOW] ERRO CRÍTICO ao inicializar ESP-NOW!");
    exibirNoLCD("Erro ESP-NOW!", "Reinicie o chip");
    while (true) delay(100);
  }

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  esp_now_register_recv_cb([](const esp_now_recv_info_t *info, const uint8_t *dados, int tam) {
    tratarDadosRecebidos(dados, tam);
  });
#else
  esp_now_register_recv_cb([](const uint8_t *mac, const uint8_t *dados, int tam) {
    tratarDadosRecebidos(dados, tam);
  });
#endif

  resetarInferencia();

  Serial.println("[ESP-NOW] Receptor de Luvas iniciado com sucesso!");
  Serial.println("[MODO ATUAL] IA (Inferencia Edge Impulse) Ativado.");
  Serial.println("-> Faca sinais com a luva OU digite qualquer letra/comando no Serial!\n");
  exibirNoLCD("Aguardando Luva", "Modo IA Ativo");
  imprimirAjuda();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  // 1. Processa comandos digitados no Monitor Serial
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

  // 2. Processa de acordo com o Modo Atual
  if (modoAtual == MODO_DEBUG) {
    if (temDireita && luvaConectada(ultimoRecebidoDireita)) imprimirDadosLuva(luvaDireita);
    if (temEsquerda && luvaConectada(ultimoRecebidoEsquerda)) imprimirDadosLuva(luvaEsquerda);

    if (millis() - ultimoStatusDebug > 3000UL) {
      ultimoStatusDebug = millis();
      if (!luvaConectada(ultimoRecebidoDireita) && !luvaConectada(ultimoRecebidoEsquerda)) {
        Serial.println("[AVISO] Nenhuma luva conectada no momento...");
      }
    }
  }
  else if (modoAtual == MODO_COLETA) {
    if (temDireita && luvaConectada(ultimoRecebidoDireita)) enviarCSV(luvaDireita);
    if (temEsquerda && luvaConectada(ultimoRecebidoEsquerda)) enviarCSV(luvaEsquerda);
  }
  else if (modoAtual == MODO_INFERENCIA) {
    // Por padrão executamos a IA na Luva Direita
    if (temDireita && luvaConectada(ultimoRecebidoDireita)) {
      processarInferencia(luvaDireita);
    }
  }

  delay(5);
}

// ============================================================
// REPRODUÇÃO DE ÁUDIO E ANÚNCIO NO LCD
// ============================================================
void anunciarSinalLibras(String sinal) {
  sinal.trim();
  sinal.toUpperCase();

  const unsigned char *audioPtr = NULL;
  unsigned int audioLen = 0;
  String textoExibicao = "";

  if (obterAudioLibras(sinal, &audioPtr, &audioLen, &textoExibicao)) {
    if (sinal.length() == 1 && sinal.charAt(0) >= 'A' && sinal.charAt(0) <= 'Z') {
      exibirNoLCD("Luva Libras IA:", "   [ " + sinal + " ]");
    } else {
      exibirNoLCD("Luva Libras IA:", textoExibicao);
    }
    reproduzirAudioPROGMEM(audioPtr, audioLen, textoExibicao);
  } else {
    Serial.printf("[AVISO] O sinal \"%s\" foi classificado pela IA, mas não possui áudio gravado.\n", sinal.c_str());
    exibirNoLCD("IA Reconheceu:", sinal);
  }
}

void reproduzirAudioPROGMEM(const unsigned char *dados, unsigned int tamanho, String texto) {
  if (dados == NULL || tamanho == 0) return;

  Serial.println("--------------------------------------------------");
  Serial.printf("[FALANDO] \"%s\" (%d bytes)\n", texto.c_str(), tamanho);

  AudioFileSourcePROGMEM *fonte = new AudioFileSourcePROGMEM(dados, tamanho);
  AudioGeneratorMP3 *mp3 = new AudioGeneratorMP3();
  AudioOutputI2S *saidaI2S = new AudioOutputI2S();

  saidaI2S->SetPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
  saidaI2S->SetGain(0.0f);

  ligarAmplificador(saidaI2S);
  mp3->begin(fonte, saidaI2S);

  while (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }

  mp3->stop();
  desligarAmplificador(saidaI2S);
  saidaI2S->stop();

  delete mp3;
  delete fonte;
  delete saidaI2S;

  Serial.println("[CONCLUÍDO] Alto-falante em silêncio (Mute).");
  Serial.println("--------------------------------------------------\n");
}

void ligarAmplificador(AudioOutputI2S *out) {
  digitalWrite(I2S_SD, HIGH);
  if (out != NULL) out->SetGain(volumeAtual);
  delay(15);
}

void desligarAmplificador(AudioOutputI2S *out) {
  if (out != NULL) out->SetGain(0.0f);
  digitalWrite(I2S_SD, LOW);
}

// ============================================================
// CONTROLE DO LCD E LIMPEZA DE ACENTOS
// ============================================================
void inicializarLCD() {
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void exibirNoLCD(String linha1, String linha2) {
  lcd.clear();
  String l1Clean = removerAcentosLCD(linha1);
  String l2Clean = removerAcentosLCD(linha2);

  int espacosL1 = (16 - l1Clean.length()) / 2;
  if (espacosL1 > 0 && l1Clean.length() < 16) {
    String spc = "";
    for (int i = 0; i < espacosL1; i++) spc += " ";
    l1Clean = spc + l1Clean;
  }

  lcd.setCursor(0, 0);
  lcd.print(l1Clean.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(l2Clean.substring(0, 16));
}

String removerAcentosLCD(String texto) {
  String out = texto;
  out.replace("á", "a"); out.replace("à", "a"); out.replace("ã", "a"); out.replace("â", "a"); out.replace("Á", "A");
  out.replace("é", "e"); out.replace("ê", "e"); out.replace("É", "E");
  out.replace("í", "i"); out.replace("Í", "I");
  out.replace("ó", "o"); out.replace("ô", "o"); out.replace("õ", "o"); out.replace("Ó", "O");
  out.replace("ú", "u"); out.replace("ü", "u"); out.replace("Ú", "U");
  out.replace("ç", "c"); out.replace("Ç", "C");
  return out;
}

// ============================================================
// EDGE IMPULSE - INFERÊNCIA E BUFFER
// ============================================================
void resetarInferencia() {
  ei_buffer_index = 0;
  contadorEstavel = 0;
  ultimaLetraIA = "";
  for (size_t i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) ei_buffer[i] = 0.0f;
}

void adicionarAmostraAoBuffer(const DadosLuva &luva) {
  if (ei_buffer_index + NUM_EIXOS > EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) ei_buffer_index = 0;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_polegar;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_indicador;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_medio;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_anelar;
  ei_buffer[ei_buffer_index++] = (float)luva.flex_minimo;
}

bool bufferProntoParaInferencia() {
  return ei_buffer_index >= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
}

void processarInferencia(const DadosLuva &luva) {
  adicionarAmostraAoBuffer(luva);
  if (bufferProntoParaInferencia()) executarInferenciaBuffer(luva);
}

void executarInferenciaBuffer(const DadosLuva &luva) {
  signal_t signal;
  int err = numpy::signal_from_buffer(ei_buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (err != 0) {
    resetarInferencia();
    return;
  }

  ei_impulse_result_t result = { 0 };
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
  if (res != EI_IMPULSE_OK) {
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

  // Se a confiança for baixa ou for um sinal que não queremos testar agora, reseta
  if (maiorConfianca < CONFIANCA_MINIMA || !letraEstaFuncionando(String(melhorLetra))) {
    contadorEstavel = 0;
    ultimaLetraIA = "";
    ei_buffer_index = 0;
    return;
  }

  if (ultimaLetraIA == melhorLetra) {
    contadorEstavel++;
  } else {
    ultimaLetraIA = melhorLetra;
    contadorEstavel = 1;
  }

  // Quando o sinal for confirmado o número de vezes estáveis (REPETICOES_ESTAVEIS):
  if (contadorEstavel >= REPETICOES_ESTAVEIS) {
    unsigned long agora = millis();
    
    // Evita repetir a fala como metralhadora: só fala se mudou o sinal OU se passou 3 segundos
    if (String(melhorLetra) != ultimoSinalAnunciado || (agora - ultimoAudioTocado) > 3000UL) {
      Serial.printf("\n[IA CONFIRMOU] Sinal da Luva: \"%s\" | Confiança: %.2f%%\n", melhorLetra, maiorConfianca * 100.0f);
      anunciarSinalLibras(String(melhorLetra));
      ultimoSinalAnunciado = String(melhorLetra);
      ultimoAudioTocado = agora;
    }
    
    contadorEstavel = 0;
    ultimaLetraIA = "";
  }

  ei_buffer_index = 0; // Zera para iniciar nova janela limpa
}

// ============================================================
// ESP-NOW E RECEPÇÃO DE DADOS
// ============================================================
void tratarDadosRecebidos(const uint8_t *dados, int tamanho) {
  if (tamanho != sizeof(DadosLuva)) return;
  DadosLuva temp;
  memcpy(&temp, dados, sizeof(DadosLuva));

  if (temp.id == ID_LUVA_DIREITA) {
    memcpy(&luvaDireita, dados, sizeof(DadosLuva));
    novoDadoDireita = true;
    ultimoRecebidoDireita = millis();
  } else if (temp.id == ID_LUVA_ESQUERDA) {
    memcpy(&luvaEsquerda, dados, sizeof(DadosLuva));
    novoDadoEsquerda = true;
    ultimoRecebidoEsquerda = millis();
  }
}

bool luvaConectada(unsigned long ultimoRecebido) {
  if (ultimoRecebido == 0) return false;
  return (millis() - ultimoRecebido) < TIMEOUT_CONEXAO;
}

// ============================================================
// PROCESSADOR DE COMANDOS DO MONITOR SERIAL
// ============================================================
void processarComandoSerial() {
  if (!Serial.available()) return;

  String comando = Serial.readStringUntil('\n');
  comando.trim();
  comando.toUpperCase();

  if (comando == "DEBUG" || comando == "/DEBUG") {
    modoAtual = MODO_DEBUG;
    Serial.println("[MODO] Debug Ativado (Mostrando sensores brutos na tela)");
    exibirNoLCD("Modo: DEBUG", "Sensores Brutos");
    return;
  }
  if (comando == "COLETA" || comando == "/COLETA") {
    modoAtual = MODO_COLETA;
    Serial.println("[MODO] Coleta CSV Ativada");
    imprimirCabecalhoCSV();
    exibirNoLCD("Modo: COLETA", "Gerando CSV...");
    return;
  }
  if (comando == "IA" || comando == "/IA" || comando == "INFERENCIA") {
    modoAtual = MODO_INFERENCIA;
    resetarInferencia();
    Serial.println("[MODO] IA Edge Impulse e Voz Ativados!");
    exibirNoLCD("Modo: IA", "Pronto p/ Luva");
    return;
  }
  if (comando == "/TESTE" || comando == "TESTE") {
    executarTesteAutomatico();
    return;
  }
  if (comando == "/LISTA" || comando == "LISTA") {
    listarVocabulario();
    return;
  }
  if (comando == "/SCAN" || comando == "SCAN") {
    realizarScanI2C();
    return;
  }
  if (comando == "/AJUDA" || comando == "AJUDA" || comando == "STATUS") {
    imprimirAjuda();
    return;
  }

  // Se digitar uma palavra ou letra manualmente, testa no alto-falante
  if (comando.length() > 0) {
    Serial.printf("[TESTE MANUAL] Simulando sinal: \"%s\"\n", comando.c_str());
    anunciarSinalLibras(comando);
  }
}

// ============================================================
// UTILITÁRIOS E AJUDA
// ============================================================
void imprimirDadosLuva(const DadosLuva &luva) {
  Serial.printf("[%c] Flex: P=%d I=%d M=%d A=%d m=%d | Acel: X=%.2f Y=%.2f Z=%.2f\n",
                luva.id, luva.flex_polegar, luva.flex_indicador, luva.flex_medio, luva.flex_anelar, luva.flex_minimo,
                luva.acel_x, luva.acel_y, luva.acel_z);
}

void enviarCSV(const DadosLuva &luva) {
  Serial.printf("DATA,%c,%d,%d,%d,%d,%d,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.2f,%.2f\n",
                luva.id, luva.flex_polegar, luva.flex_indicador, luva.flex_medio, luva.flex_anelar, luva.flex_minimo,
                luva.acel_x, luva.acel_y, luva.acel_z, luva.giro_x, luva.giro_y, luva.giro_z, luva.roll, luva.pitch);
}

void imprimirCabecalhoCSV() {
  Serial.println("HEADER,id,flex_polegar,flex_indicador,flex_medio,flex_anelar,flex_minimo,acel_x,acel_y,acel_z,giro_x,giro_y,giro_z,roll,pitch");
}

void executarTesteAutomatico() {
  Serial.println("\n[TESTE AUTOMÁTICO] Simulando reconhecimento de sinais...");
  anunciarSinalLibras("OLA MUNDO");
  delay(1000);
  anunciarSinalLibras("A");
  delay(800);
  anunciarSinalLibras("B");
  delay(800);
  anunciarSinalLibras("OBRIGADO");
  delay(1000);
  exibirNoLCD("Aguardando Luva", "Modo IA Ativo");
}

void listarVocabulario() {
  Serial.println("==========================================================");
  Serial.println("     DICIONÁRIO OFFLINE (41 ÁUDIOS GRAVADOS NO CHIP):");
  Serial.println("==========================================================");
  Serial.println(" --- PALAVRAS: OLA, OLA MUNDO, SIM, NAO, OBRIGADO, BOM DIA,");
  Serial.println("               BOA TARDE, BOA NOITE, TUDO BEM, SIGN TALK, AJUDA,");
  Serial.println("               POR FAVOR, AGUA, PAZ, AMOR.");
  Serial.println(" --- ALFABETO: A até Z");
  Serial.println("==========================================================\n");
}

void realizarScanI2C() {
  Serial.println("\n[I2C SCANNER] Escaneando barramento I2C...");
  int dev = 0;
  for (byte i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  -> LCD encontrado em: 0x%02X\n", i);
      dev++;
    }
  }
  if (dev == 0) Serial.println("  -> Nenhum LCD encontrado!");
}

void imprimirAjuda() {
  Serial.println("==========================================================");
  Serial.println("            STATUS E COMANDOS DO SISTEMA");
  Serial.println("==========================================================");
  Serial.printf("Modo Atual: %s\n", (modoAtual == MODO_INFERENCIA) ? "IA (Inferencia na Luva)" : (modoAtual == MODO_DEBUG) ? "DEBUG" : "COLETA");
  Serial.printf("Luva Direita : %s\n", luvaConectada(ultimoRecebidoDireita) ? "ONLINE" : "OFFLINE");
  Serial.printf("Luva Esquerda: %s\n", luvaConectada(ultimoRecebidoEsquerda) ? "ONLINE" : "OFFLINE");
  Serial.println();
  Serial.println("Comandos no Serial:");
  Serial.println("  IA       ➔ Ativa o reconhecimento da luva com fala e LCD");
  Serial.println("  DEBUG    ➔ Mostra os sensores brutos da luva");
  Serial.println("  COLETA   ➔ Gera CSV para treinar no Edge Impulse");
  Serial.println("  /TESTE   ➔ Simula falas e tela automaticamente");
  Serial.println("  /LISTA   ➔ Lista os 41 áudios disponíveis");
  Serial.println("  A, B, C  ➔ Digite qualquer letra ou palavra para testar a voz");
  Serial.println("==========================================================\n");
}
