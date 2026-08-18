# Projeto SignTalk: Luva Tradutora de Libras com TinyML

**Instituição:** PUC Minas - Engenharia de Computação (Sistemas Embarcados)  
**Equipe:** Amanda Canizela, Ariel Inácio, Bernardo Rodrigues, David Nunes

---

## 1. A Ideia do Projeto

O **SignTalk** é um dispositivo wearable (tecnologia vestível) projetado para quebrar barreiras de comunicação. Trata-se de uma luva inteligente capaz de captar os movimentos das mãos do usuário e traduzir os gestos da Língua Brasileira de Sinais (Libras) para texto e áudio em tempo real.

O diferencial do projeto é a sua arquitetura distribuída e sem fio (Wireless) focada em ergonomia e processamento na borda (Edge Computing/TinyML):

- **Coleta na Luva (Transmissor):** Um microcontrolador compacto fica acoplado à luva, lendo continuamente a curvatura de cada dedo (sensores de flexão) e a inclinação/movimentação da mão no espaço (giroscópio/acelerômetro).
- **Comunicação Direta:** Os dados brutos da mão são empacotados em uma struct e enviados via protocolo ESP-NOW (baixíssima latência) para uma base central.
- **Processamento e Saída (Receptor):** Um segundo microcontrolador recebe os dados, executa a inferência em uma rede neural embarcada e reproduz a tradução do gesto através de um alto-falante.

---

## 2. Componentes Utilizados (Hardware)

O projeto utiliza componentes de baixo custo e alta eficiência. Abaixo está a lista atualizada do hardware que compõe o sistema:

### 🧠 Microcontroladores
- **1x ESP32-S3:** O "cérebro" principal. Fica na base receptora, responsável por rodar o modelo de Machine Learning (TinyML) e acionar a saída de áudio.
- **2x ESP32-C3 (Super Mini):** O nó coletor. Microcontrolador minúsculo que fica na luva coletando os dados analógicos e I2C, enviando-os via ESP-NOW.

### ✋ Sensores (A Mão)
- **10x Sensores Flexíveis A12E-10X:** Instalados nos dedos da luva (5 por luva). Eles funcionam baseados na variação de resistência quando dobrados.
- **2x Módulos MPU-6050 (GY-521):** Acelerômetro e Giroscópio de 3 eixos (6 DOF) que fornecem a orientação espacial e aceleração angular da mão (comunicação I2C).
- **10x Resistores de 10kΩ e 10x Capacitores 100nF:** Utilizados para montar os circuitos divisores de tensão e filtros passivos para a leitura estável dos sensores flexíveis.

### 🔊 Interface e Saída
- **1x Amplificador de Áudio MAX98357A (Módulo I2S):** Recebe o áudio digital do ESP32-S3 e amplifica para a caixa de som.
- **1x Alto-falante PK230007O00:** Para a reprodução do Text-to-Speech da tradução.
- **1x Display LCD 16x2 com I2C (Backlight Azul):** Para exibir visualmente em tempo real a letra ou palavra que foi traduzida pela IA.

### 🔋 Estrutura e Alimentação
- **3x Baterias de Lítio (Li-Po/Li-Ion) 3.7V.**
- **2x Placas de Fenolite Perfuradas:** Para a montagem e soldagem dos circuitos da luva e da base.
- **1x Par de Luvas.**
- **(Previsto) Cabeamento Flexível:** Fios de silicone maleáveis (AWG 26 a 30) para interligar os sensores das pontas dos dedos à placa na luva, evitando a quebra por fadiga mecânica que ocorre com fios rígidos (como cabos de rede). 
  - *Obs.: Atualmente, devido ao prazo de entrega, estamos seguindo com o uso de fios rígidos (cabo de rede).*
- **(Previsto) Módulo carregador TP4056:** Para proteção e recarga da bateria via USB. 
  - *Obs.: Precisamos confirmar exatamente o carregador que estamos utilizando.*

---

## 3. Status Atual: Machine Learning e Edge Impulse

O projeto já ultrapassou a fase de validação de hardware isolado (leitura estável de ADC, I2C e I2S) e estabeleceu a comunicação ESP-NOW. Atualmente, o foco principal é o treinamento da Inteligência Artificial.

### Abordagem Atual
Estamos utilizando a plataforma **Edge Impulse** para criar, treinar e exportar o modelo de rede neural que será embarcado no ESP32-S3.

### ⚠️ Desafios e Soluções na Coleta de Dados

#### O Problema (I vs. E)
Durante as primeiras coletas de dados, notou-se que o modelo de IA estava confundindo letras com angulações similares, como o "O" e o "E", devido a variações físicas no repuxo do tecido e número reduzido de amostras (20 por letra).

---

## 4. Registro de Atualizações e Progressos (Log)

*(Adicione novos progressos e atualizações nesta seção)*

### [DD/MM/AAAA] - Título da Atualização
- Descrição da atualização, testes realizados ou novos resultados obtidos.

---

## 5. Registro de Problemas (Issues)

*(Adicione novos problemas, bugs ou obstáculos nesta seção)*

### [Em Aberto] - Título do Problema
- **Data de Identificação:** DD/MM/AAAA
- **Descrição:** Qual é o problema encontrado?
- **Possíveis Causas:** O que pode estar causando isso?
- **Tentativas de Solução:** O que já foi feito para tentar resolver?