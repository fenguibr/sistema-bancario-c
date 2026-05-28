#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>
#endif

#define MAX_CONTAS 50
#define ID_TAM 20
#define SENHA_TAM 20
#define HIST_TAM 300
#define MAX_TRANSACOES 50
#define LINHA_TAM 256

#define DATA_FILE "dados.txt"

typedef struct {
    char id[ID_TAM];
    float saldo;
    char senha[SENHA_TAM];
    char historico[MAX_TRANSACOES][HIST_TAM];
    int qtd_transacoes;
} Conta;

static void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

static void print_sep(char c, int n) {
    for (int i = 0; i < n; i++) putchar(c);
    putchar('\n');
}

static void print_banner(const char *titulo) {
    print_sep('=', 60);
    printf("%s\n", titulo);
    print_sep('=', 60);
}

static void ler_string_sem_quebra(char *out, int tam) {
    if (!fgets(out, tam, stdin)) return;
    size_t len = strlen(out);
    if (len > 0 && out[len - 1] == '\n') out[len - 1] = '\0';
}

static void ler_senha_oculta(char *out, int tam) {
#ifdef _WIN32
    int idx = 0;
    out[0] = '\0';

    while (1) {
        int ch = _getch();
        if (ch == '\r' || ch == '\n') {
            out[idx] = '\0';
            break;
        }
        if (ch == 8) { // backspace
            if (idx > 0) {
                idx--;
                out[idx] = '\0';
                // apaga um '*' no terminal
                printf("\b \b");
            }
            continue;
        }

        if (idx < tam - 1) {
            out[idx++] = (char)ch;
            putchar('*');
        }
    }
    putchar('\n');
#else
    // fallback: visível
    printf("(Digite a senha): ");
    fflush(stdout);
    ler_string_sem_quebra(out, tam);
#endif
}

static int carregar_contas(Conta contas[], int *qtd) {
    FILE *f = fopen(DATA_FILE, "r");
    if (!f) {
        *qtd = 0;
        return 0;
    }

    int n = 0;
    if (fscanf(f, "%d\n", &n) != 1) {
        fclose(f);
        *qtd = 0;
        return 0;
    }
    if (n < 0) n = 0;
    if (n > MAX_CONTAS) n = MAX_CONTAS;

    for (int i = 0; i < n; i++) {
        Conta c;
        memset(&c, 0, sizeof(c));

        // linha esperada:
        // id|senha|saldo|qtd_transacoes
        char id[ID_TAM] = {0};
        char senha[SENHA_TAM] = {0};
        float saldo = 0.0f;
        int qtdt = 0;

        char linha[LINHA_TAM];
        if (!fgets(linha, sizeof(linha), f)) break;

        // tenta parsing com delimitadores
        // remove '\n' depois
        size_t len = strlen(linha);
        if (len > 0 && linha[len - 1] == '\n') linha[len - 1] = '\0';

        // parsing manual por '|'
        char *p1 = strchr(linha, '|');
        if (!p1) break;
        *p1 = '\0';
        strcpy(id, linha);

        char *p2 = strchr(p1 + 1, '|');
        if (!p2) break;
        *p2 = '\0';
        strcpy(senha, p1 + 1);

        char *p3 = strchr(p2 + 1, '|');
        if (!p3) break;
        *p3 = '\0';
        saldo = (float)atof(p2 + 1);

        qtdt = atoi(p3 + 1);

        strncpy(c.id, id, ID_TAM - 1);
        strncpy(c.senha, senha, SENHA_TAM - 1);
        c.saldo = saldo;
        c.qtd_transacoes = 0;

        // ler histórico (qtdt linhas)
        for (int t = 0; t < qtdt && t < MAX_TRANSACOES; t++) {
            char ht[HIST_TAM];
            if (!fgets(ht, sizeof(ht), f)) ht[0] = '\0';
            size_t htlen = strlen(ht);
            if (htlen > 0 && ht[htlen - 1] == '\n') ht[htlen - 1] = '\0';
            strncpy(c.historico[t], ht, HIST_TAM - 1);
            c.historico[t][HIST_TAM - 1] = '\0';
            c.qtd_transacoes++;
        }

        contas[i] = c;
    }

    fclose(f);
    *qtd = n;
    return 1;
}

