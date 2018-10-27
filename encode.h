#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<tgmath.h>

/*
Gabriel Resende de Andrade

Esta biblioteca serve para criptografar, esconder, descriptografar e identificar uma mensagem em uma imagem de formato .ppm
O metodo de criptografia utilizado e' o RSA
*/

// utilizado para facilitar os testes com diferentes tipos de variaveis para a criptografia
typedef double rsaNum;

// estrutura de dados que contem um array de strings e o tamanho desse array, para facilitar a manipulacao dos blocos a serem criptografados
typedef struct blocks {
    char **strArray;
    int size;
} Blocks;

// estrutura de dados que representa cada pixel da imagem, com os 3 valores RGB para a cor
typedef struct {
	unsigned char red, green, blue;
} PPMPixel;

// estrutura de dados que representa a imagem inteira, com os valores da altura e largura, e um ponteiro para a sequencia de pixels
typedef struct {
	int height, width;
	PPMPixel *pixels;
} PPMImage;

/* funcoes para a criptografia e descriptografia */
rsaNum convertPrimeAndVerify(char *string); // recebe uma string, verifica se e' um numero primo e converte para o formato adequado (rsaNum) para fazer os calculos para a criptografia
char *convertBlocksToString(Blocks blocks, int op); // concatena os blocos em uma string, usando op como parametro para identificar se a string sera' usada na criptografia (1) ou na descriptografia (2)
char *convertFromAscii(char *str); // recebe uma string e converte seus os valores decimais para os caracteres correspondentes em ASCII
char *convertToAscii(char *str); // recebe uma string e converte seus caracteres para os valores decimais correspondentes em ASCII
void decrypt(Blocks blocksToDecrypt, rsaNum dKey, rsaNum n); // recebe uma estrutura de blocos a serem descriptografados, a chave privada (n, d), e descriptografa cada bloco retornando o valor na propria variavel
void encrypt(Blocks blocksToEncrypt, rsaNum eKey, rsaNum n); // recebe uma estrutura de blocos a serem criptografados, a chave publica (n, e), e criptografa cada bloco retornando o valor na propria variavel
rsaNum getKeyD(rsaNum totiente, rsaNum e); // recebe o totiente e a chave E e gera um valor para a chave D
rsaNum getKeyE(rsaNum totiente); // recebe o totiente e gera um valor para a chave E
void getPrivateKey(char *fileName, rsaNum *n, rsaNum *d); // recebe o nome do arquivo com a chave privada, um ponteiro para a chave N e um ponteiro para a chave D, e define o valor das chaves de acordo com o arquivo
rsaNum getMDC(rsaNum n1, rsaNum n2); // recebe dois numeros e retorna o maior divisor comum entre eles
rsaNum getPowMod(rsaNum base, rsaNum expo, rsaNum n); // recebe uma base, um expoente (d ou e), o numero para pegar o modulo e faz a exponenciacao modular, para que nao seja necessario fazer potencias grandes
void savePrivateKey(rsaNum n, rsaNum d); // salva a chave privada (N e D) no arquivo private.txt
Blocks separateBlocksToDecrypt(char *strToSplit, rsaNum n); // separa uma string em blocos menores que N para a descriptografia
Blocks separateBlocksToEncrypt(char *cryptedStr, rsaNum n); // separa uma string em blocos menores que N para a criptografia
void setMessageEnd(char *decryptedMsg, char *cryptedMsg); // concatena o ultimo caractere na string criptografada para delimitar o final
int verifyParametersToDecode(int argsCount, char **args); // verifica se todos os parametros passados para o decodificador estao corretos
int verifyParametersToEncode(int argsCount, char **args); // verifica se todos os parametros passados para o codificador estao corretos

/* funcoes para a esteganografia */
char *convertDecimalToBinary(int num); // converte um numero decimal para binario, retornando uma string de 8 bits
char *getHiddenMessageInImage(PPMImage *img, char end); // le a mensagem criptografada escondida em uma imagem ppm
void hideMessageInImage(PPMImage *img, char *str); // esteganografa a mensagem criptografada na imagem ppm
PPMImage *readImage(char *name); // le cada pixel do arquivo ppm
void writeNewImage(char *imageName, PPMImage *img); // salva uma imagem com a mensagem escondida

