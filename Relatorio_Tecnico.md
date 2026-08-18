# Relatório Técnico - Hackathon PUC Minas Desafio ODS 2026

**Equipe:** Amanda Canizela Guimarães, Ariel Inácio Jordão Coelho, Bernardo Rodrigues Pereira, David Nunes Ribeiro
**Projeto:** SignTalk - Luva Tradutora de Libras

---

## 1. O Problema: A Barreira Invisível da Comunicação

A Língua Brasileira de Sinais (Libras) é reconhecida legalmente como meio de comunicação e expressão da comunidade surda no Brasil (Lei nº 10.436/2002). No entanto, a realidade prática está muito distante do que a legislação prevê. Segundo dados do IBGE (Censo 2022 e PNS), **mais de 10 milhões de brasileiros** possuem algum grau de deficiência auditiva, dos quais 2,3 a 2,6 milhões possuem surdez severa ou profunda. Em contrapartida, **menos de 1% da população ouvinte** possui qualquer nível de compreensão da língua.

Essa lacuna cria uma **exclusão sistêmica** que afeta a vida de uma pessoa surda em várias esferas:
*   **Saúde:** Dificuldade em descrever sintomas e compreender diagnósticos em hospitais. A ausência de intérpretes em unidades de emergência pode colocar vidas em risco.
*   **Educação:** Alunos surdos dependem quase exclusivamente de intérpretes, quando disponíveis. A falta de ferramentas acessíveis prejudica o rendimento escolar.
*   **Mercado de Trabalho:** A barreira de comunicação reduz drasticamente as oportunidades profissionais e de crescimento financeiro.

O problema se intensifica em regiões periféricas e áreas rurais, onde a oferta de intérpretes é quase nula e o acesso a tecnologias em nuvem é prejudicado por falhas na infraestrutura de internet.

## 2. Conexão com os Objetivos de Desenvolvimento Sustentável (ODS)

A barreira comunicacional representa uma desigualdade estrutural. Pessoas surdas são forçadas a depender de intermediários. O SignTalk ataca esse problema, atendendo a:

- **ODS Primário (Obrigatório) – ODS 10: Redução das Desigualdades:** Ao permitir que pessoas surdas se expressem e sejam compreendidas sem depender de intérpretes, a luva promove autonomia, dignidade e participação social plena.
- **ODS Secundário (Optativo) – ODS 4: Educação de Qualidade:** O dispositivo atua como ferramenta educacional. Quem aprende Libras pode praticar os sinais usando a luva e receber feedback instantâneo sobre a precisão do gesto, democratizando o ensino.
- **ODS Secundário Adicional – ODS 9: Indústria, Inovação e Infraestrutura:** A aplicação de Inteligência Artificial na Borda (TinyML) em microcontroladores de baixíssimo custo funciona 100% offline, levando inovação a qualquer região, independentemente da infraestrutura digital.

## 3. A Solução e o Design do Produto (MVP)

O **SignTalk** é um dispositivo vestível não-invasivo que traduz gestos de Libras para **texto e áudio em tempo real**. O sistema utiliza uma arquitetura distribuída de latência ultrabaixa para garantir agilidade no diálogo.

O MVP ("Fumaça") comprova o conceito operando com dois módulos principais:

### 3.1 Luva Transmissora (Captura)
Responsável por coletar os dados do movimento. O design é focado na ergonomia, sendo leve para não atrapalhar o movimento da mão.
*   **Microcontrolador:** ESP32-C3 Super Mini.
*   **Sensores:** 5x Sensores Flexíveis A12E-10X (para medir a curvatura de cada dedo) e 1x MPU-6050 (Acelerômetro e Giroscópio para monitorar a inclinação/rotação da mão).
*   **Comunicação:** Transmissão instantânea via ESP-NOW.

### 3.2 Base Receptora (Tradução)
É o cérebro que classifica o movimento.
*   **Microcontrolador e IA:** ESP32-S3, que roda um modelo de Rede Neural embarcado (TinyML), gerado via Edge Impulse, treinado para inferir o gesto (e.g. letras de 'A' a 'Z' do alfabeto em Libras, além de palavras).
*   **Saída Visual:** Display LCD 16x2 conectado via barramento I2C, exibindo o texto para interlocutores lerem.
*   **Saída de Áudio:** Módulo amplificador MAX98357A integrado ao barramento I2S para sintetização do áudio (Text-to-Speech), permitindo conversas faladas.

## 4. Tecnologia e Vantagens (Edge Computing)

A decisão tecnológica central foi rodar todo o sistema de Machine Learning localmente no ESP32-S3 (Offline / Edge Computing). Isso traz quatro benefícios cruciais validados em nosso MVP:

1.  **Privacidade e Segurança:** Os dados de saúde ou pessoais gestuados nunca são enviados a servidores externos.
2.  **Velocidade (Ausência de Latência):** Ao contrário de APIs baseadas na nuvem, o ESP-NOW e o TinyML entregam a tradução em poucos milissegundos.
3.  **Acessibilidade Geográfica:** Funciona perfeitamente em zonas rurais e hospitais onde o bloqueio de sinal Wi-Fi/Celular é comum.
4.  **Calibração Automática:** O software embarcado dispõe de uma rotina de normalização dos sinais analógicos que adapta a resposta dos sensores aos diferentes tamanhos de mãos dos usuários.

## 5. Viabilidade Gerencial-Mercadológica

O modelo de negócios do SignTalk une inovação tecnológica com empreendedorismo socioambiental de alto impacto:

- **Mercado B2B / B2G (Institucional):** Clínicas, SUS, repartições públicas e escolas regulares. A lei exige adequações de acessibilidade (LBI), e o dispositivo é uma solução escalável para criar "pontos de atendimento acessíveis" a um custo muito inferior à contratação 24/7 de intérpretes físicos.
- **Mercado B2C (Consumidor Final):** Focado na comunidade surda (que visa maior autonomia no seu dia a dia) e seus familiares, além de funcionar como um "gadget" educativo para aprendizes da língua em cursos particulares de Libras.
- **Custo-Benefício:** Por utilizar hardware acessível (Família ESP32) e modelagem 3D em PETG, o Custo de Mercadoria Vendida (CMV) em grande escala viabiliza um preço de venda muito competitivo para mercados emergentes, rompendo com o estigma de que tecnologias assistivas são sempre artigos de luxo.

## 6. Validação do Produto (Público-Alvo) e Impacto

O desenvolvimento incluiu validações contínuas junto à comunidade alvo (Edital item 3.3.1).
*   Foram conduzidas pesquisas através de landing pages (**MVP Fumaça**) para coletar o grau de interesse do público surdo e validar a real dor da ausência de tradução rápida. A adesão ("lista de espera") de interessados corroborou a escalabilidade B2C do produto.
*   Nos testes preliminares, a aceitação do hardware vestível foi amplamente positiva, e o modelo classificatório validou a capacidade de tradução do alfabeto completo de Libras em tempo real, atingindo com precisão as metas propostas de conectar pessoas além das barreiras do som. *(As métricas audiovisuais e prints estão disponíveis na seção de Pitch e MVP Fumaça deste projeto).*

---
*(Relatório formatado conforme a exigência do Edital - Item 3.3, alínea 'c', tópico 'i'. Máximo de 15 páginas. A equipe deverá exportá-lo e submetê-lo no sistema da PUC Minas).*
