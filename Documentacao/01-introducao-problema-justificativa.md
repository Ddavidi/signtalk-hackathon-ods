# Documentação Básica do Projeto

Preencha as seções abaixo conforme orientações da disciplina. Este conteúdo pode ser incorporado ao relatório formal do projeto.

---

## Introdução

A Língua Brasileira de Sinais (Libras) é o meio de comunicação natural da comunidade surda no Brasil, sendo reconhecida legalmente como meio legal de comunicação e expressão. No entanto, a maior parte da população ouvinte não possui fluência ou sequer conhecimento básico em Libras. Nesse contexto, a tecnologia surge como uma ponte potencial para aproximar esses dois mundos. O **SignTalk** insere-se nesse cenário como um dispositivo wearable (tecnologia vestível) — especificamente, uma luva inteligente equipada com sensores de movimento e flexão — projetado para capturar gestos em Libras e traduzi-los, em tempo real, para texto e áudio perceptíveis por ouvintes. Utilizando Inteligência Artificial na Borda (TinyML), o SignTalk opera de forma autônoma e portátil, dispensando a necessidade de conexões com a internet ou smartphones intermediários.

---

## Problema

O principal problema que o projeto busca resolver é a **barreira de comunicação existente entre a comunidade surda, usuária de Libras, e a sociedade ouvinte, que majoritariamente desconhece a língua**. Essa lacuna linguística gera exclusão social, dificultando o acesso de pessoas surdas a serviços básicos de saúde, educação, mercado de trabalho e interações cotidianas no comércio. Em situações de emergência ou em simples interações sociais, a ausência de um intérprete humano muitas vezes inviabiliza a comunicação efetiva, tornando os surdos dependentes de terceiros ou forçados a recorrer a mímicas e anotações precárias.

---

## Justificativa

O desenvolvimento do SignTalk é altamente relevante pois promove a **acessibilidade e a inclusão social direta e independente** da pessoa surda. Ao oferecer um dispositivo vestível e ergonômico que traduz gestos instantaneamente para áudio e texto, o projeto empodera o usuário, permitindo que ele seja compreendido em praticamente qualquer ambiente.

Do ponto de vista tecnológico, a solução inova ao combinar a **Computação na Borda (Edge Computing e TinyML)** com comunicação de ultrabaixa latência (ESP-NOW) através de microcontroladores acessíveis e de baixo custo (família ESP32). Isso significa que, diferentemente de soluções baseadas em processamento em nuvem ou visão computacional pesada (câmeras), o SignTalk é barato, processa as redes neurais localmente (garantindo privacidade e velocidade) e não sofre com problemas de conexão à internet, resultando em uma ferramenta prática, viável e escalável para o dia a dia.