rsaNum convertPrimeAndVerify(char *string) {
    char* notNumber;
    rsaNum prime, aux;
    prime = strtol(string, &notNumber, 10); // converte a string para rsaNum (double) na base decimal, se houver caractere nao numerico, seu endereco sera passado para o ponteiro "notNumber"
    if (strlen(notNumber)) {
        prime = 0; // retorna 0 se a string contem letras
    } else if(prime < 2) {
        prime = 1; // retorna 1 se o numero e' menor que dois
    } else if (prime >= 2){
        aux = prime - 1;
        while(aux > 1) {
            if(fmod(prime, aux) == 0) {
                prime = 1; // retorna 1 se o numero for divisivel por outro numero (fora o proprio numero e 1), ou seja, nao e' primo
                break;
            }
            aux--;
        }
    }
    return prime;
}

char *convertBlocksToString(Blocks blocks, int op) {
    int i = 0, j = 0;
    char *finalString;
    finalString = (char*)calloc(blocks.size, sizeof(blocks.strArray[0]));
    if(op == 1) { // caso seja para criptografar, concatena os blocos em uma string, separando por "-"
        while(i < blocks.size) {
            strcat(finalString, blocks.strArray[i]);
            if(i < blocks.size - 1) {
                strcat(finalString, "-");
            }
            i++;
        }
    } else if(op == 2) { // caso seja para descriptografar, concatena os blocos diretamente em uma string
        while(i < blocks.size) {
            strcat(finalString, blocks.strArray[i]);
            i++;
        }
    }
    return finalString;
}

char *convertFromAscii(char *str) {
    int i = 0, asciiValue;
    char asciiChar[3], *originalStr;
    originalStr = (char*)calloc(1, strlen(str)/3);
    while(i < strlen(str)) { // percorre a string, de 3 em 3 posicoes, convertendo o valor de 3 digitos para um caractere
        asciiChar[0] = str[i];
        asciiChar[1] = str[i+1];
        asciiChar[2] = str[i+2];
        // usa uma string auxiliar para pegar as 3 posicoes, (todos os caracteres ascii sao definidos com 3 digitos antes da criptografia)
        sscanf(asciiChar, "%d", &asciiValue); // pega o valor das 3 posicoes juntas e coloca em uma variavel numerica
        memset(asciiChar, 0, 3); // limpa o array de caracteres para conter apenas o caractere ascii e entao concatenar na string completa
        asciiChar[0] = asciiValue;
        strcat(originalStr, asciiChar);
        i += 3;
    }
    return originalStr;
}

char *convertToAscii(char *str) {
    int i = 0, aux;
    char ascii[3];
    char *asciiStr;
    asciiStr = (char*)calloc(3 * sizeof(char), (strlen(str) + 1)); // aloca 3 posicoes do tipo char para cada caractere, pois cada um sera' representado por um numero ascii com 3 digitos
    asciiStr[0] = '\0';
    char zero[2] = {48, '\0'}; // string contendo apenas o caractere '0' para concatenar
    for(i=0; i<strlen(str); i++) { // percorre a string, convertendo caractere por caractere para o valor ascii
        memset(ascii, 0, 3);
        aux = (int)str[i]; // pega o valor ascii do caractere atual e converte para string
        sprintf(ascii, "%d", aux);
        if(strlen(ascii) < 3) { // se a string tem menos de 3 digitos, completa com zeros a esquerda
            if(strlen(ascii) == 2) {
                strcat(asciiStr, zero);
            } else if(strlen(ascii) == 1) {
                strcat(asciiStr, zero);
                strcat(asciiStr, zero);
            }
        } // as comparacoes garantem que o valor ascii na string sempre tenha 3 digitos, completando com zeros a esquerda se necessario
        strcat(asciiStr, ascii);
    }
    return asciiStr;
}