static int salvar_contas(Conta contas[], int qtd) {
    FILE *f = fopen(DATA_FILE, "w");
    if (!f) return 0;

    fprintf(f, "%d\n", qtd);
    for (int i = 0; i < qtd; i++) {
        Conta *c = &contas[i];
        fprintf(f, "%s|%s|%.2f|%d\n", c->id, c->senha, c->saldo, c->qtd_transacoes);
        for (int t = 0; t < c->qtd_transacoes; t++) {
            fprintf(f, "%s\n", c->historico[t]);
        }
    }

    fclose(f);
    return 1;
}

static int buscar_conta_por_id(Conta contas[], int qtd, const char *id) {
    for (int i = 0; i < qtd; i++) {
        if (strcmp(contas[i].id, id) == 0) return i;
    }
    return -1;
}

static void registrar_transacao(Conta *c, const char *texto) {
    if (c->qtd_transacoes >= MAX_TRANSACOES) {
        // simples: descartar a mais antiga ao invés de realocar
        for (int i = 1; i < c->qtd_transacoes; i++) {
            strcpy(c->historico[i - 1], c->historico[i]);
        }
        c->qtd_transacoes = MAX_TRANSACOES - 1;
    }
    strncpy(c->historico[c->qtd_transacoes], texto, HIST_TAM - 1);
    c->historico[c->qtd_transacoes][HIST_TAM - 1] = '\0';
    c->qtd_transacoes++;
}

static int fazer_saque(Conta *c, float valor) {
    if (valor <= 0.0f) return 0;
    if (valor > c->saldo) return -1;

    c->saldo -= valor;

    char msg[HIST_TAM];
    snprintf(msg, sizeof(msg), "Saque: R$ %.2f | Saldo: R$ %.2f", valor, c->saldo);
    registrar_transacao(c, msg);

    return 1;
}

static int fazer_deposito(Conta *c, float valor) {
    if (valor <= 0.0f) return 0;

    c->saldo += valor;

    char msg[HIST_TAM];
    snprintf(msg, sizeof(msg), "Deposito: R$ %.2f | Saldo: R$ %.2f", valor, c->saldo);
    registrar_transacao(c, msg);

    return 1;
}

static int fazer_transferencia(Conta contas[], int qtd, Conta *origem, const char *destino_id, float valor) {
    if (valor <= 0.0f) return 0;
    if (!destino_id || destino_id[0] == '\0') return 0;

    int idx_dest = buscar_conta_por_id(contas, qtd, destino_id);
    if (idx_dest < 0) return -2; // destino não existe
    if (strcmp(origem->id, destino_id) == 0) return -3; // mesma conta

    if (valor > origem->saldo) return -1; // saldo insuficiente

    Conta *destino = &contas[idx_dest];

    origem->saldo -= valor;
    destino->saldo += valor;

    char msg_o[HIST_TAM];
    snprintf(msg_o, sizeof(msg_o), "Transferencia para %s: R$ %.2f | Saldo: R$ %.2f", destino->id, valor, origem->saldo);
    registrar_transacao(origem, msg_o);

    char msg_d[HIST_TAM];
    snprintf(msg_d, sizeof(msg_d), "Transferencia de %s: R$ %.2f | Saldo: R$ %.2f", origem->id, valor, destino->saldo);
    registrar_transacao(destino, msg_d);

    return 1;
}

static void mostrar_historico(const Conta *c) {
    print_sep('-', 60);
    printf("Histórico de transações — conta %s\n", c->id);
    print_sep('-', 60);

    if (c->qtd_transacoes == 0) {
        printf("(Sem transações ainda)\n");
        return;
    }

    for (int i = 0; i < c->qtd_transacoes; i++) {
        printf("%d) %s\n", i + 1, c->historico[i]);
    }
}

