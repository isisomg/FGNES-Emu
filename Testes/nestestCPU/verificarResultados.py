PATH_RES_ESPERADOS = "resultadosEsperados.txt"
PATH_RES_OBTIDOS = "resultadosObtidos.txt"

def lerArquivo(path):
    with open(path, "r") as arq:
        linhas = arq.readlines()
        for i in range(len(linhas)):
            linhas[i] = linhas[i].replace("\n", "")
        return linhas

def filtrarEsperados():
    linhas = lerArquivo(PATH_RES_ESPERADOS)
    for i in range(len(linhas)):
        linhas[i] = linhas[i][0:4] + " " + linhas[i][48:73]
    return linhas

def comparar(esperado, obtido):
    for i in range(len(obtido)): # obtido eh menor ou mesmo tamanho de esperado
        print(esperado[i] + "  -  ", end="")
        if (esperado[i] == obtido[i]):
            print("OK")
        else:
            print("FALHA -> " + obtido[i])

esperado = filtrarEsperados()
obtido = lerArquivo(PATH_RES_OBTIDOS)
comparar(esperado, obtido)
