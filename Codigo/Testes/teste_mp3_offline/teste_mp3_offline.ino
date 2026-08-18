/*
 * ============================================================================
 *  SIGN TALK - TESTE DE VOZ HUMANA EM MP3 (100% OFFLINE / SEM WI-FI / PROGMEM)
 *  Placa: ESP32-S3
 *  Pasta: Codigo Antigo/testes/teste_mp3_offline
 * ============================================================================
 *
 *  OBJETIVO:
 *    Demonstrar o funcionamento 100% OFFLINE (sem internet e sem Wi-Fi) com
 *    áudios MP3 de voz humana real gravados diretamente na memória Flash do
 *    ESP32-S3 (através do arquivo audios_libras.h).
 *
 *  ============================================================================
 *  COMO USAR NO MONITOR SERIAL (115200 baud):
 *  ============================================================================
 *    - Digite uma palavra do vocabulário (ex: "OLA", "SIM", "OBRIGADO", "AGUA")
 *      e pressione ENTER para ouvir a voz real instantaneamente!
 *    - Comandos Especiais:
 *        "/TESTE"   ➔ Executa sequência demonstrativa de falas em MP3.
 *        "/LISTA"   ➔ Exibe todas as 14 palavras disponíveis na memória.
 *        "/VOL 0.4" ➔ Ajusta o volume do alto-falante (entre 0.05 e 1.0).
 *        "/AJUDA"   ➔ Exibe o menu de instruções.
 *
 *  ============================================================================
 *  ESQUEMA DE LIGAÇÃO NO ESP32-S3 (O mesmo de sempre):
 *  ============================================================================
 *    - VIN  ➔ Pino 3.3V
 *    - GND  ➔ GND
 *    - GAIN ➔ GND
 *    - SD   ➔ GPIO 15 (Controle inteligente de silêncio Mute)
 *    - BCLK ➔ GPIO 16
 *    - LRC  ➔ GPIO 17
 *    - DIN  ➔ GPIO 18
 *    - Saídas (+ e -) ➔ Alto-falante
 * ============================================================================
 */

#include <Arduino.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include "audios_libras.h" // Arquivo gerado com os 14 áudios MP3 na memória Flash

// ============================================================
// CONFIGURAÇÃO DOS PINOS DO ÁUDIO - ESP32-S3
// ============================================================
#define I2S_BCLK       16
#define I2S_LRC        17
#define I2S_DIN        18
#define I2S_SD         15
#define VOLUME_INICIAL 0.35f // Volume padrão (entre 0.05 e 1.0)

float volumeAtual = VOLUME_INICIAL;

// Protótipos de funções
void ligarAmplificador(AudioOutputI2S *out);
void desligarAmplificador(AudioOutputI2S *out);
void reproduzirAudioPROGMEM(const unsigned char *dados, unsigned int tamanho, String texto);
void processarComando(String entrada);
void executarTesteAutomatico();
void listarVocabulario();
void imprimirAjuda();

// ============================================================
// SETUP - INICIALIZAÇÃO
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("  SIGN TALK - VOZ HUMANA EM MP3 (100% OFFLINE / SEM WI-FI)");
  Serial.println("==========================================================");
  Serial.printf("Pinos Áudio: BCLK=%d | LRC=%d | DIN=%d | SD=%d | GAIN=GND\n", I2S_BCLK, I2S_LRC, I2S_DIN, I2S_SD);
  Serial.println("==========================================================\n");

  // Configura o pino SD em modo LOW (Mute/Dormência no hardware)
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, LOW);

  // Reproduz o áudio inicial de boas-vindas diretamente da Flash
  Serial.println("[INIT] Testando reprodução offline da memória interna...");
  const unsigned char *audioPtr = NULL;
  unsigned int audioLen = 0;
  String textoExibicao = "";
  
  if (obterAudioLibras("SIGN TALK", &audioPtr, &audioLen, &textoExibicao)) {
    reproduzirAudioPROGMEM(audioPtr, audioLen, textoExibicao);
  }

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
// CONTROLE DO AMPLIFICADOR (MUTE / SHUTDOWN)
// ============================================================
void ligarAmplificador(AudioOutputI2S *out) {
  digitalWrite(I2S_SD, HIGH); // Acorda o chip no hardware
  if (out != NULL) out->SetGain(volumeAtual); // Aplica volume digital
  delay(15); // Pausa para evitar estalo no falante
}

void desligarAmplificador(AudioOutputI2S *out) {
  if (out != NULL) out->SetGain(0.0f); // Muta no software
  digitalWrite(I2S_SD, LOW); // Coloca em modo de dormência (0.6 uA)
}

