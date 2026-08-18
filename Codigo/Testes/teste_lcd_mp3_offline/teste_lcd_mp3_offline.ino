/*
 * ============================================================================
 *  SIGN TALK - SISTEMA COMPLETO: DISPLAY LCD 16x2 + ÁUDIO MP3 OFFLINE (VOZ REAL)
 *  Placa: ESP32-S3 (16MB Flash / 8MB PSRAM)
 *  Pasta: Codigo Antigo/testes/teste_lcd_mp3_offline
 * ============================================================================
 *
 *  OBJETIVO:
 *    Integrar o Display LCD 16x2 (I2C) com a síntese de voz humana de alta
 *    qualidade em MP3 tocada 100% OFFLINE (da memória Flash PROGMEM).
 *    Quando o usuário ou o sensor envia uma letra (A-Z) ou palavra (OLA, OBRIGADO,
 *    OLA MUNDO), o ESP32 exibe o texto limpo no LCD e pronuncia com voz real no
 *    alto-falante instantaneamente!
 *
 *  ============================================================================
 *  BIBLIOTECAS NECESSÁRIAS:
 *  ============================================================================
 *    1. LiquidCrystal I2C (por Frank de Brabander - versão 1.1.4 ou superior)
 *    2. ESP8266Audio      (por Earle F. Philhower) -> Decodificador MP3 e I2S
 *
 *  ============================================================================
 *  ESQUEMA DE LIGAÇÃO DOS PINOS NO ESP32-S3:
 *  ============================================================================
 *  📌 1. DISPLAY LCD 16x2 I2C (Módulo PCF8574):
 *     - GND  ➔ GND
 *     - VCC  ➔ 5V (Para bom contraste da tela)
 *     - SDA  ➔ GPIO 8
 *     - SCL  ➔ GPIO 9
 *     * Obs: Use o comando /SCAN no Monitor Serial para verificar o endereço I2C!
 *
 *  📌 2. AMPLIFICADOR ÁUDIO I2S MAX98357A:
 *     - VIN  ➔ 3.3V (ou 5V)
 *     - GND  ➔ GND
 *     - GAIN ➔ GND
 *     - SD   ➔ GPIO 15 (Silenciamento Mute automático)
 *     - BCLK ➔ GPIO 16
 *     - LRC  ➔ GPIO 17
 *     - DIN  ➔ GPIO 18
 *     - Saídas (+ e -) ➔ Alto-falante
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include "audios_libras.h" // Arquivo com os 41 áudios MP3 gravados na Flash (Palavras + Alfabeto A-Z)

// ============================================================
// CONFIGURAÇÃO DOS PINOS - ESP32-S3
// ============================================================
// 1. Display LCD 16x2 I2C
#define LCD_ENDERECO 0x27  // Endereço padrão (use /SCAN se a tela não acender)
#define LCD_COLUNAS  16
#define LCD_LINHAS   2
#define LCD_SDA      8
#define LCD_SCL      9

// 2. Áudio I2S
#define I2S_BCLK       16
#define I2S_LRC        17
#define I2S_DIN        18
#define I2S_SD         15
#define VOLUME_INICIAL 0.35f

float volumeAtual = VOLUME_INICIAL;
LiquidCrystal_I2C lcd(LCD_ENDERECO, LCD_COLUNAS, LCD_LINHAS);

// Protótipos de funções
void inicializarLCD();
void exibirNoLCD(String linha1, String linha2);
String removerAcentosLCD(String texto);
void ligarAmplificador(AudioOutputI2S *out);
void desligarAmplificador(AudioOutputI2S *out);
void reproduzirAudioPROGMEM(const unsigned char *dados, unsigned int tamanho, String texto);
void processarComando(String entrada);
void executarTesteAutomatico();
void listarVocabulario();
void realizarScanI2C();
void imprimirAjuda();

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("  SIGN TALK - SISTEMA INTEGRADO: LCD 16x2 + ÁUDIO MP3 OFFLINE");
  Serial.println("==========================================================");
  Serial.printf("Pinos LCD: SDA=%d | SCL=%d | Endereço=0x%02X\n", LCD_SDA, LCD_SCL, LCD_ENDERECO);
  Serial.printf("Pinos Áudio: BCLK=%d | LRC=%d | DIN=%d | SD=%d | GAIN=GND\n", I2S_BCLK, I2S_LRC, I2S_DIN, I2S_SD);
  Serial.println("==========================================================\n");

  // 1. Configura o pino SD em modo LOW (Mute/Dormência)
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, LOW);

  // 2. Inicializa o Display LCD
  inicializarLCD();
  exibirNoLCD("== SIGN TALK ==", "Iniciando...");

  // 3. Reproduz áudio e exibe boas-vindas
  Serial.println("[INIT] Testando sistema de tela e áudio...");
  const unsigned char *audioPtr = NULL;
  unsigned int audioLen = 0;
  String textoExibicao = "";
  
  if (obterAudioLibras("SIGN TALK", &audioPtr, &audioLen, &textoExibicao)) {
    exibirNoLCD("Sign Talk", "Tradutor Libras");
    reproduzirAudioPROGMEM(audioPtr, audioLen, textoExibicao);
  }

  exibirNoLCD("Sistema Pronto!", "Digite no Serial");
  imprimirAjuda();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    if (entrada.length() > 0) {
      processarComando(entrada);
    }
  }
  delay(20);
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
  
  // Limpa acentos para não gerar caracteres estranhos na tela LCD 16x2
  String l1Clean = removerAcentosLCD(linha1);
  String l2Clean = removerAcentosLCD(linha2);

  // Centraliza a linha 1 se for curta
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
// CONTROLE DO AMPLIFICADOR (MUTE / SHUTDOWN)
// ============================================================
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
// REPRODUÇÃO DE ÁUDIO MP3 DA MEMÓRIA FLASH (PROGMEM)
// ============================================================
void reproduzirAudioPROGMEM(const unsigned char *dados, unsigned int tamanho, String texto) {
  if (dados == NULL || tamanho == 0) return;

  Serial.println("--------------------------------------------------");
  Serial.printf("[FALANDO] \"%s\" (%d bytes da Flash)\n", texto.c_str(), tamanho);

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
  Serial.println("--------------------------------------------------\n-> Digite outra palavra ou letra no Serial:");
}

// ============================================================
// PROCESSADOR DE COMANDOS E VOCABULÁRIO
// ============================================================
void processarComando(String entrada) {
  String comando = entrada;
  comando.trim();
  comando.toUpperCase();

  // 1. Comandos do Sistema
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
  if (comando == "/AJUDA" || comando == "AJUDA" || comando == "HELP") {
    imprimirAjuda();
    return;
  }
  if (comando.startsWith("/VOL") || comando.startsWith("VOL")) {
    int posEspaco = entrada.indexOf(' ');
    if (posEspaco != -1) {
      float novoVol = entrada.substring(posEspaco + 1).toFloat();
      if (novoVol >= 0.05f && novoVol <= 1.0f) {
        volumeAtual = novoVol;
        Serial.printf("\n[VOLUME] Novo volume ajustado para: %.2f\n\n", volumeAtual);
        exibirNoLCD("Volume Ajustado:", String(volumeAtual));
        const unsigned char *ptr; unsigned int len; String txt;
        if (obterAudioLibras("SIM", &ptr, &len, &txt)) reproduzirAudioPROGMEM(ptr, len, "Volume alterado");
        return;
      }
    }
    Serial.println("\n[AVISO] Uso correto: /VOL 0.4 (valores entre 0.05 e 1.0)\n");
    return;
  }

  // 2. Busca o comando na memória Flash (audios_libras.h)
  const unsigned char *audioPtr = NULL;
  unsigned int audioLen = 0;
  String textoExibicao = "";

  if (obterAudioLibras(comando, &audioPtr, &audioLen, &textoExibicao)) {
    // Se for apenas 1 letra (Alfabeto A-Z), exibe formato especial na tela
    if (comando.length() == 1 && comando.charAt(0) >= 'A' && comando.charAt(0) <= 'Z') {
      exibirNoLCD("Letra Libras:", "   [ " + comando + " ]");
    } else {
      exibirNoLCD("Libras Falado:", textoExibicao);
    }
    reproduzirAudioPROGMEM(audioPtr, audioLen, textoExibicao);
  } else {
    Serial.printf("\n[AVISO] O comando \"%s\" ainda não está cadastrado na memória Flash!\n", entrada.c_str());
    Serial.println("Digite /LISTA para ver os 41 itens disponíveis.\n");
    exibirNoLCD("Nao Cadastrado:", entrada);
  }
}

// ============================================================
// ESCANeador I2C (/SCAN)
// ============================================================
void realizarScanI2C() {
  Serial.println("\n[I2C SCANNER] Escaneando barramento I2C (SDA=8, SCL=9)...");
  exibirNoLCD("I2C Scanner", "Buscando LCD...");
  int dispositivos = 0;
  for (byte endereco = 1; endereco < 127; endereco++) {
    Wire.beginTransmission(endereco);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  -> Dispositivo encontrado no endereço: 0x%02X\n", endereco);
      exibirNoLCD("LCD Encontrado!", "Endereco: 0x" + String(endereco, HEX));
      dispositivos++;
    }
  }
  if (dispositivos == 0) {
    Serial.println("  -> Nenhum dispositivo I2C encontrado! Verifique fios SDA e SCL.");
    exibirNoLCD("Erro I2C LCD!", "Verifique os fios");
  } else {
    Serial.printf("[I2C SCANNER] Concluído. %d dispositivo(s) encontrado(s).\n\n", dispositivos);
  }
}

// ============================================================
// LISTA DE PALAVRAS DISPONÍVEIS
// ============================================================
void listarVocabulario() {
  Serial.println("==========================================================");
  Serial.println("     DICIONÁRIO OFFLINE (41 ÁUDIOS GRAVADOS NO CHIP):");
  Serial.println("==========================================================");
  Serial.println("  1. OLA         ➔ \"Olá!\"");
  Serial.println("  2. OLA MUNDO   ➔ \"Olá, mundo!\"");
  Serial.println("  3. SIM         ➔ \"Sim.\"");
  Serial.println("  4. NAO         ➔ \"Não.\"");
  Serial.println("  5. OBRIGADO    ➔ \"Muito obrigado!\"");
  Serial.println("  6. BOM DIA     ➔ \"Bom dia!\"");
  Serial.println("  7. BOA TARDE   ➔ \"Boa tarde!\"");
  Serial.println("  8. BOA NOITE   ➔ \"Boa noite!\"");
  Serial.println("  9. TUDO BEM    ➔ \"Tudo bem?\"");
  Serial.println(" 10. SIGN TALK   ➔ \"Sign Talk, tradutor de Libras em voz.\"");
  Serial.println(" 11. AJUDA       ➔ \"Preciso de ajuda, por favor.\"");
  Serial.println(" 12. POR FAVOR   ➔ \"Por favor.\"");
  Serial.println(" 13. AGUA        ➔ \"Quero água.\"");
  Serial.println(" 14. PAZ         ➔ \"Paz.\"");
  Serial.println(" 15. AMOR        ➔ \"Amor.\"");
  Serial.println(" --- ALFABETO MANUAL A até Z (ex: digite A, B, C... Z) ---");
  Serial.println("==========================================================\n");
}

// ============================================================
// TESTE AUTOMÁTICO (/TESTE)
// ============================================================
void executarTesteAutomatico() {
  Serial.println("\n[TESTE AUTOMÁTICO] Testando Display LCD e Voz MP3...");
  delay(500);

  const unsigned char *ptr; unsigned int len; String txt;

  if (obterAudioLibras("OLA MUNDO", &ptr, &len, &txt)) {
    exibirNoLCD("Sign Talk Teste:", txt);
    reproduzirAudioPROGMEM(ptr, len, txt);
  }
  delay(1000);

  if (obterAudioLibras("A", &ptr, &len, &txt)) {
    exibirNoLCD("Letra Libras:", "   [ A ]");
    reproduzirAudioPROGMEM(ptr, len, txt);
  }
  delay(800);

  if (obterAudioLibras("B", &ptr, &len, &txt)) {
    exibirNoLCD("Letra Libras:", "   [ B ]");
    reproduzirAudioPROGMEM(ptr, len, txt);
  }
  delay(800);

  if (obterAudioLibras("OBRIGADO", &ptr, &len, &txt)) {
    exibirNoLCD("Libras Falado:", txt);
    reproduzirAudioPROGMEM(ptr, len, txt);
  }
  delay(1000);

  exibirNoLCD("Teste Concluido!", "Digite no Serial");
  Serial.println("[TESTE AUTOMÁTICO] Concluído com sucesso!\n");
}

// ============================================================
// MENU DE AJUDA
// ============================================================
void imprimirAjuda() {
  Serial.println("==========================================================");
  Serial.println("       INSTRUÇÕES DO SISTEMA LCD + ÁUDIO MP3 OFFLINE:");
  Serial.println("==========================================================");
  Serial.println("1. Digite qualquer LETRA (A-Z) ou PALAVRA (OLA MUNDO, AGUA)");
  Serial.println("   no Serial e pressione ENTER.");
  Serial.println("2. O texto aparecerá na tela do LCD e será falado com");
  Serial.println("   voz de estúdio pelo alto-falante sem nenhuma internet!");
  Serial.println();
  Serial.println("Comandos Especiais:");
  Serial.println("  /TESTE   ➔ Executa rotina de demonstração (LCD + Voz)");
  Serial.println("  /LISTA   ➔ Mostra os 41 comandos disponíveis");
  Serial.println("  /SCAN    ➔ Verifica se o Display LCD está respondendo");
  Serial.println("  /VOL 0.4 ➔ Ajusta o volume do alto-falante");
  Serial.println("  /AJUDA   ➔ Exibe este menu novamente");
  Serial.println("==========================================================\n");
  Serial.println("-> Digite algo e pressione ENTER:");
}
