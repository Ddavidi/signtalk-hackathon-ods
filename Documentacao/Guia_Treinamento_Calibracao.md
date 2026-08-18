# Guia de Calibração e Treinamento de Sinais (Libras)

Este documento descreve o passo a passo recomendado para calibrar a luva SignTalk e realizar a coleta de dados e treinamento de uma nova letra (ou gesto) utilizando o Edge Impulse. O objetivo é mitigar problemas de confusão entre letras parecidas (como 'O' e 'E') e garantir a máxima precisão na inferência embarcada.

---

## 1. Calibração Inicial (Hardware e Firmware)

Antes de coletar dados para o Edge Impulse, é fundamental garantir que as leituras enviadas pela luva estejam consistentes.

### 1.1. Posicionamento Físico
*   **Ajuste da Luva:** A luva deve estar bem ajustada à mão. Variações na forma como o tecido repuxa podem alterar drasticamente a leitura dos sensores de flexão.
*   **Fixação dos Sensores:** Garanta que os sensores não estão deslizando no dedo. O ponto de dobra do sensor deve coincidir sempre com as articulações dos dedos.

### 1.2. Normalização via Firmware (Mapeamento)
Os valores analógicos lidos dos sensores de flexão costumam variar de pessoa para pessoa e dependem da tensão da bateria. 
*   **Defina os Limites (Min/Max):** No código do ESP32-C3, crie uma rotina de calibração rápida ao ligar a luva:
    1.  Mão totalmente aberta: Registre o valor mínimo de cada sensor.
    2.  Mão totalmente fechada: Registre o valor máximo de cada sensor.
*   **Mapeamento:** Utilize a função `map()` para converter os valores brutos para uma escala percentual padrão (ex: `0` a `100`), onde `0` é o dedo reto e `100` é o dedo totalmente dobrado.
*   *Nota:* Enviar dados normalizados para a IA facilita o treinamento e torna o modelo menos dependente de um hardware específico.

---

## 2. Coleta e organização dos dados

### 2.1. Captura dos dados com script Python

Para a coleta dos sinais da luva, utilize o script Python desenvolvido para comunicação com o ESP32-S3 pela porta Serial. Esse script é responsável por:

- conectar-se à porta Serial do ESP32-S3;
- enviar automaticamente o comando `COLETA` para o microcontrolador;
- ler as linhas enviadas pela Serial;
- filtrar apenas linhas iniciadas com `DATA,`;
- salvar as amostras em arquivos `.csv` dentro da pasta `dataset`.

Durante a execução, o script solicita o nome do gesto que será gravado. Exemplos:

```text
letra_A
letra_B
letra_D
letra_E
repouso
```

Após informar o nome do gesto, o usuário posiciona a mão, pressiona Enter e o script grava os dados durante o tempo definido na variável `TEMPO_GRAVACAO`.

Exemplo de arquivo gerado:

```text
dataset/letra_A_20260706_153012.csv
```

O arquivo CSV gerado contém o seguinte cabeçalho:

```text
timestamp,id_luva,flex_polegar,flex_indicador,flex_medio,flex_anelar,flex_minimo,acel_x,acel_y,acel_z,giro_x,giro_y,giro_z,roll,pitch
```

Para o treinamento no Edge Impulse, as principais entradas do modelo devem ser os sensores flex e os dados da IMU. As colunas `timestamp` e `id_luva` servem apenas como informação auxiliar e não devem ser usadas como features do modelo.

As features recomendadas são:

```text
flex_polegar
flex_indicador
flex_medio
flex_anelar
flex_minimo
acel_x
acel_y
acel_z
giro_x
giro_y
giro_z
roll
pitch
```

---

### 2.2. Procedimento de captura de uma letra

Para capturar uma letra, por exemplo a letra `A`, siga o procedimento abaixo:

1. Execute o script Python de coleta.
2. Quando solicitado, informe o nome da classe:

```text
letra_A
```

3. Posicione a mão no formato da letra desejada.
4. Pressione Enter para iniciar a gravação.
5. Mantenha o gesto durante o tempo definido no script.
6. Ao final, o script salvará automaticamente o arquivo `.csv` na pasta `dataset`.

Esse processo deve ser repetido várias vezes para cada letra, gerando múltiplos arquivos CSV por classe.

