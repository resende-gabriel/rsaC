#include<stdio.h>
#include"encode.h"

/*
Gabriel Resende de Andrade

Este programa serve para indentificar e descriptografar pelo metodo RSA uma mensagem escondida em uma imagem

Deve ser rodado pelo terminal, e recebe 3 parametros
(1) - nome da imagem para a leitura da mensagem escondida (no formato ppm)
(2) - caractere que delimita o fim da mensagem
(3) - nome do arquivo com a chave privada (n, d)
*/

int main(int argsCount, char **args) {
    if(verifyParametersToDecode(argsCount, args) == 0) { // se a funcao que verifica os parametros retornar 0, todos os valores estao corretos e o programa pode ser executado
        Blocks messageBlocks;
        char *asciiMessage, *decryptedMessage, *hiddenMsg;
        PPMImage *img;
        rsaNum n, d;
        
        getPrivateKey(args[3], &n, &d); // pega os valores da chave privada (n, d) do arquivo informado
		
		img = readImage(args[1]); // le a imagem PPM e armazena o conteudo dos pixels em uma variavel
		hiddenMsg = getHiddenMessageInImage(img, args[2][0]); // le os canais de cor dos pixels ate' encontrar o caractere delimitador e salva a mensagem criptografada em uma variavel
		
		messageBlocks = separateBlocksToDecrypt(hiddenMsg, n); // separa os blocos da mensagem criptografada
		decrypt(messageBlocks, d, n); // descriptografa os blocos
		asciiMessage = convertBlocksToString(messageBlocks, 2); // concatena os blocos descriptografados em uma string em ascii

		decryptedMessage = convertFromAscii(asciiMessage); // converte a string descriptografada em ascii para os caracteres originais
		printf("\n Decoded message: %s \n", decryptedMessage);
		
		free(asciiMessage);
		free(decryptedMessage);
		free(img->pixels);
		free(img);
    }
    return 0;
}
