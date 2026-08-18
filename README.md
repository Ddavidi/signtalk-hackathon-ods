<div align="center">

# 🤟 SignTalk

### Luva Inteligente Tradutora de Libras

*Transformando gestos em palavras e silêncio em diálogo.*

[![ODS 10](https://img.shields.io/badge/ODS%2010-Redu%C3%A7%C3%A3o%20das%20Desigualdades-E5243B?style=for-the-badge)](https://brasil.un.org/pt-br/sdgs/10)
[![ODS 4](https://img.shields.io/badge/ODS%204-Educa%C3%A7%C3%A3o%20de%20Qualidade-C5192D?style=for-the-badge)](https://brasil.un.org/pt-br/sdgs/4)
[![ODS 9](https://img.shields.io/badge/ODS%209-Inova%C3%A7%C3%A3o%20e%20Infraestrutura-FD6925?style=for-the-badge)](https://brasil.un.org/pt-br/sdgs/9)

[![Vídeo Demo](https://img.shields.io/badge/▶%20Assistir%20Demo-YouTube-FF0000?style=for-the-badge&logo=youtube)](https://www.youtube.com/watch?v=ag-5S937Mlc)
[![Licença](https://img.shields.io/badge/Licen%C3%A7a-CC--BY--4.0-blue?style=for-the-badge)](LICENSE)

---

**Hackathon PUC Minas - Desafio ODS 2026**

</div>

## 📌 O Problema

No Brasil, **mais de 10 milhões de pessoas** possuem algum grau de deficiência auditiva (IBGE, 2023). A maioria se comunica pela Língua Brasileira de Sinais (Libras), mas **menos de 1% da população ouvinte** compreende a língua.

Essa lacuna cria uma **barreira invisível** que exclui pessoas surdas de:
- 🏥 Consultas médicas e serviços de saúde
- 🏫 Ambientes educacionais e salas de aula
- 💼 Oportunidades no mercado de trabalho
- 🛒 Interações cotidianas no comércio e serviços

A desigualdade comunicacional entre surdos e ouvintes é uma das maiores barreiras à inclusão plena no país, agravada em regiões periféricas e municípios menores onde a oferta de intérpretes é praticamente inexistente.

> 📄 [Leia mais sobre o problema e seu contexto](docs/problema.md)

---

## 💡 Nossa Solução

O **SignTalk** é uma luva inteligente que traduz gestos de Libras para **texto e áudio em tempo real**.

<div align="center">

```
  ┌─────────────┐     ESP-NOW      ┌──────────────────┐
  │  🧤 LUVA     │   (sem fio)     │  🖥️ BASE          │
  │             │  ──────────────► │                  │
  │ 5x Flex     │   Latência       │ TinyML (IA)      │
  │ 1x MPU-6050 │    mínima        │ LCD + Speaker    │
  │ ESP32-C3    │                  │ ESP32-S3         │
  └─────────────┘                  └──────────────────┘
       Captura                        Tradução
    dos gestos                    texto + áudio
```

</div>

**Como funciona:**
1. **Captura:** Sensores de flexão nos dedos e um giroscópio detectam os movimentos da mão
2. **Transmissão:** Os dados são enviados via ESP-NOW (protocolo sem fio de latência mínima)
3. **Processamento:** Uma rede neural embarcada (TinyML) classifica o gesto de Libras
4. **Saída:** A tradução aparece no display LCD e é reproduzida em áudio pelo alto-falante

> 📄 [Detalhes técnicos da solução](docs/solucao.md)

---

## 🎯 Conexão com os Objetivos de Desenvolvimento Sustentável

### ODS 10: Redução das Desigualdades
O SignTalk ataca diretamente a desigualdade comunicacional. Ao permitir que pessoas surdas se expressem e sejam compreendidas sem depender de intérpretes humanos, o dispositivo promove **autonomia, dignidade e participação social plena**.

### ODS 4: Educação de Qualidade
O dispositivo funciona como **ferramenta educacional interativa**: quem está aprendendo Libras pratica os sinais e recebe feedback instantâneo sobre a correção dos gestos. Isso democratiza o ensino da língua e incentiva mais ouvintes a aprenderem Libras.

### ODS 9: Indústria, Inovação e Infraestrutura
Aplicamos **Inteligência Artificial na Borda (TinyML)** em microcontroladores de baixo custo. O sistema funciona 100% offline, sem depender de internet, nuvem ou infraestrutura digital, levando inovação a qualquer região do país.

> 📄 [Análise completa do impacto social](docs/impacto-social.md)

---

## 🎥 Demonstração

[![Vídeo do Protótipo](https://img.youtube.com/vi/ag-5S937Mlc/maxresdefault.jpg)](https://www.youtube.com/watch?v=ag-5S937Mlc)

> Clique na imagem para assistir à demonstração do protótipo funcionando.

---

## 🔧 Tecnologia

| Componente | Descrição |
|---|---|
| **Microcontroladores** | ESP32-C3 Super Mini (luva) + ESP32-S3 (base) |
| **Sensores** | 5x Flexíveis A12E-10X + 1x MPU-6050 (giroscópio/acelerômetro) |
| **Comunicação** | ESP-NOW (2.4 GHz, sem roteador, latência mínima) |
| **IA Embarcada** | TinyML via Edge Impulse (rede neural no microcontrolador) |
| **Saída** | LCD 16x2 I2C + Alto-falante 3W via MAX98357A (I2S) |
| **Alimentação** | Baterias de lítio 3.7V |
| **Cases** | Modelagem 3D (STL) para impressão em PETG/TPU |

> 📄 [Detalhamento completo da tecnologia](docs/tecnologia.md)

---

## 👥 Público-alvo

- **Pessoas surdas e com deficiência auditiva** que enfrentam barreiras de comunicação no cotidiano
- **Estudantes e aprendizes de Libras**, incluindo familiares de surdos, profissionais de saúde e educadores, que podem usar a luva como ferramenta prática de treino com correção em tempo real
- **Instituições de ensino** (escolas, universidades) e **entidades sociais** (associações de surdos, ONGs de inclusão) que buscam ferramentas acessíveis para promover educação bilíngue e acessibilidade

---

## 📊 Resultados Esperados

- ✅ Reconhecimento do **alfabeto completo de Libras** em tempo real
- ✅ Tradução instantânea para **texto (LCD) e áudio (Text-to-Speech)**
- ✅ Funcionamento **100% offline**, sem dependência de internet
- ✅ **Calibração automática** que se adapta a qualquer tamanho de mão
- 🎯 Expansão para dezenas de **palavras e expressões** do cotidiano
- 🎯 Adoção como **ferramenta educacional** em escolas e centros comunitários

---

## 📁 Estrutura do Repositório

```
signtalk-hackathon-ods/
├── README.md                    # Este arquivo
├── LICENSE                      # Licença CC-BY-4.0
├── docs/
│   ├── problema.md              # Contexto do problema e justificativa
│   ├── solucao.md               # Descrição detalhada da solução
│   ├── impacto-social.md        # Conexão com as ODS e impacto social
│   ├── tecnologia.md            # Detalhamento técnico do hardware e software
│   └── equipe.md                # Informações sobre a equipe
├── hardware/
│   ├── Central.stl              # Modelagem 3D do case da base receptora
│   └── Luva.stl                 # Modelagem 3D do case da luva
└── media/
    └── fotos/                   # Fotos do protótipo e desenvolvimento
```

---

## 👨‍💻 Equipe

| Nome | Papel |
|---|---|
| **Amanda Canizela Guimarães** | Engenharia de Computação, PUC Minas |
| **Ariel Inácio Jordão Coelho** | Engenharia de Computação, PUC Minas |
| **Bernardo Rodrigues Pereira** | Engenharia de Computação, PUC Minas |
| **David Nunes Ribeiro** | Engenharia de Computação, PUC Minas |

**Orientador:** Prof. Ilo Amy Saldanha Rivero

> 📄 [Mais sobre a equipe](docs/equipe.md)

---

<div align="center">

*Projeto desenvolvido para o Hackathon PUC Minas - Desafio ODS 2026*

**PUC Minas - Engenharia de Computação**

</div>
