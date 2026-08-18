# 📋 Contexto da Conversa — SignTalk (Continuação)

> **Data:** 03/08/2026  
> **Objetivo:** Retomar o planejamento de melhorias da luva SignTalk após esta sessão.  
> **Como usar:** Compartilhe este arquivo inteiro com a IA ao começar uma nova conversa para retomar o contexto completo.

---

## 🧤 Estado Atual do Projeto

O **SignTalk** é uma luva tradutora de Libras com arquitetura distribuída:

- **Luva (Transmissor):** ESP32-C3 Super Mini + 5x Sensores Flex A12E-10X + 1x MPU-6050 (GY-521)
- **Base (Receptor):** ESP32-S3 + LCD 16x2 I2C + MAX98357A + Speaker + TinyML (Edge Impulse)
- **Comunicação:** ESP-NOW (2.4 GHz, latência mínima)
- **IA:** Edge Impulse com dataset atual de 20–60 amostras por letra (ainda insuficiente)
- **Firmware da luva:** [`/Codigo/Firmware/Arduino/esp32_c3_luva.ino`](../Codigo/Firmware/Arduino/esp32_c3_luva.ino)
- **Firmware da base:** [`/Codigo/Firmware/Arduino/esp32_s3_cerebro.ino`](../Codigo/Firmware/Arduino/esp32_s3_cerebro.ino) (não foi alterado)

**Status dos códigos:** Os códigos estão no estado original — **nenhuma alteração foi feita no código** nesta sessão. Apenas análises foram discutidas.

---

## ✅ Decisões Tomadas Nesta Sessão

### Foco do Hackathon ODS:
- **Prioridade 1:** Alfabeto completo (letras A–Z, incluindo J e Z que são dinâmicas)
- **Prioridade 2 (se sobrar tempo):** Palavras simples sem movimento corporal

### O que foi decidido sobre cada tópico:

#### 1. Cabos Flexíveis (já comprados)
- Instalar fios de silicone AWG 26-30
- Técnica: folga em "U" em cada articulação do dedo (service loop)
- Rotear pelo dorso da mão (não pela palma)
- Usar cola quente para aliviar tensão nas soldas das pontas

#### 2. MPU por dedo — DESCARTADO
- Não vale a pena: o MPU mede orientação da mão, não curvatura de dedo
- O problema real era os fios rígidos deslocando os sensores flex
- Solução: cabos flexíveis + mais amostras de treino (100–200 por letra)
- Se precisar de mais precisão: adicionar o **2º MPU-6050** (já no BOM) no dorso da mão usando TCA9548A

#### 3. Letra J e gestos dinâmicos
- O hardware atual **consegue capturar** o J via MPU-6050 (giroscópio)
- Mudanças necessárias no Edge Impulse para J/Z:
  - Janela maior: 1500–2000ms
  - Bloco: `Spectral Analysis` (não Flatten)
  - Arquitetura: considerar LSTM em vez de Dense simples

#### 4. Frases completas — Limitação confirmada
- O MPU-6050 **não consegue posição absoluta** da mão (drift na integração dupla)
- Muitos sinais de Libras dependem de onde a mão está relativa ao corpo
- **Para o hackathon:** focar no alfabeto e palavras sem referência corporal
- Roadmap futuro: v1.5 = ~30 palavras, v2.0 = IMU no antebraço como referência

#### 5. Calibração por usuário (tamanho de mão)
- **Ideia aprovada:** Rotina de calibração antes do uso (mão aberta → punho fechado)
- Normaliza os sensores flex de 0–4095 (ADC bruto) para 0–100 (por usuário)
- **Código pronto mas NÃO foi gravado no arquivo** — esperar para implementar junto com a equipe
- A lógica envolve: `flexMin[5]`, `flexMax[5]`, função `calibrarFlex()`, função `normalizarFlex()`

#### 6. Design inclusivo e cobertura dos componentes
- Usar impressora 3D para case no dorso da mão
- Material recomendado: **PETG ou TPU** (não PLA)
- Cobrir com capa de **lycra** por cima de tudo
- Cores discretas: preto, cinza, navy (não cores médicas)

#### 7. Higiene e limpeza
- **Solução ideal:** Design modular — módulo eletrônico desencaixa via pogo pins/JST
- **Solução imediata:** Liner descartável (luva cirúrgica fina) por dentro
- Limpeza de superfície: álcool isopropílico 70% no tecido externo

#### 8. Luva base para comprar
- **Recomendação:** Luva de musculação/academia **half-finger** (meio dedo)
- Material: neoprene + lycra, palma de couro sintético
- Dedos parcialmente expostos = articulações acessíveis para os sensores
- Marcas: Penalty, Hammerhead, Fila, Gold's Gym

---

## ⚠️ Problemas Conhecidos (Não Resolvidos)

1. **Fios rígidos ainda na luva** — aguardando montagem dos cabos flexíveis comprados
2. **Dataset pequeno (20–60 amostras/letra)** — precisa chegar a 100–200
3. **Confusão O/E e A/S** — será parcialmente resolvida pelos cabos flexíveis + mais dados
4. **J e Z não foram treinadas** — precisam de configuração diferente no Edge Impulse
5. **Nenhum case 3D ainda** — ESP32 e bateria expostos no dorso da mão

---

## 📌 Próximos Passos (Por Prioridade)

- [ ] **1. Instalar cabos flexíveis** já comprados com técnica de folga em U
- [ ] **2. Comprar luva de academia half-finger** como nova base
- [ ] **3. Ampliar dataset** para 100+ amostras por letra com nova configuração de fios
- [ ] **4. Implementar calibração flex por usuário** no firmware (código já planejado)
- [ ] **5. Modelar e imprimir case 3D** para ESP32 + bateria no dorso da mão
- [ ] **6. Costurar capa de lycra** sobre os componentes
- [ ] **7. Configurar Edge Impulse para J e Z** (janela maior + Spectral Analysis)
- [ ] **8. Retreinar modelo** com dataset ampliado

---

## 📚 Referências para Pesquisar (Termos de Busca)

Como os links diretos não foram verificados, use estes termos no YouTube/Printables/Thingiverse:

| O que procurar | Onde pesquisar | Termo |
|---|---|---|
| Canal de fio em luva | YouTube | `smart glove flex sensor wire routing` |
| Guia de cabo 3D | Printables | `OpenPrintSense` ou `cable finger guide TPU` |
| Projeto de luva 3D | Thingiverse | `data glove flex sensor` |
| Tutorial montagem luva | SparkFun | `learn.sparkfun.com/tutorials/flex-sensor-hookup-guide` |
| Conector modular lavável | YouTube | `pogo pin connector wearable` |
| Luva academia base | Mercado Livre | `luva academia meio dedo` ou `luva crossfit grip` |

---

## 🔖 Informações do Projeto

- **Equipe:** Amanda Canizela, Ariel Inácio, Bernardo Rodrigues, David Nunes
- **Orientador:** Prof. Ilo Amy Saldanha Rivero
- **Instituição:** PUC Minas — Engenharia de Computação
- **ODS Principal:** ODS 10 — Redução das Desigualdades
- **ODS Secundária:** ODS 9 — Indústria, Inovação e Infraestrutura
- **Hackathon:** PUC Minas Hackathon ODS 2026 (desenvolvimento: 11 julho a 14 agosto)
- **Vídeo do protótipo:** https://www.youtube.com/watch?v=ag-5S937Mlc

---

*Este arquivo foi gerado automaticamente para continuidade da sessão de planejamento.*
