<div align="center">

# 🤟 SignTalk - Hackathon PUC Minas Desafio ODS 2026

### Luva Inteligente Tradutora de Libras

*Transformando gestos em palavras e silêncio em diálogo.*

[![ODS 10](https://img.shields.io/badge/ODS%2010-Redu%C3%A7%C3%A3o%20das%20Desigualdades-E5243B?style=for-the-badge)](https://brasil.un.org/pt-br/sdgs/10)
[![ODS 4](https://img.shields.io/badge/ODS%204-Educa%C3%A7%C3%A3o%20de%20Qualidade-C5192D?style=for-the-badge)](https://brasil.un.org/pt-br/sdgs/4)
[![ODS 9](https://img.shields.io/badge/ODS%209-Inova%C3%A7%C3%A3o%20e%20Infraestrutura-FD6925?style=for-the-badge)](https://brasil.un.org/pt-br/sdgs/9)

[![Vídeo Demo](https://img.shields.io/badge/▶%20Assistir%20Pitch%20%26%20Demo-YouTube-FF0000?style=for-the-badge&logo=youtube)](https://www.youtube.com/watch?v=ag-5S937Mlc)

</div>

---

O **SignTalk** é uma luva inteligente (dispositivo wearable/IoT) capaz de captar os movimentos das mãos do usuário e traduzir os gestos da Língua Brasileira de Sinais (Libras) para texto e áudio em tempo real, utilizando Inteligência Artificial na Borda (TinyML).

Este repositório contém a documentação completa e os artefatos exigidos para a entrega da **Terceira Etapa** do programa *Hackathon PUC Minas – Desafio ODS 2026*, assim como o código-fonte (Firmware e Scripts), diagramas de hardware e mídias de validação.

## 📦 Entregáveis do Hackathon (Edital Etapa 3)

Conforme estipulado no Edital (Item 3.3, alínea 'c'), este repositório centraliza nossas entregas oficiais:

1. 📄 **[Relatório Técnico (PDF)](./Relatorio_Tecnico.md):** Documento consolidado detalhando problema, tecnologia, design, viabilidade mercadológica, validação do público-alvo e impacto social. *(O arquivo Markdown está formatado e pronto para exportação em PDF).*
2. 💨 **[Pitch e MVP Fumaça](./Pitch_e_MVP.md):** Contém os links de acesso para o vídeo de Pitch (YouTube) e para o link público de validação (Landing Page / Protótipo de adoção).

---

## 📌 O Problema e Público-Alvo

No Brasil, **mais de 10 milhões de pessoas** possuem algum grau de deficiência auditiva (IBGE, 2023). A maioria se comunica pela Língua Brasileira de Sinais (Libras), mas **menos de 1% da população ouvinte** compreende a língua. Essa lacuna cria uma **barreira invisível** que exclui pessoas surdas de hospitais, escolas e do mercado de trabalho.

## 💡 A Solução (MVP)

<div align="center">

```text
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

O modelo foi projetado para rodar offline, sem custo de nuvem e com latência imperceptível, consistindo de:

| Componente | Descrição |
|---|---|
| **Microcontroladores** | ESP32-C3 Super Mini (luva) + ESP32-S3 (base) |
| **Sensores** | 5x Flexíveis A12E-10X + 1x MPU-6050 (giroscópio/acelerômetro) |
| **Comunicação** | ESP-NOW (2.4 GHz, sem roteador, latência mínima) |
| **IA Embarcada** | TinyML via Edge Impulse (rede neural no microcontrolador) |
| **Saída** | LCD 16x2 I2C + Alto-falante via MAX98357A (I2S) |

## 🌍 Conexão com os Objetivos de Desenvolvimento Sustentável

A nossa solução ataca diretamente o problema da exclusão social e comunicacional:

- **ODS 10 (Primário) - Redução das Desigualdades:** Ao permitir que pessoas surdas se expressem e sejam compreendidas sem depender de intérpretes, o dispositivo promove autonomia, dignidade e participação social plena.
- **ODS 4 (Secundário) - Educação de Qualidade:** O dispositivo funciona como ferramenta educacional: quem aprende Libras recebe feedback em tempo real se o gesto está correto.
- **ODS 9 - Indústria, Inovação e Infraestrutura:** Aplicamos IA em microcontroladores baratos e de baixo consumo. O sistema funciona offline, superando a falta de infraestrutura de conectividade em regiões afastadas do país.

## 📁 Estrutura do Repositório

Para garantir total transparência do nosso MVP, disponibilizamos o projeto base:
- **`/Codigo`**: Código fonte em C++ (ESP-IDF/Arduino) para a Luva e Base, e scripts Python para coleta de datasets.
- **`/Hardware`**: Diagramas elétricos e modelos `.stl` para impressão 3D (Case da luva e central).
- **`/Documentacao`**: Material acadêmico base da concepção do projeto.
- **`/Divulgacao`**: Vídeos, fotos de oficinas e prints dos questionários de validação.

## 👨‍💻 Equipe
- **Amanda Canizela Guimarães** (Engenharia de Computação, PUC Minas)
- **Ariel Inácio Jordão Coelho** (Engenharia de Computação, PUC Minas)
- **Bernardo Rodrigues Pereira** (Engenharia de Computação, PUC Minas)
- **David Nunes Ribeiro** (Engenharia de Computação, PUC Minas)

**Orientador:** Prof. Ilo Amy Saldanha Rivero

---
<div align="center">
  <em>Projeto desenvolvido para o Hackathon PUC Minas - Desafio ODS 2026.</em>
</div>
