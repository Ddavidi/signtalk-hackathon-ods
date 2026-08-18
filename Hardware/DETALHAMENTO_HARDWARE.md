# Detalhamento de Hardware - SignTalk

Este documento descreve detalhadamente a arquitetura física, as conexões de pinagem e a lista completa de materiais (Bill of Materials - BOM) necessários para reproduzir o projeto SignTalk, contemplando tanto a luva transmissora quanto a base receptora.

---

## 1. Bill of Materials (BOM)

Abaixo está a lista completa de componentes, módulos e materiais utilizados na construção dos protótipos físicos:

| Componente | Quantidade | Aplicação | Descrição / Modelo |
| :--- | :---: | :--- | :--- |
| **Microcontrolador (Base)** | 1 | Base Receptora | Placa de desenvolvimento ESP32-S3 |
| **Microcontrolador (Nó)** | 2 | Luvas (Mão Direita/Esquerda) | Placa ESP32-C3 Super Mini |
| **Sensor Flexível** | 10 | Luvas | Sensores flexíveis A12E-10X (mede a flexão dos dedos) |
| **IMU (Acelerômetro/Giroscópio)**| 2 | Luvas | Módulo MPU-6050 (I2C) para orientação da mão |
| **Amplificador de Áudio I2S** | 1 | Base Receptora | Módulo MAX98357A (Decodificador e amplificador) |
| **Alto-falante** | 1 | Base Receptora | Alto-falante de 3W 4Ohm (Modelo PK230007O00) |
| **Display** | 1 | Base Receptora | LCD 16x2 com módulo I2C (Backlight azul) |
| **Bateria Lipo** | 3 | Alimentação Geral | Baterias de Polímero de Lítio (Li-Po) 3.7V |
| **Resistores (Divisor de Tensão)**| 10 | Luvas | Resistores de 10kΩ ou valor adequado (Through-hole) |
| **Estrutura Física** | - | Impressão 3D | Filamento PLA/PETG ou Resina para os cases |
| **Tecido / Vestimenta** | 2 | Luvas | Luvas confortáveis de tecido flexível (lycra ou algodão) |
| **Cabos e Conectores** | Vários | Conexão Geral | Jumpers, fios flexíveis finos (AWG 30) e solda |

---

## 2. Circuitos e Pinagem (Esquemático Lógico)

### 2.1 Luva Transmissora (Nó - ESP32-C3)
A luva precisa ler continuamente a inclinação do pulso e o grau de contração dos 5 dedos. 

* **Sensores de Flexão (Divisor de Tensão):** 
  Os sensores funcionam como resistores variáveis que são conectados em pull down em portas analogicas do ESP32-C3. Utilizamos divisores de tensão (com resistores fixos) conectados às portas analógicas do ESP32-C3 para ler a variação de voltagem.
  * *Pinos ADC:* Conectados do Pino Analógico 0 ao 4 (dependendo da pinagem disponível no Super Mini).
* **Orientação (MPU-6050 via I2C):**
  * `VCC` -> 3.3V
  * `GND` -> GND
  * `SDA` -> Pino SDA do ESP32-C3
  * `SCL` -> Pino SCL do ESP32-C3

### 2.2 Base Receptora (ESP32-S3)
A base fica responsável pelo processamento pesado (TinyML), exibição do texto e reprodução da voz sintetizada.

* **Display LCD 16x2 (via I2C):**
  * `VCC` -> 5V (ou 3.3V se o LCD suportar)
  * `GND` -> GND
  * `SDA` -> Pino SDA do ESP32-S3
  * `SCL` -> Pino SCL do ESP32-S3
* **Áudio Digital (MAX98357A via I2S):**
  * `VIN` -> 5V
  * `GND` -> GND
  * `BCLK` (Bit Clock) -> Pino I2S BCLK configurado
  * `LRC` (Left/Right Clock ou Word Select) -> Pino I2S WS configurado
  * `DIN` (Data In) -> Pino I2S DOUT configurado
  * Os pinos de saída do amplificador ligam-se aos polos positivo e negativo do alto-falante.

---

## 3. Modelagem 3D e Invólucro (Cases)

Para proteção dos componentes, ergonomia e acabamento estético, modelagens 3D em formato `.STL` foram desenvolvidas.

* **Case da Mão (Luva):**
  Um pequeno case de superfície curva desenvolvido para ser fixado nas costas da mão (costurado ou fixado por velcro na luva). Seu objetivo é abrigar a bateria Lipo e a placa perfurada onde estão o ESP32-C3 e o MPU-6050, minimizando o impacto no movimento dos dedos.
* **Case Central (Base):**
  Uma caixa com tampa dedicada para abrigar o ESP32-S3 e sua bateria. A estrutura conta com furação frontal sob medida para o encaixe do display LCD 16x2 e saídas acústicas laterais otimizadas para o alto-falante 3W acoplado ao MAX98357A.
