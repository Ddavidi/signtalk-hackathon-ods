/*
 * ============================================================================
 *  SIGN TALK - TESTE DO DISPLAY LCD 16x2 E ÁUDIO I2S (MAX98357A) VIA SERIAL
 *  Placa: ESP32-S3
 *  Pasta: Codigo Antigo/testes/teste_display_speaker
 * ============================================================================
 *
 *  OBJETIVO:
 *    Testar o funcionamento integrado e as bibliotecas do Display LCD 16x2 I2C
 *    e do amplificador de áudio I2S MAX98357A (com sintetizador de voz TTS SAM).
 *    O usuário pode digitar qualquer LETRA ou PALAVRA no Monitor Serial para que
 *    seja exibida na tela do LCD e falada através do alto-falante.
 *
 *  ============================================================================
 *  BIBLIOTECAS NECESSÁRIAS (Instalar via Arduino Library Manager ou ZIP):
 *  ============================================================================
 *    1. LiquidCrystal I2C (por Frank de Brabander - versão 1.1.4 ou superior)
 *    2. ESP8266Audio      (por Earle F. Philhower) -> Para controle do I2S
 *    3. ESP8266SAM        (por Earle F. Philhower) -> Sintetizador de voz TTS
 *
 *  ============================================================================
 *  ESQUEMA DE LIGAÇÃO DOS PINOS NO ESP32-S3:
 *  ============================================================================
 *
 *  📌 1. DISPLAY LCD 16x2 I2C (Módulo PCF8574):
 *     - GND  ➔ GND (ESP32-S3)
 *     - VCC  ➔ 5V / VIN / VBUS (A maioria dos displays precisa de 5V para contraste)
 *     - SDA  ➔ GPIO 8  (Pino I2C SDA padrão)
 *     - SCL  ➔ GPIO 9  (Pino I2C SCL padrão)
 *     * Obs: Endereço I2C comum é 0x27 ou 0x3F (o comando /SCAN verifica automaticamente).
 *
 *  📌 2. AMPLIFICADOR ÁUDIO I2S MAX98357A + ALTO-FALANTE:
 *     - GND  ➔ GND (ESP32-S3)
 *     - VIN  ➔ 5V / VIN (Ou 3.3V, mas 5V entrega maior potência e clareza)
 *     - BCLK ➔ GPIO 16 (Bit Clock / SCK)
 *     - LRC  ➔ GPIO 17 (Word Select / WS / Left-Right Clock)
 *     - DIN  ➔ GPIO 18 (Data In / SD / SDIN)
 *     - GAIN ➔ GND (Ligado ao GND para fixar ganho em 6dB - economia e sem travamento)
 *     - SD   ➔ GPIO 15 (Controlado pelo ESP32 para silêncio 100% no modo espera)
 *     - Saídas (+ e -) ➔ Alto-falante (ex: 4 Ohms / 8 Ohms - 2W/3W)
 *
 *  ============================================================================
 *  COMO USAR NO MONITOR SERIAL (115200 baud):
 *  ============================================================================
 *    - Digite uma letra (ex: "A", "B", "C") ➔ Exibe a letra no LCD e fala em português.
 *    - Digite uma palavra (ex: "OLA", "BOM DIA") ➔ Exibe e fala a frase completa.
 *    - Comandos Especiais:
 *        "/TESTE"  ➔ Executa rotina de testes automáticos (pisca LCD e testa áudio).
 *        "/SCAN"   ➔ Escanea o barramento I2C e mostra o endereço do seu LCD.
 *        "/AJUDA"  ➔ Exibe o menu de instruções no Monitor Serial.
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <AudioOutputI2S.h>
#include <ESP8266SAM.h>

// ============================================================
// CONFIGURAÇÃO DOS PINOS - ESP32-S3
// ============================================================
// 1. Display LCD 16x2 I2C
#define LCD_ENDERECO 0x27  // Endereço comum do PCF8574 (use /SCAN para confirmar)
#define LCD_COLUNAS  16
#define LCD_LINHAS   2
#define LCD_SDA      8     // GPIO 8 para SDA
#define LCD_SCL      9     // GPIO 9 para SCL

// 2. Amplificador de Áudio I2S MAX98357A
#define I2S_BCLK     16    // Bit Clock (BCLK / SCK)
#define I2S_LRC      17    // Word Select (LRC / WS)
#define I2S_DIN      18    // Data In (DIN / SDIN / SD)
#define I2S_SD       15    // Pino SD (Shutdown/Mute) - Ligado no GPIO 15 para silêncio absoluto quando inativo
#define VOLUME_AUDIO 0.25f // Volume reduzido para evitar picos de corrente (Brownout)

// Instâncias dos objetos de tela e áudio
LiquidCrystal_I2C lcd(LCD_ENDERECO, LCD_COLUNAS, LCD_LINHAS);
AudioOutputI2S *audioOutput = NULL;
ESP8266SAM *vozSAM = NULL;

// Protótipos de funções
void exibirNoLCD(String titulo, String texto);
void realizarScanI2C();
void imprimirAjuda();
void executarTesteAutomatico();
void ligarAmplificador();
void desligarAmplificador();
void processarETestarTexto(String texto);
String obterFonetica(String texto);

// ============================================================
// SETUP - INICIALIZAÇÃO
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("  SIGN TALK - TESTE DE DISPLAY LCD E ÁUDIO I2S (MAX98357A)");
  Serial.println("==========================================================");
  Serial.println("Pinos configurados para o ESP32-S3:");
  Serial.printf("  [LCD I2C]       SDA = GPIO %d | SCL = GPIO %d | VCC = 3.3V\n", LCD_SDA, LCD_SCL);
  Serial.printf("  [MAX98357A I2S] BCLK = %d | LRC = %d | DIN = %d | SD = %d | GAIN = GND\n", I2S_BCLK, I2S_LRC, I2S_DIN, I2S_SD);
  Serial.println("==========================================================\n");

  // 1. Inicializa barramento I2C e Display LCD
  Serial.println("[INIT] Inicializando barramento I2C e Display LCD 16x2...");
  Wire.begin(LCD_SDA, LCD_SCL);
  lcd.init();
  lcd.backlight();
  exibirNoLCD(" SIGN TALK AI ", "Iniciando teste.");

  // Realiza escaneamento I2C para auxiliar na verificação de conexões
  realizarScanI2C();

  // 2. Inicializa o Áudio I2S e Sintetizador SAM
  Serial.println("[INIT] Inicializando Amplificador I2S MAX98357A e Sintetizador SAM...");
  pinMode(I2S_SD, OUTPUT);
  digitalWrite(I2S_SD, HIGH); // Ativa o amplificador
  audioOutput = new AudioOutputI2S();
  audioOutput->SetPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
  audioOutput->SetGain(VOLUME_AUDIO);
  vozSAM = new ESP8266SAM();

  // 3. Teste sonoro inicial de boas-vindas
  Serial.println("[INIT] Reproduzindo som de boas-vindas...");
  exibirNoLCD(" SIGN TALK AI ", "Sistema Pronto!");
  if (audioOutput != NULL && vozSAM != NULL) {
    vozSAM->Say(audioOutput, "Sign Talk");
  }
  // Coloca o amplificador em modo mudo/soneca ao terminar as boas-vindas
  if (audioOutput != NULL) audioOutput->SetGain(0.0f);
  digitalWrite(I2S_SD, LOW);

  // 4. Exibe instruções no Monitor Serial
  imprimirAjuda();

  exibirNoLCD(" SIGN TALK AI ", "Aguardando serial");
}

// ============================================================
// LOOP PRINCIPAL - LEITURA DO MONITOR SERIAL
// ============================================================
void loop() {
  if (Serial.available() > 0) {
    String entrada = Serial.readStringUntil('\n');
    entrada.trim();

    if (entrada.length() > 0) {
      String comando = entrada;
      comando.toUpperCase();

      // Verifica comandos especiais
      if (comando == "/TESTE" || comando == "TESTE") {
        executarTesteAutomatico();
      } 
      else if (comando == "/SCAN" || comando == "SCAN" || comando == "I2C") {
        realizarScanI2C();
      } 
      else if (comando == "/AJUDA" || comando == "AJUDA" || comando == "HELP" || comando == "/HELP") {
        imprimirAjuda();
      } 
      else {
        // Processa o texto digitado (Letra ou Palavra)
        processarETestarTexto(entrada);
      }
    }
  }
  delay(20);
}

// ============================================================
// FUNÇÃO PARA ATUALIZAR DISPLAY LCD
// ============================================================
void exibirNoLCD(String titulo, String texto) {
  lcd.clear();
  
  if (texto.length() <= 16) {
    // Exibe o título na linha superior centralizado
    lcd.setCursor(0, 0);
    int espacosTitulo = (16 - titulo.length()) / 2;
    for (int i = 0; i < espacosTitulo; i++) lcd.print(" ");
    lcd.print(titulo);

    // Formata e exibe o texto na linha inferior centralizado
    lcd.setCursor(0, 1);
    String exibicao = texto;
    if (texto.length() <= 12 && titulo == "Texto Recebido:") {
      exibicao = "[ " + texto + " ]";
    }
    int espacosTexto = (16 - exibicao.length()) / 2;
    for (int i = 0; i < espacosTexto; i++) lcd.print(" ");
    lcd.print(exibicao);
  } else {
    // Se o texto for longo (> 16 chars), divide em duas linhas (até 32 caracteres)
    lcd.setCursor(0, 0);
    lcd.print(texto.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(texto.substring(16, min((int)texto.length(), 32)));
  }
}

// ============================================================
// CONTROLE DE MUTE / SILENCIAMENTO DO AMPLIFICADOR
// ============================================================
void ligarAmplificador() {
  digitalWrite(I2S_SD, HIGH); // Liga o chip no hardware
  if (audioOutput != NULL) audioOutput->SetGain(VOLUME_AUDIO); // Desmuta no software
  delay(15); // Pausa curta para o capacitor interno do MAX98357A carregar sem estalo
}

void desligarAmplificador() {
  if (audioOutput != NULL) audioOutput->SetGain(0.0f); // Muta no software
  digitalWrite(I2S_SD, LOW); // Corta a energia do amplificador no hardware (modo 0.6 uA)
}

// ============================================================
// PROCESSAMENTO DE LETRA OU PALAVRA (LCD + ÁUDIO)
// ============================================================
void processarETestarTexto(String texto) {
  Serial.println("--------------------------------------------------");
  Serial.printf("[RECEBIDO] Texto original: \"%s\"\n", texto.c_str());

  // 1. Exibe o texto recebido no Display LCD
  exibirNoLCD("Texto Recebido:", texto);

  // 2. Obtém a pronúncia fonética adaptada para o sintetizador SAM
  String textoFalar = obterFonetica(texto);
  Serial.printf("[AUDIO] Pronúncia fonética: \"%s\"\n", textoFalar.c_str());
  Serial.println("[AUDIO] Reproduzindo no alto-falante via MAX98357A...");

  // 3. Reproduz no alto-falante via I2S
  if (audioOutput != NULL && vozSAM != NULL) {
    ligarAmplificador();
    vozSAM->Say(audioOutput, textoFalar.c_str());
    desligarAmplificador();
  } else {
    Serial.println("[ERRO] Sistema de áudio I2S não inicializado!");
  }

  Serial.println("[CONCLUÍDO] Fim da reprodução.");
  Serial.println("--------------------------------------------------");
  Serial.println("-> Digite outra LETRA ou PALAVRA e pressione ENTER:\n");

  // 4. Aguarda 1 segundo e retorna à tela de espera no LCD
  delay(1000);
  exibirNoLCD(" SIGN TALK AI ", "Aguardando serial");
}

// ============================================================
// MAPEAMENTO FONÉTICO PARA VOZ EM PORTUGUÊS
// ============================================================
// O sintetizador ESP8266SAM foi treinado em inglês.
// Esta função converte letras avulsas e palavras comuns em Libras
// para representações fonéticas que soam naturais em português.
String obterFonetica(String texto) {
  String t = texto;
  t.trim();
  t.toUpperCase();

  // Alfabeto (Letras avulsas em Libras):
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

  // Palavras comuns em Libras / Sign Talk:
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

  // Se for uma palavra ou frase não cadastrada, retorna o próprio texto
  return texto;
}

// ============================================================
// ESCANEAMENTO DO BARRAMENTO I2C (/SCAN)
// ============================================================
void realizarScanI2C() {
  Serial.println("\n[I2C SCANNER] Escaneando barramento I2C (SDA = GPIO 8, SCL = GPIO 9)...");
  byte erro, endereco;
  int dispositivosEncontrados = 0;

  for (endereco = 1; endereco < 127; endereco++) {
    Wire.beginTransmission(endereco);
    erro = Wire.endTransmission();

    if (erro == 0) {
      Serial.printf("  -> Dispositivo I2C encontrado no endereço 0x%02X", endereco);
      if (endereco == 0x27 || endereco == 0x3F) {
        Serial.print(" (Possível Display LCD PCF8574!)");
      }
      Serial.println();
      dispositivosEncontrados++;
    } else if (erro == 4) {
      Serial.printf("  -> Erro desconhecido no endereço 0x%02X\n", endereco);
    }
  }

  if (dispositivosEncontrados == 0) {
    Serial.println("  [AVISO] Nenhum dispositivo I2C foi encontrado!");
    Serial.println("  Verifique as ligações: GND, 5V/3.3V, SDA (GPIO 8) e SCL (GPIO 9).");
    exibirNoLCD("Erro I2C LCD", "Nenhum no barram.");
  } else {
    Serial.printf("[I2C SCANNER] Concluído. %d dispositivo(s) encontrado(s).\n", dispositivosEncontrados);
  }
  Serial.println();
}

// ============================================================
// TESTE AUTOMÁTICO COMPLETO (/TESTE)
// ============================================================
void executarTesteAutomatico() {
  Serial.println("\n[TESTE AUTOMÁTICO] Iniciando rotina de testes do LCD e MAX98357A...");

  // Teste 1: Backlight do LCD
  Serial.println("  -> 1. Testando Display LCD (Piscando Backlight)...");
  exibirNoLCD("Teste de Tela", "Piscando Luz...");
  delay(500);
  lcd.noBacklight();
  delay(500);
  lcd.backlight();
  delay(500);

  // Teste 2: Preenchimento de Tela
  Serial.println("  -> 2. Testando alinhamento e caracteres do LCD...");
  exibirNoLCD("Colunas 1 a 16", "1234567890123456");
  delay(1500);

  // Teste 3: Áudio (Sintetizador SAM + I2S)
  Serial.println("  -> 3. Testando síntese de voz TTS no alto-falante...");
  exibirNoLCD("Teste de Audio", "Falando: Teste");
  if (audioOutput != NULL && vozSAM != NULL) {
    ligarAmplificador();
    vozSAM->Say(audioOutput, "Tes chee");
    desligarAmplificador();
  }
  delay(600);

  exibirNoLCD("Teste de Audio", "Falando: Libras");
  if (audioOutput != NULL && vozSAM != NULL) {
    ligarAmplificador();
    vozSAM->Say(audioOutput, "Lee bras");
    desligarAmplificador();
  }
  delay(600);

  Serial.println("[TESTE AUTOMÁTICO] Concluído com sucesso!\n");
  Serial.println("-> Digite uma LETRA ou PALAVRA para continuar testando:\n");
  exibirNoLCD(" SIGN TALK AI ", "Aguardando serial");
}

// ============================================================
// IMPRESSÃO DE INSTRUÇÕES NO MONITOR SERIAL
// ============================================================
void imprimirAjuda() {
  Serial.println("==========================================================");
  Serial.println("                   COMO USAR ESTE TESTE:");
  Serial.println("==========================================================");
  Serial.println("1. Digite qualquer LETRA (ex: A, B, C) no Monitor Serial");
  Serial.println("   (configurado a 115200 baud) e pressione ENTER.");
  Serial.println("2. Ou digite uma PALAVRA/FRASE (ex: OLA, BOM DIA, SIM).");
  Serial.println("3. O ESP32-S3 exibirá o texto na tela do LCD e falará a");
  Serial.println("   pronúncia correspondente pelo alto-falante MAX98357A!");
  Serial.println();
  Serial.println("Comandos Especiais:");
  Serial.println("  /TESTE  ➔ Executa teste automático (pisca LCD e testa voz)");
  Serial.println("  /SCAN   ➔ Escanea o barramento I2C e confirma endereço do LCD");
  Serial.println("  /AJUDA  ➔ Exibe este menu de instruções no Monitor Serial");
  Serial.println("==========================================================\n");
  Serial.println("-> Digite uma letra ou palavra e pressione ENTER:");
}