static void ler_float(float *out) {
    char linha[64];
    while (1) {
        if (!fgets(linha, sizeof(linha), stdin)) continue;
        if (linha[0] == '\n') continue;
        char *end = NULL;
        float v = (float)strtod(linha, &end);
        if (end != linha) {
            *out = v;
            return;
        }
        printf("Valor invalido. Digite novamente: ");
    }
}

static float ler_valor_prompt(const char *prompt) {
    printf("%s", prompt);
    fflush(stdout);

    // limpa possíveis restos do scanf anterior
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        // descarta
    }

    float v = 0.0f;
    ler_float(&v);
    return v;
}

static int cadastrar_conta(Conta contas[], int *qtd) {
    if (*qtd >= MAX_CONTAS) {
        printf("Limite de contas atingido (%d).\n", MAX_CONTAS);
        return 0;
    }

    char id[ID_TAM];
    char senha[SENHA_TAM];

    printf("\n=== Cadastro de conta ===\n");
    printf("ID da conta (até %d caracteres): ", ID_TAM - 1);
    fgets(id, sizeof(id), stdin);
    // remove \n
    size_t len = strlen(id);
    if (len > 0 && id[len - 1] == '\n') id[len - 1] = '\0';

    if (id[0] == '\0') {
        printf("ID invalido.\n");
        return 0;
    }

    if (buscar_conta_por_id(contas, *qtd, id) >= 0) {
        printf("Esse ID já existe.\n");
        return 0;
    }

    printf("Senha (oculta): ");
    ler_senha_oculta(senha, SENHA_TAM);
    if (senha[0] == '\0') {
        printf("Senha invalida.\n");
        return 0;
    }

    float saldo_inicial = 1000.0f;
    printf("Saldo inicial padrão é R$ 1000. Deseja alterar? (s/n): ");
    char resp[8];
    fgets(resp, sizeof(resp), stdin);

    if (resp[0] == 's' || resp[0] == 'S') {
        saldo_inicial = ler_valor_prompt("Digite o saldo inicial: ");
        if (saldo_inicial < 0.0f) {
            printf("Saldo inicial invalido.\n");
            return 0;
        }
    } else {
        // consome newline antes de próxima leitura se necessário
    }

    Conta *c = &contas[*qtd];
    memset(c, 0, sizeof(Conta));

    strncpy(c->id, id, ID_TAM - 1);
    strncpy(c->senha, senha, SENHA_TAM - 1);
    c->saldo = saldo_inicial;
    c->qtd_transacoes = 0;

    char msg[HIST_TAM];
    snprintf(msg, sizeof(msg), "Conta criada | Saldo: R$ %.2f", c->saldo);
    registrar_transacao(c, msg);

    (*qtd)++;
    printf("Conta cadastrada com sucesso!\n");
    return 1;
}

static int login(Conta contas[], int qtd, Conta **out_conta) {
    char id[ID_TAM];
    char senha[SENHA_TAM];

    print_banner("LOGIN");

    printf("ID da conta: ");
    fgets(id, sizeof(id), stdin);
    size_t len = strlen(id);
    if (len > 0 && id[len - 1] == '\n') id[len - 1] = '\0';

    printf("Senha (oculta): ");
    ler_senha_oculta(senha, SENHA_TAM);

    int idx = buscar_conta_por_id(contas, qtd, id);
    if (idx < 0) {
        printf("Conta não encontrada.\n");
        return 0;
    }

    if (strcmp(contas[idx].senha, senha) != 0) {
        printf("Senha incorreta.\n");
        return 0;
    }

    *out_conta = &contas[idx];
    printf("Login realizado com sucesso!\n");
    return 1;
}

