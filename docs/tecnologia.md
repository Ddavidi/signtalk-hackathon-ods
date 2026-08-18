# Detalhamento Tecnológico

## Visão Geral da Arquitetura

O SignTalk utiliza uma arquitetura distribuída de dois módulos independentes, priorizando ergonomia (luva leve) e poder de processamento (base com IA embarcada).

```
┌──────────────────────┐                    ┌──────────────────────────┐
│   LUVA TRANSMISSORA  │     ESP-NOW        │     BASE RECEPTORA       │
│                      │   (2.4 GHz)        │                          │
│  ESP32-C3 Super Mini │ ─────────────────► │  ESP32-S3                │
│  5x Sensor Flex      │   Sem roteador     │  Modelo TinyML           │
│  1x MPU-6050         │   Latência < 5ms   │  LCD 16x2 I2C            │
│  Bateria 3.7V        │                    │  MAX98357A + Speaker 3W  │
│                      │                    │  Bateria 3.7V            │
└──────────────────────┘                    └──────────────────────────┘
```

---

## Hardware

### Lista Completa de Componentes (BOM)

| Componente | Qtd | Função |
|---|:---:|---|
| ESP32-S3 | 1 | Microcontrolador da base receptora (processamento TinyML) |
| ESP32-C3 Super Mini | 1 | Microcontrolador da luva (coleta e transmissão) |
| Sensor Flexível A12E-10X | 5 | Mede a curvatura de cada dedo |
| MPU-6050 (GY-521) | 1 | Acelerômetro + Giroscópio 6 eixos (orientação da mão) |
| Amplificador MAX98357A | 1 | Decodificador e amplificador de áudio I2S |
| Alto-falante 3W 4Ω | 1 | Reprodução da tradução em áudio |
| Display LCD 16x2 I2C | 1 | Exibição da tradução em texto |
| Baterias Li-Po 3.7V | 2 | Alimentação portátil (luva + base) |
| Resistores 10kΩ | 5 | Divisores de tensão para os sensores flex |

### Luva Transmissora

#### Sensores de Flexão
Os sensores funcionam como resistores variáveis. Quando o dedo se curva, a resistência do sensor muda, alterando a tensão lida pela porta analógica (ADC) do ESP32-C3. Cada sensor está conectado em um divisor de tensão com um resistor fixo de 10kΩ.

- **Pinos ADC**: GPIO 0, 1, 2, 3 e 4
- **Resolução**: 12 bits (0 a 4095)
- **Taxa de leitura**: Contínua, a cada ciclo do loop principal

#### Sensor Inercial (MPU-6050)
O módulo GY-521 fornece dados de 6 eixos (3 de acelerômetro + 3 de giroscópio) via barramento I2C. É essencial para detectar:
- A inclinação e rotação da mão (diferencia gestos com mesma posição dos dedos)
- Letras dinâmicas como **J** e **Z**, que envolvem movimento no espaço

- **Conexão**: I2C (SDA: GPIO 8, SCL: GPIO 9)

### Base Receptora

#### Processamento (ESP32-S3)
O ESP32-S3 foi escolhido por seu poder computacional superior, necessário para rodar o modelo de rede neural. Ele recebe os pacotes via ESP-NOW, alimenta o modelo TinyML e gera as saídas.

#### Display LCD 16x2
Conectado via I2C, exibe a letra ou palavra identificada pela IA. Permite que o interlocutor leia a tradução visualmente.

#### Sistema de Áudio
O módulo MAX98357A recebe o sinal digital via barramento I2S e o converte/amplifica para o alto-falante de 3W. O sistema utiliza Text-to-Speech para verbalizar a tradução, permitindo que ouvintes compreendam o gesto sem precisar olhar para o display.

---

## Software

### Inteligência Artificial (TinyML)

O modelo de IA foi treinado utilizando o **Edge Impulse**, plataforma especializada em machine learning para dispositivos embarcados.

**Pipeline de treinamento:**
1. **Coleta de dados**: Captura de múltiplas amostras de cada gesto via sensores
2. **Processamento de sinais**: Extração de features relevantes dos dados brutos
3. **Treinamento**: Rede neural classificadora treinada na nuvem do Edge Impulse
4. **Exportação**: Modelo otimizado exportado como biblioteca C++ para o ESP32-S3
5. **Inferência local**: O modelo roda no microcontrolador, classificando gestos em milissegundos

**Dados de entrada do modelo (por amostra):**
- 5 valores de flexão (um por dedo)
- 6 valores inerciais (acelerômetro XYZ + giroscópio XYZ)
- Total: 11 features por janela de tempo

### Comunicação ESP-NOW

O ESP-NOW é um protocolo da Espressif que permite comunicação direta entre dispositivos ESP32 sem necessidade de roteador Wi-Fi.

**Características:**
- Frequência: 2.4 GHz
- Latência: Tipicamente inferior a 5 milissegundos
- Alcance: Até 200 metros em linha de visão
- Segurança: Suporte a criptografia CCMP
- Pareamento: Automático via endereço MAC

### Calibração Automática

O sistema inclui uma rotina de calibração que adapta os sensores ao biotipo de cada usuário:
1. O usuário abre completamente a mão (registra valores máximos dos sensores)
2. O usuário fecha completamente o punho (registra valores mínimos)
3. O firmware normaliza as leituras de 0 a 100 baseado nesses extremos
4. A calibração elimina variações causadas por diferentes tamanhos de mão

---

## Modelagem 3D

Cases em formato STL foram desenvolvidos para proteger os componentes e garantir ergonomia:

- **Case da Luva** (`Luva.stl`): Alojamento curvo para o dorso da mão, abriga o ESP32-C3, MPU-6050 e bateria
- **Case Central** (`Central.stl`): Caixa com tampa para a base receptora, com encaixe para LCD e saída acústica para o alto-falante

Ambos os modelos estão disponíveis na pasta `/hardware` para impressão em PETG ou TPU.
