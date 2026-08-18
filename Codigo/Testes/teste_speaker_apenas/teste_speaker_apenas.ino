/*
 * ============================================================================
 *  SIGN TALK - TESTE ISOLADO DO AMPLIFICADOR DE ÁUDIO I2S (MAX98357A)
 *  Placa: ESP32-S3
 *  Pasta: Codigo Antigo/testes/teste_speaker_apenas
 * ============================================================================
 *
 *  OBJETIVO:
 *    Testar exclusivamente o sistema de som (MAX98357A + Sintetizador SAM),
 *    sem nenhum código de display LCD. Você digita qualquer letra ou palavra
 *    no Monitor Serial para ser pronunciada pelo alto-falante.
 *
 *  ============================================================================
 *  BIBLIOTECAS NECESSÁRIAS (Instalar via Arduino Library Manager):
 *  ============================================================================
 *    1. ESP8266Audio      (por Earle F. Philhower) ➔ Controle do barramento I2S
 *    2. ESP8266SAM        (por Earle F. Philhower) ➔ Sintetizador de voz TTS
 *
 *  ============================================================================
 *  ESQUEMA DE LIGAÇÃO NO ESP32-S3:
 *  ============================================================================
 *    - VIN  ➔ Pino 3.3V (Alimentação do amplificador)
 *    - GND  ➔ GND
 *    - GAIN ➔ GND (Ligado ao terra para fixar o ganho em 6dB - economia e segurança)
 *    - SD   ➔ GPIO 15 (Controlado pelo ESP32 para silêncio 100% no modo espera)
 *    - BCLK ➔ GPIO 16 (Bit Clock / SCK)
 *    - LRC  ➔ GPIO 17 (Word Select / WS / Left-Right Clock)
 *    - DIN  ➔ GPIO 18 (Data In / SDIN / SD)
 *    - Saídas (+ e -) ➔ Alto-falante
 *
 *  ============================================================================
 *  COMO USAR NO MONITOR SERIAL (115200 baud):
 *  ============================================================================
 *    - Digite uma LETRA (ex: "A", "B", "Z") ou PALAVRA (ex: "OLA", "SIGN TALK")
 *      e pressione ENTER para que o ESP32 pronuncie.
 *    - Comandos Especiais:
 *        "/TESTE"     ➔ Fala palavras e frases de teste automáticas.
 *        "/VOL 0.3"   ➔ Altera o volume em tempo real (de 0.05 a 1.0).
 *        "/AJUDA"     ➔ Exibe este menu de ajuda no Monitor Serial.
 * ============================================================================
 */

#include <Arduino.h>
#include <AudioOutputI2S.h>
#include <ESP8266SAM.h>

// ============================================================
// CONFIGURAÇÃO DOS PINOS - ESP32-S3
// ============================================================
#define I2S_BCLK     16    // Bit Clock (BCLK / SCK)
#define I2S_LRC      17    // Word Select (LRC / WS)
#define I2S_DIN      18    // Data In (DIN / SDIN)
#define I2S_SD       15    // Pino SD (Shutdown/Mute) - Ligado no GPIO 15
#define VOLUME_INICIAL 0.25f // Volume inicial seguro para 3.3V (0.0 a 1.0)

// Instâncias de áudio
ESP8266SAM *vozSAM = NULL;
float volumeAtual = VOLUME_INICIAL;

// Protótipos de funções
void ligarAmplificador(AudioOutputI2S *out);
void desligarAmplificador(AudioOutputI2S *out);
void falarTexto(String texto);
void processarComandoOuTexto(String entrada);
void executarTesteAutomatico();
void imprimirAjuda();
String obterFonetica(String texto);

// ============================================================
// SETUP - INICIALIZAÇÃO DO SISTEMA DE ÁUDIO
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("   SIGN TALK - TESTE EXCLUSIVO DE ÁUDIO (MAX98357A)");
  Serial.println("==========================================================");
  Serial.printf("Pinos configurados no ESP32-S3:\n");
  Serial.printf("  [MAX98357A I2S] BCLK = %d | LRC = %d | DIN = %d\n", I2S_BCLK, I2S_LRC, I2S_DIN);
  Serial.printf("  [Controle Mute] SD = %d | GAIN = GND | VCC = 3.3V\n", I2S_SD);
  Serial.println("==========================================================\n");

  // 1. Configura o pino SD (Shutdown/Mute) como saída e inicia desligado (mudo)
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, LOW);

  // 2. Inicializa o Sintetizador SAM
  Serial.println("[INIT] Inicializando Sintetizador SAM e sistema I2S...");
  vozSAM = new ESP8266SAM();

  Serial.println("[INIT] Sistema pronto! Testando áudio inicial...");
  falarTexto("Sign Talk");

  // 3. Exibe o menu de instruções
  imprimirAjuda();
}