static void menu_operacoes(Conta contas[], int qtd, Conta *conta_atual) {
    int opcao = 0;

    while (1) {
        limpar_tela();
        print_banner("MINI BANK — MENU");
        printf("Conta: %s\n", conta_atual->id);
        printf("Saldo atual: R$ %.2f\n", conta_atual->saldo);
        print_sep('-', 60);

        printf("1) Saque\n");
        printf("2) Deposito\n");
        printf("3) Transferencia\n");
        printf("4) Historico\n");
        printf("5) Salvar e sair\n");
        print_sep('-', 60);
        printf("Escolha: ");

        char linha[32];
        fgets(linha, sizeof(linha), stdin);
        opcao = atoi(linha);

        if (opcao == 1) {
            float valor = ler_valor_prompt("\nQuanto deseja sacar? (R$): ");
            int r = fazer_saque(conta_atual, valor);
            if (r == 0) printf("Valor invalido.\n");
            else if (r == -1) printf("Saldo insuficiente.\n");
            else printf("Saque realizado com sucesso!\n");
            printf("Pressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        } else if (opcao == 2) {
            float valor = ler_valor_prompt("\nQuanto deseja depositar? (R$): ");
            int r = fazer_deposito(conta_atual, valor);
            if (r == 0) printf("Valor invalido.\n");
            else printf("Deposito realizado com sucesso!\n");
            printf("Pressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        } else if (opcao == 3) {
            printf("\nID da conta destino: ");
            char destino_id[ID_TAM];
            fgets(destino_id, sizeof(destino_id), stdin);
            size_t dl = strlen(destino_id);
            if (dl > 0 && destino_id[dl - 1] == '\n') destino_id[dl - 1] = '\0';

            float valor = ler_valor_prompt("Valor da transferencia (R$): ");

            int r = fazer_transferencia(contas, qtd, conta_atual, destino_id, valor);
            if (r == 0) printf("Valor invalido.\n");
            else if (r == -1) printf("Saldo insuficiente.\n");
            else if (r == -2) printf("Conta destino não existe.\n");
            else if (r == -3) printf("Você não pode transferir para si mesmo.\n");
            else printf("Transferencia realizada com sucesso!\n");

            printf("Pressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        } else if (opcao == 4) {
            limpar_tela();
            mostrar_historico(conta_atual);
            printf("\nPressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        } else if (opcao == 5) {
            if (salvar_contas(contas, qtd)) {
                printf("Dados salvos em %s.\n", DATA_FILE);
            } else {
                printf("Falha ao salvar dados em %s.\n", DATA_FILE);
            }
            break;
        } else {
            printf("Opcao invalida.\n");
            printf("Pressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        }
    }
}

int main(void) {
    Conta contas[MAX_CONTAS];
    int qtd = 0;
    carregar_contas(contas, &qtd);

    while (1) {
        limpar_tela();
        print_banner("INICIO");
        printf("Contas cadastradas: %d\n", qtd);
        print_sep('-', 60);

        printf("1) Login\n");
        printf("2) Cadastrar conta\n");
        printf("3) Sair\n");
        print_sep('-', 60);
        printf("Escolha: ");

        char linha[32];
        if (!fgets(linha, sizeof(linha), stdin)) continue;
        int opcao = atoi(linha);

        if (opcao == 1) {
            Conta *c = NULL;
            if (qtd == 0) {
                printf("Nenhuma conta cadastrada. Cadastre primeiro.\n");
                printf("Pressione ENTER...");
                fgets(linha, sizeof(linha), stdin);
                continue;
            }
            if (login(contas, qtd, &c)) {
                menu_operacoes(contas, qtd, c);
                break; // encerra após salvar e sair
            } else {
                printf("Pressione ENTER...");
                fgets(linha, sizeof(linha), stdin);
            }
        } else if (opcao == 2) {
            // garante stdin limpo
            cadastrar_conta(contas, &qtd);
            printf("Pressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        } else if (opcao == 3) {
            // salva antes de sair
            salvar_contas(contas, qtd);
            break;
        } else {
            printf("Opcao invalida. Pressione ENTER...");
            fgets(linha, sizeof(linha), stdin);
        }
    }

    return 0;
}

