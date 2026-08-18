/*
 * ============================================================================
 *  SIGN TALK - TESTE DE VOZ NATURAL COM GOOGLE TRADUTOR TTS (ONLINE / WI-FI)
 *  Placa: ESP32-S3
 *  Pasta: Codigo Antigo/testes/teste_google_tts
 * ============================================================================
 *
 *  OBJETIVO:
 *    Demonstrar a qualidade de voz 100% natural e humana em Português (pt-BR)
 *    utilizando o motor de inteligência artificial do Google Tradutor TTS.
 *    O ESP32 se conecta ao Wi-Fi, busca o áudio MP3 em tempo real e reproduz
 *    no alto-falante através do amplificador MAX98357A.
 *
 *  ============================================================================
 *  PASSO A PASSO ANTES DE CARREGAR:
 *  ============================================================================
 *    1. Altere as variáveis WIFI_SSID e WIFI_SENHA logo abaixo com os dados
 *       da sua rede Wi-Fi.
 *    2. Faça o upload para o ESP32-S3 e abra o Monitor Serial em 115200 baud.
 *    3. Digite qualquer frase ou palavra em português e ouça a voz natural!
 *
 *  ============================================================================
 *  ESQUEMA DE LIGAÇÃO NO ESP32-S3 (O mesmo que você já montou):
 *  ============================================================================
 *    - VIN  ➔ Pino 3.3V
 *    - GND  ➔ GND
 *    - GAIN ➔ GND
 *    - SD   ➔ GPIO 15 (Silenciamento automático no modo espera)
 *    - BCLK ➔ GPIO 16
 *    - LRC  ➔ GPIO 17
 *    - DIN  ➔ GPIO 18
 *    - Saídas (+ e -) ➔ Alto-falante
 * ============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

// ============================================================
// CONFIGURAÇÃO DE WI-FI (SUBSTITUA PELOS SEUS DADOS)
// ============================================================
const char* WIFI_SSID  = "IphoneBernardo";
const char* WIFI_SENHA = "Baer1234";

// ============================================================
// CONFIGURAÇÃO DOS PINOS DO ÁUDIO - ESP32-S3
// ============================================================
#define I2S_BCLK       16
#define I2S_LRC        17
#define I2S_DIN        18
#define I2S_SD         15
#define VOLUME_INICIAL 1.0f // Volume (de 0.05 a 1.0)

float volumeAtual = VOLUME_INICIAL;

// Protótipos de funções
void conectarWiFi();
void ligarAmplificador(AudioOutputI2S *out);
void desligarAmplificador(AudioOutputI2S *out);
void falarGoogleTTS(String texto);
void processarComando(String entrada);
void executarTesteAutomatico();
void imprimirAjuda();
String codificarURL(String str);

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("  SIGN TALK - VOZ NATURAL COM GOOGLE TRADUTOR TTS (WI-FI)");
  Serial.println("==========================================================");
  Serial.printf("Pinos Áudio: BCLK=%d | LRC=%d | DIN=%d | SD=%d | GAIN=GND\n", I2S_BCLK, I2S_LRC, I2S_DIN, I2S_SD);
  Serial.println("==========================================================\n");

  // Configura o pino SD em modo LOW (Mute/Dormência no hardware)
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, LOW);

  // Conecta ao Wi-Fi
  conectarWiFi();

  // Teste inicial de boas-vindas com voz natural
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("[INIT] Testando reprodução de áudio inicial...");
    falarGoogleTTS("Sistema Sign Talk iniciado. Conectado e pronto para falar.");
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
// CONEXÃO WI-FI
// ============================================================
void conectarWiFi() {
  if (String(WIFI_SSID) == "SEU_WIFI_AQUI") {
    Serial.println("[AVISO IMPORTANTE] Você precisa editar o arquivo e colocar o nome");
    Serial.println("e a senha do seu Wi-Fi nas variáveis WIFI_SSID e WIFI_SENHA!");
    Serial.println("O sistema não conseguirá buscar a voz online sem internet.\n");
    return;
  }

  Serial.printf("[WI-FI] Conectando à rede: \"%s\"...", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_SENHA);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WI-FI] Conectado com sucesso!");
    Serial.printf("[WI-FI] Endereço IP atribuído: %s\n\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[ERRO] Não foi possível conectar ao Wi-Fi. Verifique o nome e a senha.\n");
  }
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
// REPRODUÇÃO DE VOZ NATURAL VIA GOOGLE TRADUTOR TTS
// ============================================================
void falarGoogleTTS(String texto) {
  if (texto.length() == 0) return;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERRO] Wi-Fi desconectado! Conecte-se à internet para usar o Google TTS.");
    Serial.println("Dica: Verifique as variáveis WIFI_SSID e WIFI_SENHA no topo do código.");
    return;
  }

  Serial.println("--------------------------------------------------");
  Serial.printf("[GOOGLE TTS] Solicitando fala em português: \"%s\"\n", texto.c_str());

  // Constrói a URL do Google Tradutor TTS (API pública sem chave)
  String url = "http://translate.google.com/translate_tts?ie=UTF-8&client=tw-ob&tl=pt-BR&q=" + codificarURL(texto);
  
  HTTPClient http;
  http.begin(url);
  http.addHeader("User-Agent", "Mozilla/5.0"); // Identificador para evitar bloqueio

  Serial.println("[GOOGLE TTS] Baixando áudio MP3 da nuvem...");
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    int tamanhoInformado = http.getSize();
    Serial.printf("[GOOGLE TTS] Conexão estabelecida (Tamanho informado: %d bytes). Baixando stream MP3...\n", tamanhoInformado);
    
    // Define o tamanho máximo do buffer (120 KB é suficiente para 30 segundos de fala contínua na RAM do ESP32-S3)
    int maxBuffer = 120000;
    if (tamanhoInformado > 0 && tamanhoInformado < maxBuffer) {
      maxBuffer = tamanhoInformado;
    }

    uint8_t *bufferAudio = (uint8_t*)malloc(maxBuffer);
    if (bufferAudio != NULL) {
      WiFiClient *stream = http.getStreamPtr();
      int totalLido = 0;
      unsigned long tempoUltimoDado = millis();
      
      // Lê o stream (suporta Content-Length fixo ou Transfer-Encoding: chunked onde getSize() é -1)
      while ((http.connected() || stream->available() > 0) && totalLido < maxBuffer) {
        int disponivel = stream->available();
        if (disponivel > 0) {
          int lerAgora = min(disponivel, maxBuffer - totalLido);
          int lidos = stream->readBytes(&bufferAudio[totalLido], lerAgora);
          if (lidos > 0) {
            totalLido += lidos;
            tempoUltimoDado = millis();
          }
        } else {
          delay(2);
        }
        // Para se a conexão fechar sem dados no buffer, ou por timeout sem receber novos pacotes (1.5s)
        if ((!http.connected() && stream->available() == 0) || (millis() - tempoUltimoDado > 1500)) {
          break;
        }
      }

      Serial.printf("[GOOGLE TTS] Download concluído! %d bytes lidos para a RAM. Reproduzindo no alto-falante...\n", totalLido);

      if (totalLido > 100) { // Verifica se baixou um arquivo MP3 válido (> 100 bytes)
        AudioFileSourcePROGMEM *fonte = new AudioFileSourcePROGMEM(bufferAudio, totalLido);
        AudioGeneratorMP3 *mp3 = new AudioGeneratorMP3();
        AudioOutputI2S *saidaI2S = new AudioOutputI2S();
        
        saidaI2S->SetPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
        saidaI2S->SetGain(0.0f);
        
        ligarAmplificador(saidaI2S);
        mp3->begin(fonte, saidaI2S);
        
        // Loop de reprodução do MP3
        while (mp3->isRunning()) {
          if (!mp3->loop()) mp3->stop();
        }
        
        // Finaliza e muta o áudio
        mp3->stop();
        desligarAmplificador(saidaI2S);
        saidaI2S->stop();
        
        // Limpa toda a memória (previne vazamento de canais I2S)
        delete mp3;
        delete fonte;
        delete saidaI2S;
        Serial.println("[GOOGLE TTS] Reprodução concluída! Alto-falante em silêncio (Mute).");
      } else {
        Serial.println("[ERRO] Áudio recebido é muito pequeno ou corrompido.");
      }
      free(bufferAudio);
    } else {
      Serial.println("[ERRO] Memória RAM insuficiente para o buffer do áudio MP3!");
    }
  } else {
    Serial.printf("[ERRO] Falha ao buscar áudio no Google. Código HTTP: %d\n", httpCode);
  }
  
  http.end();
  Serial.println("--------------------------------------------------\n-> Digite outra frase em português e pressione ENTER:");
}

// ============================================================
// PROCESSADOR DE COMANDOS
// ============================================================
void processarComando(String entrada) {
  String comando = entrada;
  comando.toUpperCase();

  if (comando == "/TESTE" || comando == "TESTE") {
    executarTesteAutomatico();
  }
  else if (comando == "/AJUDA" || comando == "AJUDA" || comando == "HELP") {
    imprimirAjuda();
  }
  else if (comando.startsWith("/VOL") || comando.startsWith("VOL")) {
    int posEspaco = entrada.indexOf(' ');
    if (posEspaco != -1) {
      float novoVol = entrada.substring(posEspaco + 1).toFloat();
      if (novoVol >= 0.05f && novoVol <= 1.0f) {
        volumeAtual = novoVol;
        Serial.printf("\n[VOLUME] Volume ajustado para: %.2f\n\n", volumeAtual);
        falarGoogleTTS("Volume alterado com sucesso");
        return;
      }
    }
    Serial.println("\n[AVISO] Uso correto: /VOL 0.4 (valores entre 0.05 e 1.0)\n");
  }
  else {
    // Qualquer outro texto é enviado para o Google falar
    falarGoogleTTS(entrada);
  }
}

// ============================================================
// CODIFICADOR DE URL (PARA ACENTOS E ESPAÇOS EM PORTUGUÊS)
// ============================================================
String codificarURL(String str) {
  String codificada = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      codificada += '+';
    } else if (isalnum(c)) {
      codificada += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      codificada += '%';
      codificada += code0;
      codificada += code1;
    }
  }
  return codificada;
}

// ============================================================
// TESTE AUTOMÁTICO (/TESTE)
// ============================================================
void executarTesteAutomatico() {
  Serial.println("\n[TESTE AUTOMÁTICO] Iniciando sequência de fala natural...");
  delay(500);

  falarGoogleTTS("Olá! Eu sou o assistente Sign Talk, traduzindo gestos em Libras para voz em tempo real.");
  delay(1000);

  falarGoogleTTS("Obrigado por utilizar o nosso sistema de acessibilidade.");
  delay(1000);

  Serial.println("[TESTE AUTOMÁTICO] Concluído!\n");
}

// ============================================================
// MENU DE INSTRUÇÕES
// ============================================================
void imprimirAjuda() {
  Serial.println("==========================================================");
  Serial.println("            INSTRUÇÕES DO GOOGLE TRADUTOR TTS:");
  Serial.println("==========================================================");
  Serial.println("1. Digite QUALQUER FRASE em português e pressione ENTER.");
  Serial.println("2. O ESP32 baixará o áudio oficial do Google e falará");
  Serial.println("   com voz humana 100% natural no seu alto-falante!");
  Serial.println();
  Serial.println("Comandos Especiais:");
  Serial.println("  /TESTE     ➔ Fala frases completas demonstrativas");
  Serial.println("  /VOL 0.4   ➔ Ajusta o volume de fala (entre 0.05 e 1.0)");
  Serial.println("  /AJUDA     ➔ Exibe este menu novamente");
  Serial.println("==========================================================\n");
  Serial.println("-> Digite algo e pressione ENTER:");
}