// ============================================================
// LOOP PRINCIPAL - LEITURA DO MONITOR SERIAL
// ============================================================
void loop() {
  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    if (entrada.length() > 0) {
      processarComandoOuTexto(entrada);
    }
  }
  delay(20);
}

// ============================================================
// CONTROLE DE MUTE / SILENCIAMENTO DO AMPLIFICADOR
// ============================================================
void ligarAmplificador(AudioOutputI2S *out) {
  digitalWrite(I2S_SD, HIGH); // Acorda o MAX98357A no hardware
  if (out != NULL) out->SetGain(volumeAtual); // Aplica volume digital
  delay(15); // Pequena pausa para os capacitores estabilizarem sem estalo/pop
}

void desligarAmplificador(AudioOutputI2S *out) {
  if (out != NULL) out->SetGain(0.0f); // Zera o ganho no software
  digitalWrite(I2S_SD, LOW); // Coloca o chip em modo de dormência (0.6 uA)
}

// ============================================================
// FUNÇÃO PRINCIPAL DE SÍNTESE DE VOZ
// ============================================================
void falarTexto(String texto) {
  if (texto.length() == 0) return;

  Serial.println("--------------------------------------------------");
  Serial.printf("[FALANDO] Texto original: \"%s\"\n", texto.c_str());

  // Converte para a fonética em português
  String textoFalar = obterFonetica(texto);
  if (textoFalar != texto) {
    Serial.printf("[FALANDO] Pronúncia fonética: \"%s\"\n", textoFalar.c_str());
  }

  // Cria o canal I2S sob demanda (solução obrigatória no ESP32 Core 3.x para evitar erro "no available channel found")
  AudioOutputI2S *saidaI2S = new AudioOutputI2S();
  saidaI2S->SetPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
  saidaI2S->SetGain(0.0f);

  // Aciona o alto-falante
  if (saidaI2S != NULL && vozSAM != NULL) {
    ligarAmplificador(saidaI2S);
    vozSAM->Say(saidaI2S, textoFalar.c_str());
    desligarAmplificador(saidaI2S);
  } else {
    Serial.println("[ERRO] Sistema de áudio não inicializado!");
  }

  // Libera o canal I2S da memória (garante que i2s_del_channel seja chamado pelo destrutor!)
  if (saidaI2S != NULL) {
    saidaI2S->stop();
    delete saidaI2S;
  }

  Serial.println("[CONCLUÍDO] Fim da reprodução. Alto-falante em silêncio (Mute).\n");
  Serial.println("-> Digite outra LETRA ou PALAVRA e pressione ENTER:");
}

// ============================================================
// PROCESSADOR DE COMANDOS E ENTRADA SERIAL
// ============================================================
void processarComandoOuTexto(String entrada) {
  String comando = entrada;
  comando.toUpperCase();

  // 1. Comando de Teste Automático
  if (comando == "/TESTE" || comando == "TESTE") {
    executarTesteAutomatico();
  }
  // 2. Comando de Ajuda
  else if (comando == "/AJUDA" || comando == "AJUDA" || comando == "HELP" || comando == "/HELP") {
    imprimirAjuda();
  }
  // 3. Ajuste de Volume em tempo real (ex: "/VOL 0.4")
  else if (comando.startsWith("/VOL") || comando.startsWith("VOL")) {
    int posEspaco = entrada.indexOf(' ');
    if (posEspaco != -1) {
      float novoVol = entrada.substring(posEspaco + 1).toFloat();
      if (novoVol >= 0.0f && novoVol <= 1.0f) {
        volumeAtual = novoVol;
        Serial.printf("\n[VOLUME] Novo volume ajustado para: %.2f\n\n", volumeAtual);
        falarTexto("Volume alterado");
        return;
      }
    }
    Serial.println("\n[AVISO] Uso correto para volume: /VOL 0.3 (valores entre 0.05 e 1.0)\n");
  }
  // 4. Se não for comando, fala o texto digitado
  else {
    falarTexto(entrada);
  }
}

