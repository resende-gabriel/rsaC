#include<stdio.h>
#include"encode.h"

/*
Autor: Gabriel Resende de Andrade

Este programa serve para criptografar pelo metodo RSA e esteganografar uma mensagem em uma imagem

Deve ser rodado pelo terminal, e recebe 5 parametros
(1) - nome da imagem para a esteganografia (no formato ppm)
(2) - mensagem a ser criptografada e escondida, com algum caractere que delimita o fim
(3) - nome da imagem que sera' salva com a mensagem esteganografada (no formato ppm)
(4) - numero p para a criptografia RSA
(5) - numero q para a criptografia RSA
*/

int main(int argsCount, char **args) {
    if(verifyParametersToEncode(argsCount, args) == 0) { // se a funcao que verifica os parametros retornar 0, todos os valores estao corretos e o programa pode ser executado
        Blocks messageBlocks;
        char *asciiMessage, *cryptedMessage;
        PPMImage *img;
        rsaNum p, q, n, totiente, d, e;
        
        p = convertPrimeAndVerify(args[4]); // verifica se o valor inserido para o número P e' valido e converte para double
        q = convertPrimeAndVerify(args[5]); // verifica se o valor inserido para o número Q e' valido e converte para double
        n = p * q;
        totiente = (p - 1) * (q - 1);
        e = getKeyE(totiente); // gera a chave E baseado no valor do totiente
        d = getKeyD(totiente, e); // gera a chave D baseado nos valores do totiente e da chave E
		
		asciiMessage = convertToAscii(args[2]); // converte os caracteres da mensagem a ser criptografada para seus respectivos valores ascii (todos com 3 digitos, incluindo zeros a esquerda)
		messageBlocks = separateBlocksToEncrypt(asciiMessage, n); // divide a mensagem em ascii em blocos com valores menores que N
		encrypt(messageBlocks, e, n); // criptografa os blocos
		cryptedMessage = convertBlocksToString(messageBlocks, 1); // concatena os blocos criptografados separados com '-' (o segundo parametro identifica que e' para criptografia)
		setMessageEnd(args[2], cryptedMessage); // coloca o ultimo digito no final da mensagem para que seja possivel identificar o fim na imagem
		savePrivateKey(n, d); // salva a chave privada em um arquivo
		
		img = readImage(args[1]); // le a imagem PPM e armazena o conteudo dos pixels em uma variavel
		
		hideMessageInImage(img, cryptedMessage); // grava a mensagem criptografada nos pixels da imagem
		writeNewImage(args[3], img); // salva a nova imagem com a mensagem escondida
		
		free(asciiMessage);
		free(cryptedMessage);
		free(img->pixels);
		free(img);
    }
    return 0;
}