---

### 2.3. Variabilidade durante a coleta

Durante a gravação, evite manter a mão perfeitamente imóvel e sempre na mesma posição. É importante introduzir pequenas variações naturais, como:

- pequenas mudanças no ângulo do pulso;
- leve variação na força aplicada nos sensores flex;
- pequena mudança na inclinação da mão;
- ajustes naturais entre uma amostra e outra;
- pequenas diferenças entre uma repetição e outra do mesmo gesto.

Isso ajuda o modelo a aprender que pequenas diferenças físicas ainda representam a mesma letra.

Por exemplo, a letra `A` não deve ser gravada sempre com os sensores exatamente nos mesmos valores. O modelo precisa reconhecer a letra `A` mesmo que o polegar esteja um pouco mais flexionado, que o pulso esteja levemente inclinado ou que a mão esteja em uma posição um pouco diferente.

Essa variabilidade reduz o risco de **overfitting**, ou seja, evita que a rede neural aprenda apenas uma versão muito específica do gesto.

---

### 2.4. Quantidade de amostras

Para obter um modelo mais confiável, recomenda-se coletar pelo menos:

```text
50 a 100 amostras por letra
```

Cada amostra corresponde a uma janela de gravação salva em um arquivo `.csv`.

Se algumas letras estiverem sendo confundidas, como `E` e `O`, ou `A` e `S`, é recomendado aumentar a quantidade de amostras dessas classes específicas.

Para letras problemáticas, pode ser útil coletar:

```text
100 a 200 amostras
```

Também é importante equilibrar o conjunto de dados. Evite situações como:

```text
100 amostras da letra E
20 amostras da letra A
15 amostras da letra B
```

Esse desequilíbrio pode fazer o modelo favorecer a classe com mais exemplos.

---

### 2.5. Classe de repouso

Além das letras, é importante criar uma classe de repouso, por exemplo:

```text
repouso
```

ou:

```text
nenhum_sinal
```

Essa classe representa a mão relaxada ou em posição neutra.

Ela é importante porque, sem essa classe, o modelo sempre tentará escolher uma letra, mesmo quando o usuário não estiver fazendo nenhum sinal. Com a classe de repouso, a luva aprende a diferenciar um gesto real de uma situação em que a mão está parada ou relaxada.

---

## 3. Envio dos dados para o Edge Impulse

Após a coleta, os arquivos `.csv` gerados pelo script devem ser enviados para o Edge Impulse.

No Edge Impulse, acesse:

```text
Data acquisition
```

Depois envie os arquivos correspondentes a cada classe.

Ao importar os arquivos, confirme se:

- os nomes das colunas estão corretos;
- cada arquivo está associado ao rótulo correto;
- os dados estão organizados como série temporal;
- a frequência de amostragem está coerente com a coleta feita no ESP32-S3;
- as colunas `timestamp` e `id_luva` não estão sendo usadas como entrada do modelo.

As colunas recomendadas para o modelo são:

```text
flex_polegar
flex_indicador
flex_medio
flex_anelar
flex_minimo
acel_x
acel_y
acel_z
giro_x
giro_y
giro_z
roll
pitch
```

As colunas abaixo devem ser desconsideradas como features:

```text
timestamp
id_luva
```

O `timestamp` serve apenas para indicar o tempo da amostra, e o `id_luva` serve apenas para identificar qual luva enviou os dados.

---

## 4. Configuração do impulse

No Edge Impulse, acesse a aba:

```text
Create impulse
```

---

### 4.1. Time Series Data

Configure os dados como série temporal.

Para letras estáticas de Libras, uma janela entre:

```text
500 ms e 1000 ms
```

geralmente é suficiente.

Como as letras analisadas são majoritariamente posições da mão, a rede não precisa de uma janela muito longa. O objetivo é capturar a posição dos dedos e a inclinação da mão durante um curto intervalo de tempo.

Se a coleta atual for de 3 segundos por arquivo, o Edge Impulse pode dividir esse arquivo em janelas menores durante o processamento, dependendo da configuração de janela e incremento.

Exemplo inicial recomendado:

```text
Window size: 500 ms a 1000 ms
Window increase: 100 ms a 250 ms
```

---

### 4.2. Processing Block

Para letras estáticas, prefira usar:

```text
Raw Data
```

ou:

```text
Flatten
```

O bloco **Raw Data** mantém a sequência temporal original das leituras, permitindo que o modelo aprenda diretamente a variação dos sensores ao longo da janela.

O bloco **Flatten** pode ser útil quando o objetivo é representar cada janela por características como média, mínimo, máximo e variações dos sensores. Para sinais estáticos, isso pode funcionar bem porque a posição dos dedos tende a ser mais importante do que oscilações rápidas.

Para o projeto Sign Talk, uma configuração recomendada é começar com:

```text
Raw Data
```

Se o modelo apresentar instabilidade ou muita variação entre classificações consecutivas, teste também:

```text
Flatten
```

Para gestos dinâmicos, como letras que envolvem movimento, pode ser necessário testar blocos mais adequados para variações temporais, como análise espectral. Porém, para letras estáticas, Raw Data ou Flatten tendem a ser escolhas mais adequadas.

---

### 4.3. Learning Block

Use o bloco:

```text
Classification (Keras)
```

Esse bloco é adequado porque o objetivo é classificar cada janela de sensores em uma classe, como:

```text
letra_A
letra_B
letra_D
letra_E
repouso
```

Uma arquitetura inicial simples pode ser suficiente:

```text
Dense 64
Dropout 0.2
Dense 32
Dropout 0.2
Dense 16
Output Softmax
```

O Softmax final é responsável por indicar a probabilidade de cada classe.

---

## 5. Treinamento do modelo

Depois de configurar o impulse, acesse a etapa de treinamento do modelo.

Durante o treinamento, observe:

- acurácia de treino;
- matriz de confusão;
- perda do modelo;
- classes com maior taxa de erro;
- possíveis sinais de overfitting.

Um modelo pode apresentar alta acurácia no treino, mas desempenho ruim em dados novos. Por isso, a validação com amostras não vistas é essencial.

Se o modelo estiver confundindo letras específicas, colete mais dados dessas classes e inclua mais variações reais do gesto.

---

## 6. Validação do modelo

Depois do treinamento, use a aba:

```text
Model testing
```

Essa etapa testa o modelo com dados que não foram usados diretamente no treino.

Observe principalmente:

- accuracy geral;
- matriz de confusão;
- letras mais confundidas;
- confiança das predições;
- desempenho da classe `repouso` ou `nenhum_sinal`.

Se uma letra estiver sendo confundida com outra, volte para a etapa de coleta e adicione mais amostras dessas classes, incluindo variações de posição, ângulo do pulso e flexão dos dedos.

---

## 7. Implantação no ESP32-S3

Quando o modelo apresentar bons resultados no Edge Impulse, vá em:

```text
Deployment
```

Selecione:

```text
Arduino library
```

Baixe o arquivo `.zip` gerado e instale na Arduino IDE:

```text
Sketch → Include Library → Add .ZIP Library
```

Depois, o modelo poderá ser executado localmente no ESP32-S3, usando a função:

```cpp
run_classifier()
```

Dessa forma, o ESP32-S3 passa a fazer a inferência diretamente na placa, sem depender do site do Edge Impulse durante o uso final.

---

## 8. Teste no ESP32-S3

Após instalar a biblioteca Arduino gerada pelo Edge Impulse e carregar o código no ESP32-S3, abra o Serial Monitor em:

```text
115200 baud
```

O sistema deve aceitar comandos como:

```text
DEBUG
COLETA
IA
STATUS
PING
```

Para testar a inferência local, envie:

```text
IA
```

O ESP32-S3 começará a acumular as amostras recebidas da luva e executará o modelo quando a janela estiver completa.

Uma saída esperada pode ser:

```text
[LETRA] E | confianca: 0.95 | Flex: Polegar=1209 Indicador=1288 Medio=2102 Anelar=1246 Minimo=1103
```

Caso o modelo esteja mostrando sempre a mesma letra, verifique:

- se a ordem dos eixos no código é igual à ordem usada no Edge Impulse;
- se os dados reais têm a mesma faixa dos dados de treino;
- se há amostras suficientes para todas as classes;
- se existe classe de repouso;
- se o modelo está usando `timestamp` ou `id_luva` indevidamente;
- se a luva está enviando pacotes com o mesmo formato esperado pelo receptor.

---