char *convertDecimalToBinary(int decimalNumber) {
	char *binStr, *result;
	binStr = (char*)calloc(9, sizeof(char));
	result = (char*)calloc(9, sizeof(char));
    int i = 0, j = 0;
    while(decimalNumber > 0) { // enquanto o numero for maior que 0, divide por 2 e pega os restos para a conversao bit a bit em binario
		binStr[i] = (decimalNumber % 2) + '0'; // os bits mais significantes sao obtidos primeiro, entao a string de bits ficara' invertida
        decimalNumber = decimalNumber / 2;
        i++;
    }
	for(j = 0; j < 8-i; j++) { // se o numero convertido tiver menos bits que 8, preenche a string com 0 a esquerda
		result[j] = (char)48;
	}
	for (; j < 8; j++) { // apos preencher com 0 a esquerda, coloca os numeros da string binaria de forma inversa (pois a mesma e' gerada invertida)
		result[j] = binStr[7 - j];
	}
	free(binStr);
	return result;
}

void decrypt(Blocks blocksToDecrypt, rsaNum dKey, rsaNum n) {
    int i = 0, j = 0, k = 0;
    rsaNum blockToDecrypt;
    char *decryptedBlock;
    decryptedBlock = (char*)calloc(1 ,sizeof(blocksToDecrypt.strArray[0]));
    while(i < blocksToDecrypt.size) { // percorre bloco por bloco
        sscanf(blocksToDecrypt.strArray[i], "%lf", &blockToDecrypt); // converte o bloco para double e passa o valor para a variavel auxiliar
        blockToDecrypt = getPowMod(blockToDecrypt, dKey, n); // eleva o bloco a D e faz modulo N, passando o resultado para a variavel auxiliar
        sprintf(decryptedBlock, "%.0lf", blockToDecrypt); // converte o resultado para string
        j = 0;
        if(48 == (int)blocksToDecrypt.strArray[i][0]) { // se o bloco comecar com um ou mais zeros, armazena previamente, uma vez que zeros a esquerda nao serao considerados na exponenciacao modular
            k = 0;
            while(j < strlen(blocksToDecrypt.strArray[i])) {
                if(48 != (int)blocksToDecrypt.strArray[i][j]) {
                    break;
                }
                j++;
            }
            while(j < strlen(blocksToDecrypt.strArray[i])) { // preenche as posicoes restantes com os valores do bloco descriptografado
                blocksToDecrypt.strArray[i][j] = decryptedBlock[k];
                j++;
                k++;
            }
        } else {
            memset(blocksToDecrypt.strArray[i], 0, strlen(blocksToDecrypt.strArray[i])); // limpa a string destino antes de concatenar
            strcat(blocksToDecrypt.strArray[i], decryptedBlock); // concatena o valor do bloco descriptografado
        }
        memset(decryptedBlock, 0, sizeof(decryptedBlock)); // limpa a variavel auxiliar do bloco descriptografado para que nao hajam caracteres excedentes na proxima conversao
        i++;
    }
    free(decryptedBlock);
}

void encrypt(Blocks blocksToEncrypt, rsaNum eKey, rsaNum n) {
    int i = 0, j = 0, k = 0;
    rsaNum blockToEncrypt;
    char *cryptedBlock;
    cryptedBlock = (char*)calloc(1, sizeof(blocksToEncrypt.strArray[0]));
    while(i < blocksToEncrypt.size) { // percorre bloco por bloco
        sscanf(blocksToEncrypt.strArray[i], "%lf", &blockToEncrypt); // converte o bloco para double e passa o valor para a variavel auxiliar
        blockToEncrypt = getPowMod(blockToEncrypt, eKey, n); // eleva o bloco a E e faz modulo N, passando o resultado para a variavel auxiliar
        sprintf(cryptedBlock, "%.0lf", blockToEncrypt);
        j = 0;
        if(48 == (int)blocksToEncrypt.strArray[i][0]) { // se o bloco comecar com um ou mais zeros, armazena previamente, uma vez que zeros a esquerda nao serao considerados na exponenciacao modular
            while(j < strlen(blocksToEncrypt.strArray[i])) {
                if(48 != (int)blocksToEncrypt.strArray[i][j]) {
                    break;
                }
                j++;
            }
            k = 0;
            while(j <= strlen(cryptedBlock)) { // preenche as posicoes restantes com os valores do bloco criptografado
                blocksToEncrypt.strArray[i][j] = cryptedBlock[k];
                j++;
                k++;
            }
        } else {
            memset(blocksToEncrypt.strArray[i], 0, strlen(blocksToEncrypt.strArray[i])); // limpa a string destino antes de concatenar
            strcat(blocksToEncrypt.strArray[i], cryptedBlock); // concatena o valor do bloco criptografado
        }
        memset(cryptedBlock, 0, sizeof(cryptedBlock)); // limpa a variavel auxiliar do bloco criptografado para que nao hajam caracteres excedentes na proxima conversao
        i++;
    }
    free(cryptedBlock);
}

