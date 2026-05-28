# Plano de implementação — mini-bank.c

## 1) Estrutura e dados
- Criar struct `Conta` com:
  - id (string)
  - saldo (float)
  - senha (string)
  - histórico (vetor de strings)
- Constantes: limites de contas e tamanho do histórico/linha.

## 2) Persistência em arquivo .txt
- Criar `dados.txt` (ou `mini-bank-data.txt`) para salvar:
  - quantidade de contas
  - para cada conta: id|senha|saldo|numTransacoes
  - linhas do histórico
- Funções: `carregar_contas()` e `salvar_contas()`.

## 3) UI no terminal
- `print_banner()`, `print_sep()`, `limpar_tela()` (Windows).
- Validação simples de entrada.

## 4) Login e cadastro (múltiplas contas)
- Antes de logar: mostrar menu inicial:
  - 1) Login
  - 2) Cadastrar conta
  - 3) Sair
- Cadastro:
  - pedir ID, senha (oculta), saldo inicial (opcional ou padrão)
  - impedir ID repetido
- Login:
  - pedir ID e senha (oculta)
  - buscar conta correspondente

## 5) Menu verdadeiro (operações)
- Loop após login:
  - 1) Saque
  - 2) Depósito
  - 3) Transferência
  - 4) Histórico
  - 5) Salvar e sair
- Cada operação registra transação no histórico.

## 6) Separação de funções
- Funções dedicadas:
  - `fazer_saque(Conta*, valor)`
  - `fazer_deposito(Conta*, valor)`
  - `fazer_transferencia(conta_origem, contas, destino_id, valor)`
  - `registrar_transacao(Conta*, texto)`

## 7) Esconder senha (Windows)
- Implementar leitura de senha com `getch()` do `<conio.h>`.

## 8) Teste
- Rodar e testar:
  - cadastro + persistência
  - saque/depósito
  - transferência entre contas
  - histórico acumulando

