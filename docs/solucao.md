# A Solução: SignTalk

## Visão Geral

O SignTalk é um dispositivo vestível (luva inteligente) que traduz gestos da Língua Brasileira de Sinais (Libras) para **texto e áudio em tempo real**. O sistema opera de forma completamente autônoma, sem necessidade de internet, servidores em nuvem ou smartphone pareado.

## Arquitetura Distribuída

O sistema é dividido em dois módulos independentes que se comunicam sem fio:

### Módulo 1: Luva Transmissora
A luva é o módulo vestível responsável por **capturar** os gestos do usuário.

**Componentes:**
- **ESP32-C3 Super Mini**: Microcontrolador de baixo consumo que coleta e transmite os dados
- **5 Sensores Flexíveis (A12E-10X)**: Um em cada dedo, medem o grau de curvatura/flexão
- **1 MPU-6050 (GY-521)**: Sensor inercial com acelerômetro e giroscópio de 6 eixos, captura a orientação e o movimento da mão (essencial para letras dinâmicas como J e Z)
- **Bateria de lítio 3.7V**: Alimentação portátil

**O que acontece:** Ao fazer um gesto em Libras, os sensores flex registram a posição de cada dedo enquanto o giroscópio detecta a inclinação e rotação da mão. Esses dados são empacotados e transmitidos instantaneamente para a base.

### Módulo 2: Base Receptora
A base é o "cérebro" do sistema, responsável por **processar** os dados e gerar a tradução.

**Componentes:**
- **ESP32-S3**: Microcontrolador com poder de processamento suficiente para rodar redes neurais
- **Modelo TinyML (Edge Impulse)**: Rede neural treinada para classificar os gestos de Libras
- **Display LCD 16x2 I2C**: Exibe a tradução em texto
- **MAX98357A + Alto-falante 3W**: Reproduz a tradução em áudio (Text-to-Speech)
- **Bateria de lítio 3.7V**: Alimentação portátil

**O que acontece:** A base recebe os dados dos sensores, alimenta a rede neural embarcada que identifica o gesto, e apresenta a tradução simultaneamente no display e no alto-falante.

### Comunicação: ESP-NOW
Os dois módulos se comunicam via **ESP-NOW**, um protocolo sem fio proprietário da Espressif que opera na frequência de 2.4 GHz. As vantagens:
- **Latência mínima**: Transmissão quase instantânea (milissegundos)
- **Sem roteador**: Não precisa de Wi-Fi, Bluetooth ou qualquer infraestrutura de rede
- **Conexão direta**: Pareamento automático entre os dispositivos
- **Baixo consumo**: Otimizado para dispositivos alimentados por bateria

## Funcionalidades Atuais

- ✅ Reconhecimento do **alfabeto completo de Libras** (A a Z)
- ✅ Suporte a **letras dinâmicas** (J e Z) via giroscópio
- ✅ Tradução em **texto** (display LCD) e **áudio** (alto-falante)
- ✅ Funcionamento **100% offline**
- ✅ **Calibração automática** por usuário (adapta-se a qualquer tamanho de mão)

## Dimensão Educacional

Além de traduzir, o SignTalk funciona como **ferramenta de ensino de Libras**:

- Estudantes vestem a luva e praticam os sinais
- O sistema verifica em tempo real se o gesto está correto
- Feedback instantâneo via display e áudio orienta o aprendizado
- Permite prática autônoma, sem necessidade de instrutor presente

Essa funcionalidade transforma o dispositivo em uma ponte de mão dupla: ajuda surdos a serem compreendidos e incentiva ouvintes a aprenderem Libras.

## Por que Funciona Offline

Uma decisão fundamental do projeto foi processar **tudo localmente**. O modelo de IA roda diretamente no microcontrolador ESP32-S3, sem enviar dados para a internet. Isso garante:

1. **Acessibilidade geográfica**: Funciona em qualquer lugar, mesmo sem cobertura de internet
2. **Privacidade**: Os dados dos gestos nunca saem do dispositivo
3. **Velocidade**: Sem latência de rede, a tradução é instantânea
4. **Custo zero de operação**: Sem assinaturas de serviço em nuvem