rsaNum getKeyD(rsaNum totiente, rsaNum e) { // pega a chave D pelo algoritmo de euclides estendido
	rsaNum beta = 0, alpha = 1, beta1 = 1, alpha1 = 0, iniTot, iniE;
	rsaNum totAux, betaAux, alphaAux, q; // variaveis auxiliares para efetuar trocas
	iniTot = totiente;
	iniE = e;
	while(e != 0) {
		q = floor(totiente / e); // pega apenas a parte inteira da divisao do totiente por E
		totAux = totiente;
		alphaAux = alpha;
		betaAux = beta;
		totiente = e;
		alpha = alpha1;
		beta = beta1;
		e = totAux - (q * e);
		alpha1 = alphaAux - (q * alpha);
		beta1 = betaAux - (q * beta1);
	}
    if(beta == iniE) { // faz as correcoes finais se necessario
        if(alpha < 0) {
            alpha += iniTot;
        }
        return alpha;
    } else {
        if(beta < 0) {
            beta += iniTot;
        }
        return beta;
    }
}

rsaNum getKeyE(rsaNum totiente) {
    rsaNum e = 2;
    while((getMDC(e, totiente) > 1) && (e < totiente)) { // pega o menor numero que nao tenha divisores em comum com o totiente
        e++;
    }
    return e;
}

void getPrivateKey(char *fileName, rsaNum *n, rsaNum *d) {
    FILE *privateKeyFile;
	privateKeyFile = fopen(fileName, "rb");
	if(privateKeyFile == NULL) {
		printf("\n Erro ao abrir o arquivo com a chave privada.\n");
	} else {
		if(fscanf(privateKeyFile, "%lf\n%lf", n, d) != 2) { // tenta ler as 2 linhas com as variaveis n e d respectivamente e retorna erro se nao for possivel
			printf("\n Arquivo com a chave privada invalido.\n");
			exit(1);
		}
	}
}

rsaNum getMDC(rsaNum n1, rsaNum n2) {
    int rem = fmod(n1, n2);
    while(rem != 0) { // enquanto o resto nao for zero, continua obtendo o resto das divisoes
        n1 = n2;
        n2 = rem;
        rem = fmod(n1, n2);
    }
    return n2; // retorna o ultimo resto antes de 0
}

rsaNum getPowMod(rsaNum base, rsaNum expo, rsaNum n) { // faz exponenciacao modular para evitar perda de precisao ao elevar numeros grandes
    rsaNum result = 1, pot;
    pot = fmod(base, n);
    for (; expo > 0; floor(expo /= 2)) {
        if(fmod(expo, 2) == 1) {
			result = fmod((result * pot), n);
		}
        pot = fmod((pot * pot), n);
    }
    return result;
}

