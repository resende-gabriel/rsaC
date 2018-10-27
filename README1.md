# rsaC
RSA encryption algorithm in C

# 1 Introdução

  O objetivo deste trabalho é implementar algoritmos em C capazes de codificar e decodificar
uma mensagem dentro de uma imagem utilizando técnicas de criptografia e esteganografia.
  Criptografia consiste em tornar uma informação específica ilegível para pessoas não selecionadas.
Assim, apenas um remetente e destinatário têm acesso ao conteúdo da informação. Esteganografia
consiste em esconder uma mensagem, de forma que ela não seja detectada por outros.
  Para a criptografia será utilizado o método RSA, que é o algoritmo de cifragem mais utilizado
e um dos mais seguros, com uma das melhores relações Segurança x Processamento.
  A esteganografia consiste em ocultar uma mensagem dentro de outra. Assim, mesmo que seja
possível ver a mensagem externa, não é possível observar a existência de outra mensagem escondida.
Esta será feita em uma imagem de formato ppm. Cada caractere da mensagem criptografada será
escondido nos bits menos significativos dos canais de cor de 3 pixels.


# 2 Criptografia

  O RSA consiste em uma cifra utilizando chave pública, ou seja, qualquer um pode cifrar mensagens
usando a chave pública. Porém, para decifrar uma mensagem, é necessário ter a chave
privada, e ter a chave pública não ajuda a descobrir a chave privada.


# 3 Esteganografia

  Para esteganografia, será usada uma imagem qualquer no formato ppm. Cada pixel da imagem
contém 3 valores RGB, que representam as cores vermelho, verde e azul para o pixel. Esses valores
são representados por 8 bits e vão de 0 a 255. Assim, será usado o bit menos significativo desses
canais para esconder a mensagem. Um caractere também pode ser representado por 8 bits, então
para cada um serão usados 3 pixels (com 3 canais de cor cada). A alteração visual na imagem é
muito baixa, pois irá alterar no máximo 1 unidade no valor de um canal de cor.


# 4 Implementação

  Foi desenvolvido um algoritmo para criptografar e esconder a mensagem criptografada em uma
imagem; e outro algoritmo para identificar a mensagem na imagem e descriptografar a mesma.
Em anexo, seguem os programas principais (codificador.c e decodificador.c) e a biblioteca que estes
utilizam (encode.h). Os programas devem ser executados por linha de comando, passando os
valores desejados como argumento. Podem ser compilados em um terminal pelo comando:

gcc codificador.c -o codificador

  Após compilar, o codificador pode ser executado pelo comando
  
codificador imagem.ppm "mensagem para criptografia" saida.ppm valorP valorQ

  Onde o primeiro parâmetro, imagem.ppm, é a imagem na qual se deseja esconder a mensagem; o
segundo parâmetro é a mensagem a ser criptografada e escondida; o terceiro parâmetro, saida.ppm,
é a nova imagem que terá a mensagem escondida; o quarto e quinto parâmetro, valorP e valorQ
respectivamente, é um número primo para P e um número primo para Q.
  O decodificador pode ser executado pelo comando
  
decodificador saida.ppm "delimitador" private.txt

  Onde o primeiro parâmetro, saida.ppm, é a imagem com a mensagem escondida; o segundo
parâmetro, "delimitador", é o caractere que mostra o fim da mensagem na imagem; o terceiro
parâmetro, private.txt, é o arquivo que contém a chave privada (os valores de N e D).
  O programa codificador verifica se os 5 parâmetros foram informados e são válidos antes de
começar a executar os passos de criptografia e esteganografia. Para isso, ele checa se imagem.ppm
é um arquivo existente e se é possível abrí-lo para leitura e checa se os parâmetros valorP e valorQ
são números primos válidos. Se os parâmetros forem válidos, serão obtidos os valores das chaves
pública e privada de acordo com os parâmetros para P e Q informados. A chave pública é composta
por (N, E) e a chave privada é composta por (N, D). N pode ser obtido pela multiplicação de P
por Q; uma variável necessária para obter as outras chaves, o totiente, é obtido por P * Q; E é o
menor valor inteiro maior que 2 que não tem divisores em comum com o totiente (o MDC entre eles
é 1); e D é obtido pelo algoritmo de Euclides estendido entre E e o totiente. Então, os caracteres
da mensagem serão convertidos para o valor ASCII. A string numérica com os valores ASCII será
separada em blocos com valor menor que o valor de N. A criptografia é feita em bloco por bloco,
elevando o bloco a E e fazendo módulo N. Em seguida, os blocos são concatenados em uma string
separados por ’-’, para identificar o fim de cada bloco. O último dígito, que irá delimitar o final da
mensagem na imagem, é concatenado na string. A chave privada é salva em binário em um arquivo
chamado private.txt. Então, o programa lê a imagem informada, verificando antes se esta possui
todas as propriedades certas (como formato - que deve ser P3 ou P6 - altura, largura e o tamanho
do canal de cor - que deve ter 8 bits (255)), salva numa variável local e esconde a mensagem nos
bits menos significativos dos canais de cor dos primeiros pixels. A seguir, salva a imagem em um
arquivo diferente com o nome informado. Os únicos retornos são o arquivo chamado private.txt,
com a chave privada (os valores D e N), e uma nova imagem no formato ppm com a mensagem
criptografada e escondida.
  O programa decodificador verifica se os 3 parâmetros foram informados e são válidos antes
de começar a executar o resto do código. Para isso, ele verifica se é possível abrir a imagem e o
arquivo com a chave privada para leitura e verifica se o "delimitador" informado possui 1 caractere.
Primeiramente, o programa lê a chave privada do arquivo informado, definindo os valores de D e
N. Então lê a imagem informada, salva numa variável local e identifica a mensagem escondida nos
bits menos significativos canais de cor dos pixels, parando ao encontrar o caractere delimitador.
A string obtida da imagem é separada em blocos, de acordo com os caracteres ’-’ na mesma. Os
blocos são descriptografados, elevando o bloco a D e fazendo módulo N, e então concatenados
em uma string. Essa nova string descriptografada contém os valores ASCII da mensagem, que é
convertida em seguida, e a mensagem original é obtida. O único retorno do programa é a exibição
da mensagem oculta na imagem.
  O código possui comentários explicativos, detalhando os procedimentos, cada trecho de código,
o objetivo e a lógica das funções.