// ============================================================
// MAPEAMENTO FONÉTICO PARA VOZ EM PORTUGUÊS
// ============================================================
String obterFonetica(String texto) {
  String t = texto;
  t.trim();
  t.toUpperCase();

  // Letras avulsas do alfabeto (Libras):
  if (t == "A") return "Ah";
  if (t == "B") return "Bay";
  if (t == "C") return "Say";
  if (t == "D") return "Day";
  if (t == "E") return "Eh";
  if (t == "F") return "Eff";
  if (t == "G") return "Jay";
  if (t == "H") return "Ah gah";
  if (t == "I") return "Eee";
  if (t == "J") return "Joh tah";
  if (t == "K") return "Kah";
  if (t == "L") return "Ell";
  if (t == "M") return "Emm";
  if (t == "N") return "Enn";
  if (t == "O") return "Oh";
  if (t == "P") return "Pay";
  if (t == "Q") return "Kay";
  if (t == "R") return "Err";
  if (t == "S") return "Ess";
  if (t == "T") return "Tay";
  if (t == "U") return "Oo";
  if (t == "V") return "Vay";
  if (t == "W") return "Dah bloo";
  if (t == "X") return "Ex";
  if (t == "Y") return "Eep see lon";
  if (t == "Z") return "Zay";

  // Palavras e saudações comuns do projeto:
  if (t == "OLA" || t == "OLÁ") return "Oh lah";
  if (t == "SIM") return "Seem";
  if (t == "NAO" || t == "NÃO") return "Now";
  if (t == "OBRIGADO" || t == "OBRIGADA") return "Oh bree gah doo";
  if (t == "BOM DIA") return "Bong dee ah";
  if (t == "BOA TARDE") return "Boh ah tar dee";
  if (t == "BOA NOITE") return "Boh ah noy chee";
  if (t == "TUDO BEM") return "Too doo beng";
  if (t == "SIGN TALK" || t == "SIGNTALK") return "Sign Talk";
  if (t == "AJUDA") return "Ah joo dah";
  if (t == "POR FAVOR") return "Por fah vor";
  if (t == "AGUA" || t == "ÁGUA") return "Ah gwah";
  if (t == "PAZ") return "Pahz";
  if (t == "AMOR") return "Ah mor";

  // Se não estiver na lista, retorna o próprio texto para o SAM pronunciar
  return texto;
}

// ============================================================
// TESTE AUTOMÁTICO DE VOZ (/TESTE)
// ============================================================
void executarTesteAutomatico() {
  Serial.println("\n[TESTE AUTOMÁTICO] Iniciando sequência de fala...");
  delay(500);

  falarTexto("Sign Talk");
  delay(600);

  falarTexto("Teste de áudio");
  delay(600);

  falarTexto("Libras");
  delay(600);

  falarTexto("Obrigado");
  delay(600);

  Serial.println("[TESTE AUTOMÁTICO] Concluído com sucesso!\n");
}

// ============================================================
// MENU DE INSTRUÇÕES
// ============================================================
void imprimirAjuda() {
  Serial.println("==========================================================");
  Serial.println("                 INSTRUÇÕES DO TESTE:");
  Serial.println("==========================================================");
  Serial.println("1. Digite qualquer LETRA (ex: A, B, C) e dê ENTER.");
  Serial.println("2. Ou digite palavras (ex: OLA, BOM DIA, SIGN TALK).");
  Serial.println("3. O ESP32 acordará o MAX98357A pelo pino 15, falará");
  Serial.println("   a pronúncia pelo alto-falante e voltará ao silêncio!");
  Serial.println();
  Serial.println("Comandos Especiais:");
  Serial.println("  /TESTE     ➔ Reproduz sequência de palavras de teste");
  Serial.println("  /VOL 0.4   ➔ Ajusta o volume no Monitor (entre 0.05 e 1.0)");
  Serial.println("  /AJUDA     ➔ Exibe este menu de instruções novamente");
  Serial.println("==========================================================\n");
  Serial.println("-> Digite algo e pressione ENTER:");
}