char *getHiddenMessageInImage(PPMImage *img, char end) {
	int i = 0, j = 0, asciiValue = 0, msgSize = 2, pixelCount = 0;
	char *rgbBitStr, *charBitStr, *str, *hiddenChar;
	charBitStr = (char*)calloc(9, sizeof(char));
	str = (char*)calloc(msgSize, sizeof(char));
	hiddenChar = (char*)calloc(2, sizeof(char));
	while(asciiValue != (int)end && i < (img->height * img->width)) { // percorre a imagem de 3 em 3 pixels, pegando 8 bits menos significativos dos canais de cor desses
		memset(charBitStr, 0, (8 * sizeof(char)));
		// se um numero for par, seu ultimo digito sera 0; se for impar, o ultimo digito sera 1. Portanto, basta pegar o modulo da divisao do numero por 2
		for(j = 0; j < 3; j++) {
			charBitStr[j] = (img->pixels[pixelCount+j].red % 2) + '0'; // armazena os bits menos significativo dos canais vermelhos de 3 pixels
		}
		for(; j < 6; j++) {
			charBitStr[j] = (img->pixels[pixelCount-3+j].green % 2) + '0'; // armazena os bits menos significativo dos canais verdes de 3 pixels
		}
		for(; j < 8; j++) {
			charBitStr[j] = (img->pixels[pixelCount-6+j].blue % 2) + '0'; // armazena os bits menos significativo dos canais azuis de 2 pixels
		}
		asciiValue = strtol(charBitStr, NULL, 2); // converte a string binaria para o valor decimal
		if(asciiValue != (int)end) {
			str = (char*)realloc(str, ++msgSize); // aloca mais memoria para acrescentar o caractere
			sprintf(hiddenChar, "%c", asciiValue);
			strcat(str, hiddenChar);
			pixelCount += 3;
		}
		i++;
	}
	if(i >= (img->height * img->width)) {
		printf("\n Nao foi possivel encontrar o caractere delimitador na imagem informada.");
		exit(1);
	}
	return str;
}

void hideMessageInImage(PPMImage *img, char *str) { // altera os primeiros pixels, mudando os digitos menos significativos dos canais de cores com os digitos da string criptografada
    int i = 0, j = 0, pixelCount = 0;
	char *rgbBitStr, *charBitStr, *notNumber;
	for(; i < strlen(str); i++) { // percorre os pixels necessarios de acordo com o tamanho da string
		charBitStr = convertDecimalToBinary((int)str[i]);
		for(j = 0; j < 3; j++) {
			rgbBitStr = convertDecimalToBinary(img->pixels[pixelCount+j].red);
			
			rgbBitStr[7] = charBitStr[j]; // preenche o bit menos significativo do canal vermelho de 3 pixels com os 3 primeiros bits do caractere criptografado
			
			img->pixels[pixelCount+j].red = strtol(rgbBitStr, &notNumber, 2);
		}
		for(; j < 6; j++) {
			rgbBitStr = convertDecimalToBinary(img->pixels[pixelCount-3+j].green);
			
			rgbBitStr[7] = charBitStr[j]; // preenche o bit menos significativo do canal verde de 3 pixels com os 3 bits seguintes do caractere criptografado
			
			img->pixels[pixelCount-3+j].green = strtol(rgbBitStr, &notNumber, 2);
		}
		for(; j < 8; j++) {
			rgbBitStr = convertDecimalToBinary(img->pixels[pixelCount-6+j].blue);
			
			rgbBitStr[7] = charBitStr[j]; // preenche o bit menos significativo do canal azul de 2 pixels com os 2 ultimos bits do caractere criptografado
			
			img->pixels[pixelCount-6+j].blue = strtol(rgbBitStr, &notNumber, 2);
		}
		pixelCount += 3;
	}
}