// ============================================================
// REPRODUÇÃO DE ÁUDIO MP3 DA MEMÓRIA FLASH (PROGMEM)
// ============================================================
void reproduzirAudioPROGMEM(const unsigned char *dados, unsigned int tamanho, String texto) {
  if (dados == NULL || tamanho == 0) return;

  Serial.println("--------------------------------------------------");
  Serial.printf("[AUDIO OFFLINE] Falando: \"%s\" (%d bytes do chip)\n", texto.c_str(), tamanho);

  // Instancia os objetos da biblioteca ESP8266Audio sob demanda
  AudioFileSourcePROGMEM *fonte = new AudioFileSourcePROGMEM(dados, tamanho);
  AudioGeneratorMP3 *mp3 = new AudioGeneratorMP3();
  AudioOutputI2S *saidaI2S = new AudioOutputI2S();

  saidaI2S->SetPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
  saidaI2S->SetGain(0.0f);

  ligarAmplificador(saidaI2S);
  mp3->begin(fonte, saidaI2S);

  // Loop de decodificação e reprodução instantânea
  while (mp3->isRunning()) {
    if (!mp3->loop()) mp3->stop();
  }

  // Finaliza e muta o áudio
  mp3->stop();
  desligarAmplificador(saidaI2S);
  saidaI2S->stop();

  // Limpa os objetos da memória (evita erro de exaustão de canais I2S no ESP32 Core 3.x)
  delete mp3;
  delete fonte;
  delete saidaI2S;

  Serial.println("[CONCLUÍDO] Fim da fala. Alto-falante em silêncio absoluto (Mute).");
  Serial.println("--------------------------------------------------\n-> Digite outra palavra e pressione ENTER:");
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
  if (comando == "/LISTA" || comando == "LISTA" || comando == "PALAVRAS") {
    listarVocabulario();
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
        const unsigned char *ptr; unsigned int len; String txt;
        if (obterAudioLibras("SIM", &ptr, &len, &txt)) reproduzirAudioPROGMEM(ptr, len, "Volume alterado");
        return;
      }
    }
    Serial.println("\n[AVISO] Uso correto: /VOL 0.4 (valores entre 0.05 e 1.0)\n");
    return;
  }

  // 2. Busca a palavra no dicionário offline (audios_libras.h)
  const unsigned char *audioPtr = NULL;
  unsigned int audioLen = 0;
  String textoExibicao = "";

  if (obterAudioLibras(comando, &audioPtr, &audioLen, &textoExibicao)) {
    reproduzirAudioPROGMEM(audioPtr, audioLen, textoExibicao);
  } else {
    Serial.printf("\n[AVISO] O comando \"%s\" ainda não está cadastrado na memória Flash!\n", entrada.c_str());
    Serial.println("Digite /LISTA para ver todos os 41 itens (15 palavras + 26 letras do alfabeto) disponíveis.\n");
  }
}

// ============================================================
// LISTA DE PALAVRAS E LETRAS DISPONÍVEIS NA MEMÓRIA
// ============================================================
void listarVocabulario() {
  Serial.println("==========================================================");
  Serial.println("     DICIONÁRIO OFFLINE (41 ÁUDIOS GRAVADOS NO CHIP):");
  Serial.println("==========================================================");
  Serial.println(" --- PALAVRAS E FRASES ---");
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
  Serial.println();
  Serial.println(" --- ALFABETO MANUAL (DACTILOLOGIA LIBRAS) ---");
  Serial.println(" Letras individuais de A até Z (ex: digite A, B, C... Z)");
  Serial.println("==========================================================\n");
  Serial.println("-> Digite qualquer palavra ou letra e dê ENTER:");
}

// ============================================================
// TESTE AUTOMÁTICO (/TESTE)
// ============================================================
void executarTesteAutomatico() {
  Serial.println("\n[TESTE AUTOMÁTICO] Reproduzindo sequência demonstrativa offline...");
  delay(500);

  const unsigned char *ptr; unsigned int len; String txt;

  if (obterAudioLibras("OLA", &ptr, &len, &txt)) reproduzirAudioPROGMEM(ptr, len, txt);
  delay(800);

  if (obterAudioLibras("TUDO BEM", &ptr, &len, &txt)) reproduzirAudioPROGMEM(ptr, len, txt);
  delay(800);

  if (obterAudioLibras("AJUDA", &ptr, &len, &txt)) reproduzirAudioPROGMEM(ptr, len, txt);
  delay(800);

  if (obterAudioLibras("OBRIGADO", &ptr, &len, &txt)) reproduzirAudioPROGMEM(ptr, len, txt);
  delay(800);

  Serial.println("[TESTE AUTOMÁTICO] Concluído com sucesso!\n");
}

// ============================================================
// MENU DE AJUDA
// ============================================================
void imprimirAjuda() {
  Serial.println("==========================================================");
  Serial.println("               INSTRUÇÕES DO TESTE OFFLINE:");
  Serial.println("==========================================================");
  Serial.println("1. Digite uma palavra (ex: OLA, OBRIGADO, AGUA) e dê ENTER.");
  Serial.println("2. O ESP32 tocará o arquivo MP3 direto da memória Flash!");
  Serial.println();
  Serial.println("Comandos Especiais:");
  Serial.println("  /LISTA   ➔ Mostra o vocabulário de 14 palavras gravadas");
  Serial.println("  /TESTE   ➔ Fala uma sequência demonstrativa de Libras");
  Serial.println("  /VOL 0.4 ➔ Ajusta o volume de fala (entre 0.05 e 1.0)");
  Serial.println("  /AJUDA   ➔ Exibe este menu novamente");
  Serial.println("==========================================================\n");
  Serial.println("-> Digite algo e pressione ENTER:");
}
