# Guia de Utilização e Configuração da Luva SignTalk

Este guia detalha como preparar o seu ambiente de desenvolvimento (Arduino IDE) para compilar o código e como utilizar fisicamente a luva SignTalk e sua base receptora.

---

## 1. Preparando o Ambiente (Arduino IDE)

O projeto SignTalk utiliza microcontroladores da família ESP32 (ESP32-C3 na luva e ESP32-S3 na base). Para programá-los usando a Arduino IDE, é necessário instalar o suporte às placas da Espressif.

### 1.1. Instalando o Suporte ao ESP32
1. Abra a Arduino IDE.
2. Vá em **Arquivo > Preferências** (ou *File > Preferences*).
3. No campo **URLs Adicionais para Gerenciadores de Placas**, insira o seguinte link:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
4. Clique em **OK**.
5. Vá em **Ferramentas > Placa > Gerenciador de Placas** (*Tools > Board > Boards Manager*).
6. Pesquise por `esp32` e instale o pacote oficial da **Espressif Systems** (recomendamos a versão mais recente).

### 1.2. Selecionando as Placas Corretas
O projeto usa dois microcontroladores diferentes, então você deve trocar a placa na IDE dependendo do código que for gravar:
*   **Para a Luva (Transmissor):** Vá em *Ferramentas > Placa* e selecione **ESP32C3 Dev Module**.
*   **Para a Base (Receptor):** Vá em *Ferramentas > Placa* e selecione **ESP32S3 Dev Module**.

---

## 2. Instalando as Bibliotecas Necessárias

Para que o código compile, você precisa baixar algumas bibliotecas. A maioria pode ser instalada diretamente pelo Gerenciador de Bibliotecas da Arduino IDE (*Sketch > Incluir Biblioteca > Gerenciar Bibliotecas*).

### Bibliotecas de Sensores (Para o ESP32-C3)
1.  **Adafruit MPU6050:** Pesquise por `Adafruit MPU6050` e instale. Quando a IDE perguntar se deseja instalar as dependências (Adafruit Unified Sensor, Adafruit BusIO), clique em "Install All".
2.  **TCA9548A (Multiplexador I2C):** Caso esteja utilizando múltiplos MPU-6050 ou sensores I2C nativos. Pesquise por `Adafruit TCA9548A` e instale.

### Bibliotecas de Comunicação e Áudio (Para o ESP32-S3)
1.  **ESP-NOW:** A biblioteca ESP-NOW já vem embutida no pacote da Espressif que instalamos no passo 1.1, você só precisa importá-la no código (`#include <esp_now.h>`).
2.  **ESP8266Audio:** Uma excelente biblioteca para decodificar arquivos de som e enviar via I2S para o MAX98357A. Pesquise por `ESP8266Audio` (do autor Earle F. Philhower) no gerenciador e instale.

### Instalação da Inteligência Artificial (Edge Impulse)
Quando você finaliza o treinamento no Edge Impulse (conforme o Guia de Treinamento), ele exporta um arquivo `.zip`.
1. Baixe o `.zip` do seu modelo no Edge Impulse.
2. Na Arduino IDE, vá em **Sketch > Incluir Biblioteca > Adicionar Biblioteca .ZIP** (*Add .ZIP Library*).
3. Selecione o arquivo baixado. Isso instalará o modelo de ML treinado diretamente no seu ambiente.

---

## 3. Configuração do Firmware (Atenção aos MAC Addresses)

A comunicação via ESP-NOW exige que o transmissor (Luva) saiba qual é o endereço MAC do receptor (Base).
1.  Primeiro, conecte a Base (ESP32-S3) e rode um código simples para descobrir o MAC Address (`WiFi.macAddress()`). Anote esse endereço (ex: `FF:EE:DD:CC:BB:AA`).
2.  No código da Luva (ESP32-C3), encontre a variável de *broadcast address* (geralmente um array de 6 bytes) e substitua pelo MAC Address que você anotou.
3.  Compile e faça o upload para as respectivas placas.

---

## 4. Passo a Passo de Utilização da Luva

Com os dois códigos gravados e as baterias carregadas, siga os passos para uso:

1.  **Vista a luva corretamente:**
    *   Insira a mão com cuidado para não tensionar demasiadamente os fios de rede (já que são rígidos e podem se romper).
    *   Ajuste as pontas dos dedos para que os sensores de flexão fiquem exatamente sobre as suas articulações.
2.  **Ligue a Base Receptora (ESP32-S3):**
    *   Ligue a base primeiro. Ela irá inicializar o barramento I2S, o amplificador MAX98357A e o módulo ESP-NOW, aguardando conexões.
3.  **Ligue a Luva (ESP32-C3):**
    *   Ligue a luva com a mão aberta e em posição de repouso por cerca de 2 segundos. O código deve executar a calibração inicial dos limites de flexão neste momento.
4.  **Sinalização:**
    *   Faça o gesto desejado e segure por um breve instante (meio segundo) para que a janela temporal do Edge Impulse capture o movimento completo e estabilizado.
    *   A Base irá reproduzir o áudio pelo alto-falante indicando a letra ou palavra reconhecida!

---

## 5. Solução de Problemas (Troubleshooting)

*   **A Luva parou de enviar dados quando fecho a mão (Problema antigo):** Verifique a integridade dos resistores no circuito *Pull-down*. Como mencionado nos relatórios, os fios rígidos podem se romper dentro da solda se houver muita movimentação.
*   **O som sai chiado ou não sai:** Verifique a tensão da bateria da base. Amplificadores como o MAX98357A puxam bastante corrente em picos. Se a bateria não estiver dando conta, o ESP32 pode reiniciar ou o som pode falhar. Verifique também os pinos de conexão BCLK, LRC e DIN do I2S.
*   **Base não recebe os sinais:** Confirme se o MAC Address da base que está inserido no código da luva está 100% correto. Se você trocou o ESP32-S3 por outro, o MAC Address mudou e você precisará atualizar o código da luva.