PPMImage *readImage(char *name) {
	char format[3];
	PPMImage *img;
	FILE *imageFile;
	int rgbComponent;
	imageFile = fopen(name, "rb"); // abre o arquivo para leitura binaria e verifica se ocorreu erro
	if(imageFile == NULL) {
		printf("\n Erro ao abrir o arquivo de imagem %s.\n", name);
		exit(1);
	}
	if(!fgets(format, sizeof(format), imageFile)) { // le a primeira linha, que contem o formato da imagem
		printf("\n Arquivo invalido ou corrompido.\n");
		exit(1);
	}
    if(format[0] != 'P' || (format[1] != '6' && format[1] != '3')) { // verifica o formato da imagem, contido na primeira linha. O formato esperado e' 'P6' ou 'P3'
        printf("\n Formato de arquivo invalido (P6 ou P3 esperado).\n");
        exit(1);
    }
    img = (PPMImage*)calloc(1, sizeof(PPMImage));
    if(fscanf(imageFile, "%d %d", &img->height, &img->width) != 2) { // verifica se a linha seguinte contem a altura e largura da imagem
		printf("\n Informacoes de tamanho de imagem invalidas.\n");
		exit(1);
    }
    if(fscanf(imageFile, "%d", &rgbComponent) != 1) { // verifica se a linha seguinte contem o tamanho do canal de cor RGB
		printf("\n Componente de cor RGB invalido.\n");
		exit(1);
    }
    if(rgbComponent != 255) { // verifica se o canal de cor RGB tem tamanho de 8-bits (0 a 255)
		printf("\n Tamanho do canal de cor diferente de 8-bits.\n");
		exit(1);
    }
    while(fgetc(imageFile) != '\n'); // percorre a linha ate' encontrar um caractere '\n'
    img->pixels = (PPMPixel*)calloc(img->height * img->width, sizeof(PPMPixel)); // aloca memoria equivalente ao numero de pixels
    fread(img->pixels, 3 * img->height, img->width, imageFile); // le os pixels da imagem
    fclose(imageFile);
    return img;
}

void savePrivateKey(rsaNum n, rsaNum d) {
	FILE *privateKeyFile;
	privateKeyFile = fopen("private.txt", "wb"); // verifica se e' possivel criar um arquivo de texto e salva a chave privada (n, d)
	if (privateKeyFile == NULL) {
		printf("\n Erro ao criar o arquivo para salvar a chave privada.\n");
	} else {
		fprintf(privateKeyFile, "%.0lf\n", n);
		fprintf(privateKeyFile, "%.0lf\n", d);
	}
	fclose(privateKeyFile);
}

Blocks separateBlocksToEncrypt(char *strToSplit, rsaNum n) { // separa a string em blocos menores que N para ser criptografada
    char *nStr, **blockStr, *auxBlock;
    Blocks splitedStr;
    int i = 0, j, count = 0, lastPosFlag, strBlockLen;
    nStr = (char*)calloc(1, sizeof(n));
    auxBlock = (char*)calloc(1, strlen(nStr));
    sprintf(nStr, "%.0lf" , n); // passa o valor de n para uma string
    strBlockLen = strlen(strToSplit) / (strlen(nStr) - 1); // define o numero maximo de blocos baseado no tamanho da string e no tamanho de N
    blockStr = (char**)calloc((strBlockLen + 1), sizeof(char*)); // aloca o tamanho necessario para um array de blocos (array de strings ou de ponteiros char), baseado no tamanho da string e no tamanho de N
    
    for(j = 0; j <= (strBlockLen); j++) { // aloca o tamanho necessario para cada bloco baseado no tamanho de N
        blockStr[j] = (char*)calloc(1, strlen(nStr));
    }
    while(i < strlen(strToSplit)) {
        j = i;
        if((i + strlen(nStr)) <= (strlen(strToSplit))) { // verifica se nao esta' no final da string, com tamanho restante menor que o numero de digitos de N
            if((strToSplit[i] + '0') > (nStr[0] + '0')) { // compara se o primeiro digito e' maior que o primeiro digito de N, se sim, o bloco a ser formado deve ter um numero a menos que o numero de digitos de N
                i += strlen(nStr) - 1;
                for(; j < i; j++) {
                    auxBlock[j-i+strlen(nStr)-1] = strToSplit[j]; // preenche a string auxiliar com o bloco menor que N
                }
            } else if((strToSplit[i] + '0') == (nStr[0] + '0')) { // se o primeiro digito for igual ao primeiro digito de N, continua comparando ate' achar um numero maior ou menor
                lastPosFlag = 0; // reseta a flag
                j++;
                auxBlock[0] = strToSplit[i]; // define o primeiro digito do bloco antes de comparar o resto
                i += strlen(nStr);
                for(; j < i; j++) {
                    if((strToSplit[j] + '0') > (nStr[j-i+strlen(nStr)] + '0')) { // se o numero for maior, diminui o tamanho do bloco
                        if(j == i-1) {
                            lastPosFlag = 1; // define uma veriavel de flag para apontar que e' a ultima posicao
                        }
                        i--;
                        break;
                    } else if((strToSplit[j] + '0') < (nStr[j-i+strlen(nStr)] + '0')) { // se o numero for menor, o bloco pode ter a mesma quantidade de digitos que N
                        break;
                    } else if(j == i - 1 && (strToSplit[j] + '0') >= (nStr[j-i+strlen(nStr)] + '0')) { // se percorrer ate' o ultimo digito possivel, e todos sao iguais aos digitos de N, o bloco devera' ter um digito a menos para ser menor que N
						lastPosFlag = 1; // define uma veriavel de flag para apontar que e' a ultima posicao
                        i--;
					}
                }
                if(lastPosFlag != 1) {
                    j = i - strlen(nStr) + 1; // se a flag nao estiver definida como 1, volta j para a posicao inicial do bloco analisado no momento, para preencher uma string auxiliar
                }
                for(; j < i; j++) {
                    auxBlock[j-i+strlen(nStr)] = strToSplit[j]; // preenche a string auxiliar com o bloco menor que N
                }
            } else if((strToSplit[i] + '0') < (nStr[0] + '0')) { // compara se o primeiro digito e' menor que o primeiro digito de N, se sim, pode ser formado um bloco com o mesmo numero de digitos que N
                i += strlen(nStr);
                for(; j < i; j++) {
                    auxBlock[j-i+strlen(nStr)] = strToSplit[j]; // preenche a string auxiliar com o bloco menor que N
                }
            }
        } else {
            for(j = i; j < strlen(strToSplit); j++) { // caso seja o final da string, com menos digitos que N, completa um bloco sem fazer comparacoes, pois sempre sera' menor
                auxBlock[(j-i)] = strToSplit[j];
            }
            i = strlen(strToSplit); // passa para a posicao da string seguinte ao bloco separado
        }
        strcpy(blockStr[count], auxBlock); // preenche o bloco com a string auxiliar obtida com as comparacoes
        count++;
        memset(auxBlock, 0, sizeof(auxBlock));
    }
    free(auxBlock);
    free(nStr);
    splitedStr.size = count;
    splitedStr.strArray = blockStr;
    if(count < strBlockLen) { // caso existam menos strings preenchidas que o previsto, libera o espaco de memoria alocado excedente
        for(; count <= strBlockLen; count++) {
            free(blockStr[count]);
        }
    }
    return splitedStr;
}

Blocks separateBlocksToDecrypt(char *strToSplit, rsaNum n) {
    int i = 0, j = 0, k = 0, count = 0, strBlockLen;
    Blocks splitedStr;
    char *nStr, **blockStr;
    nStr = (char*)calloc(1, sizeof(n));
    sprintf(nStr, "%.0lf" , n); // passa o valor de n para uma string
    strBlockLen = (strlen(strToSplit) / (strlen(nStr)-1)) + strlen(nStr); // define o numero maximo de blocos baseado no tamanho da string e no tamanho de N
    blockStr = (char**)calloc((strBlockLen), sizeof(char*));  // aloca o tamanho necessario para um array de blocos (array de strings ou de ponteiros char), baseado no tamanho da string e no tamanho de N
    for(j = 0; j <= (strBlockLen); j++) { // aloca o tamanho necessario para cada bloco baseado no tamanho de N
        blockStr[j] = (char*)calloc(1, strlen(nStr));
    }
    free(nStr);
    j = 0;
    while(i < strlen(strToSplit)) {
        if(strToSplit[i] == (char)45) { // separa os blocos quando encontra um caractere igual a '-''
            for(k = 0; j < i; j++, k++) { // passa os digitos anteriores ao caractere '-' para um bloco
                blockStr[count][k] = strToSplit[j];
            }
            count++;
            j++;
        }
        i++;
    }
    for(k = 0; j < i; j++, k++) { // passa os ultimos digitos para um bloco, pois nao ha '-' delimitando o final do bloco na string
        blockStr[count][k] = strToSplit[j];
    }
    count++;
    splitedStr.strArray = blockStr;
    splitedStr.size = count;
    return splitedStr;
}

void setMessageEnd(char *decryptedMsg, char *cryptedMsg) {
	char *end;
	end = (char*)calloc(2, sizeof(char)); // concatena o caractere que delimita o final da mensagem na string criptografada
	end[0] = decryptedMsg[strlen(decryptedMsg)-1];
	strcat(cryptedMsg, end);
}

int verifyParametersToDecode(int argsCount, char **args) {
    if(argsCount > 3) { // verifica se todos os 3 parametros foram informados
        FILE *fp;
		fp = fopen(args[1], "rb"); // verifica se o arquivo de imagem informado e' valido
		if(fp == NULL) {
			printf("\n Erro ao abrir o arquivo de imagem %s.\n", args[1]);
		} else {
			fclose(fp);
			fp = fopen(args[3], "rb"); // verifica se o arquivo de chave privada informado e' valido
			if(fp == NULL) {
				printf("\n Erro ao abrir o arquivo com a chave privada.\n");
			} else {
				fclose(fp);
				if(strlen(args[2]) == 1) { // verifica se o parametro com delimitador de mensagem esta' correto
					return 0;
				} else {
					printf("\n Caractere delimitador de fim da mensagem invalido.\n");
				}
			}
		}
    } else {
        printf("\n Os 3 parametros devem ser informados.\n");
    }
    exit(1);
}

int verifyParametersToEncode(int argsCount, char **args) {
    if(argsCount > 5) { // verifica se todos os 5 parametros foram informados
        rsaNum p;
        p = convertPrimeAndVerify(args[4]);
        if(p == 0) { // se foi passado algum valor nao numerico
            printf("\n Apenas numeros inteiros primos devem ser informados no primeiro parametro para P.\n");
        } else if(p == 1) { // se o valor informado para P nao e' primo
            printf("\n O numero P informado nao e' primo.\n");
        } else {
            rsaNum q;
            q = convertPrimeAndVerify(args[5]);
            if(q == 0) { // se foi passado algum valor nao numerico
                printf("\n Apenas numeros inteiros primos devem ser informados no segundo parametro para Q.\n");
            } else if(q == 1) { // se o valor informado para Q nao e' primo
                printf("\n O numero Q informado nao e' primo.\n");
            } else if(q == p) { // P e Q nao podem ser iguais
                printf("\n Os numeros P e Q devem ser diferentes.\n");
            } else if(q <= 3 && p <= 3) { // P e Q nao podem ser 2 e 3, pois assim N seria 6 e o totiente seria 2, nao existindo possibilidade de valor para a chave E e consequentemente nem para a chave D
                printf("\n Os numeros P e Q nao podem ser 2 e 3.\n");
            } else {
                FILE *imageToRead;
				imageToRead = fopen(args[1], "r");
				if(imageToRead == NULL) { // verifica se o arquivo de imagem informado e' valido
					printf("\n Erro ao abrir o arquivo de imagem %s.\n", args[1]);
				} else {
					fclose(imageToRead);
					return 0;
				}
            }
        }
    } else {
        printf("\n Os 5 parametros devem ser informados.\n");
    }
    exit(1);
}

void writeNewImage(char *imageName, PPMImage *img) {
    FILE *newImageFile;
    newImageFile = fopen(imageName, "wb");
    if(newImageFile == NULL) {
		printf("\n Erro ao criar o arquivo de imagem ppm.\n");
		exit(1);
    }
    fprintf(newImageFile, "P6\n"); // escreve o formato da imagem
    fprintf(newImageFile, "%d %d\n", img->height, img->width); // escreve a altura e largura da imagem
    fprintf(newImageFile, "%d\n", 255); // escreve o tamanho do canal de cor RGB, no caso 255 - 8 bits
    fwrite(img->pixels, (3 * img->height), img->width, newImageFile); // escreve os pixels salvos
    fclose(newImageFile);
}